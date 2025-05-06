#include "LayerStackCmd.h"
#include "LayeredMaterialNode.h"
#include "LayeredShadingGroup.h"

#include <maya/MArgList.h>
#include <maya/MFnMesh.h>
#include <maya/MPlug.h>
#include <maya/MPlugArray.h>
#include <maya/MSelectionList.h>
#include <maya/MFnSet.h>
#include <maya/MGlobal.h>
#include <maya/MDagPath.h>
#include <maya/MFnAttribute.h>

#include "external/nlohmann/json.hpp"

// Global, static state. For simplicity..
std::vector<LayeredShadingGroup*> sShadingGroups;
void LayerStackCmd::CleanupShadingGroups()
{
    for (LayeredShadingGroup* group : sShadingGroups)
    {
        if (!group)
            continue;

        delete group;
    }
    sShadingGroups.clear();
}

// Stolen from Maya Code, but added a custom log message
#define LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(_status, _logMsg)		\
{ 														\
	MStatus _maya_status = (_status);					\
	if ( MStatus::kSuccess != _maya_status ) 			\
	{													\
        MGlobal::displayError( _logMsg );               \
		_maya_status.pAPIerror ( __FILE__, __LINE__);   \
		return (_status);								\
	}													\
}

MStatus DebugPrintAttributes(MFnDependencyNode& fn)
{
    MStatus status;
    unsigned int numAtt = fn.attributeCount(&status);
    MGlobal::displayInfo("Attributes for : " + fn.name());
    for (unsigned int i = 0; i < numAtt; ++i)
    {
        MObject attrObj = fn.attribute(i, &status);
        if (status) {
            MFnAttribute attrFn(attrObj);
            MString attrName = attrFn.name(&status);
            MGlobal::displayInfo(MString("\tAttribute ") + i + ": " + attrName + "| Class: " + attrFn.type());
        }
    }

    return status;
}

MStatus GetShapeNodeFromSelection(const MString& selection, MObject& out_shapeNode)
{
    MStatus status;
    MObject selectedObj;

    MSelectionList selList;
    selList.add(selection);
    status = selList.getDependNode(0, selectedObj);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to find shape node for selection: " + selection);

    MDagPath meshPath;
    MFnDagNode dagFn(selectedObj);
    MObject shapeNode = dagFn.child(0, &status);
    dagFn.getPath(meshPath);

    MFnMesh meshFn(shapeNode, &status);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to create MeshFn for selection: " + selection);

    out_shapeNode = shapeNode;
    return status;
}

MStatus DisconnectFromCurrentShadingGroup(MObject& shapeObj)
{
    MStatus status;

    MFnMesh meshFn(shapeObj, &status);

    MDGModifier dgModifier;
    MFnDagNode meshDagNode(shapeObj);

    // Find the instObjGroups plug on the mesh
    MPlug instObjGroupsPlug = meshDagNode.findPlug("instObjGroups", false, &status);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to find instObjGroups on " + meshDagNode.name());

    MPlug element = instObjGroupsPlug.elementByLogicalIndex(0);

    // Check if this element is already connected
    MPlugArray connections;
    element.connectedTo(connections, false, true);

    if (connections.length() > 0) {
        for (unsigned int i = 0; i < connections.length(); i++) {
            MObject connectedNode = connections[i].node();
            MFnDependencyNode nodeFn(connectedNode);

            if (connectedNode.apiType() == MFn::kShadingEngine) {

                // Found a shading group
                MGlobal::displayInfo("Disconnecting " + meshFn.name() + " from shading group: " + nodeFn.name());

                // Disconnect existing connection first
                status = dgModifier.disconnect(element, connections[0]);
                LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to disconnect from existing shadingGroup");

                status = dgModifier.doIt();
                LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to doit() disconnect from existing shadingGroup");
            }
        }
    }

    return status;
}

