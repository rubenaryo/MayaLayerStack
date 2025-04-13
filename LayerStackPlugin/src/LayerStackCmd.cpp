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

static const MString LAYER_STACK_MATERIAL_NAME = "LayerStackMaterial";
static const MString LAYER_STACK_GROUP_NAME = "LayerStackShadingGroup";

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

    MString cmd = "connectAttr -f " + meshFn.name() + ".instObjGroups[0] " + shadingGroup.mName + ".dagSetMembers[0]";
    MGlobal::displayInfo("Executing command: " + cmd);
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
    MStatus status;

    if (args.length() < 1)
    {
        MGlobal::displayError("You must pass a mesh as an argument.");
        return MS::kFailure;
    }

    // Get the mesh name passed as an argument
    MString selectedStr = args.asString(0);

    // Look up the shape node from the name
    MObject shapeNode;
    status = GetShapeNodeFromSelection(selectedStr, shapeNode);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to fetch shape node from selection: " + selectedStr);

    MFnMesh meshFn(shapeNode);

    MObject blinnObj;
    MObject layeredSGObj;

    LayeredShadingGroup shadingGroup;
    status = shadingGroup.Create();
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to create SG");
    
    status = shadingGroup.AssignMaterial();
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to assign material to SG");
    
    status = shadingGroup.mMaterialRoot->InitDefaultMaterialTree();
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to init default material tree");

    // Disconnect the mesh from any existing shading group
    status = DisconnectFromCurrentShadingGroup(shapeNode);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to disconnect " + meshFn.name() + " from current SG");

    // Connect the mesh to the layer stack shading group.
    status = ConnectToLayeredShadingGroup(shapeNode, shadingGroup);
    LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to connect " + meshFn.name() + " to layered SG");

    return status;
}

