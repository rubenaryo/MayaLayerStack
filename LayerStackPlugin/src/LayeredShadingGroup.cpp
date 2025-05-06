#include "LayeredShadingGroup.h"
#include "LayeredMaterialNode.h"

#include <maya/MString.h>
#include <maya/MGlobal.h>

//static const MString LAYER_STACK_GROUP_NAME = "LayerStackShadingGroup";

MStatus LayeredShadingGroup::Create(MString& materialName)
{
	MStatus status;

	MString desiredName = materialName + "_ShadingGroup";

	MString cmd = "sets -renderable true -noSurfaceShader true -empty -name " + desiredName + ";";
	status = MGlobal::executeCommand(cmd, mName);
	MGlobal::displayInfo("[LayerStack] Result: " + mName);

	return status;
}

MStatus LayeredShadingGroup::AssignMaterial(LayeredMaterialNode* pRoot, MString& materialName)
{
	MStatus status;
	if (!pRoot)
	{
		// Create and initialize it if null is passed in.
		pRoot = new LayeredMaterialNode(NT_SURFACE);
		status = pRoot->Create(materialName.asChar());
		if (status != MStatus::kSuccess)
		{
			MGlobal::displayInfo("Failed to create root surface shader.");
			return status;
		}
	}

	MString cmd = "connectAttr -f " + pRoot->mInstanceName + ".outColor " + mName + ".surfaceShader";
	status = MGlobal::executeCommand(cmd);

	mMaterialRoot = pRoot;
	return status;
}