MStatus ConnectToLayeredShadingGroup(MObject& shapeObj, LayeredShadingGroup& shadingGroup)
{
    MStatus status;
    MFnMesh meshFn(shapeObj, &status);

    //MString cmd = "connectAttr -f " + meshFn.name() + ".instObjGroups[0] " + shadingGroup.mName + ".dagSetMembers[0]";
    MString cmd = "sets -addElement " + shadingGroup.mName + " " + meshFn.name();
    return MGlobal::executeCommand(cmd);
}

LayerStackCmd::LayerStackCmd() : MPxCommand()
{
}

LayerStackCmd::~LayerStackCmd() 
{
}

MStatus LayerStackCmd::doIt( const MArgList& args )
{
    using json = nlohmann::json;
    MStatus status;

    if (args.length() < 3)
    {
        MGlobal::displayError("You must pass the mesh, the json structure, and the desired material as an argument.");
        return MS::kFailure;
    }

    // Get the mesh name passed as an argument
    MString selectedStr = args.asString(0);
    MString jsonComplete = args.asString(1);
    MString materialName = args.asString(2);
    const char* testName = materialName.asChar();
    const wchar_t* testName2 = materialName.asWChar();

    // Look up the shape node from the name
    MObject shapeNode;
    status = GetShapeNodeFromSelection(selectedStr, shapeNode);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to fetch shape node from selection: " + selectedStr);

    MFnMesh meshFn(shapeNode);

    LayeredShadingGroup* pShadingGroup = FindShadingGroupForMaterialName(materialName);
    if (pShadingGroup)
    {
        MGlobal::displayInfo("[LayerStack] Found existing shading group: " + pShadingGroup->mName + "\n");
    }

    if (!pShadingGroup)
    {
        // Need to create a new one.
        pShadingGroup = CreateNewShadingGroup(materialName);
        if (!pShadingGroup)
        {
            MGlobal::displayError("[LayerStack] Fatal Error: Failed to allocate new shading group");
            return MStatus::kFailure;
        }
    }

    if (!pShadingGroup)
    {
        throw std::exception("[LayerStack] Error: Failed to create shading group!");
        return MStatus::kFailure;
    }

    // Even if the group already exists, the user may have modified properties, so need to re-init from the latest JSON.
    if (pShadingGroup->mMaterialRoot)
    {
        pShadingGroup->mMaterialRoot->Delete();
        delete pShadingGroup->mMaterialRoot;
        pShadingGroup->mMaterialRoot = nullptr;
    }

    status = pShadingGroup->AssignMaterial(nullptr, materialName);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to assign material to SG");

    status = pShadingGroup->mMaterialRoot->InitFromJSON(jsonComplete, materialName);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to init default material tree");

    // Disconnect the mesh from any existing shading group
    status = DisconnectFromCurrentShadingGroup(shapeNode);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to disconnect " + meshFn.name() + " from current SG");

    // Connect the mesh to the shading group.
    status = ConnectToLayeredShadingGroup(shapeNode, *pShadingGroup);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to connect " + meshFn.name() + " to layered SG");

    return status;
}

// Linear search is OK here since there probably won't be too many shading groups
LayeredShadingGroup* LayerStackCmd::FindShadingGroupForMaterialName(MString& materialName)
{
    LayeredShadingGroup* ret = nullptr;

    const char* targetCStr = materialName.asChar();
    std::string targetStr(targetCStr);

    for (LayeredShadingGroup* group : sShadingGroups)
    {
        if (!group || !group->mMaterialRoot)
            continue;

        // If the surface material name matches, return it so we don't have to create a new one. 
        if (materialName == group->mMaterialRoot->mInstanceName)
        {
            ret = group;
            break;
        }
    }

    return ret;
}

LayeredShadingGroup* LayerStackCmd::CreateNewShadingGroup(MString& materialName)
{
    LayeredShadingGroup* newGroup = new LayeredShadingGroup();
    sShadingGroups.push_back(newGroup);
    newGroup->Create(materialName);

    return newGroup;
}

