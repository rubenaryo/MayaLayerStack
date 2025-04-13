#include "LayeredShadingGroup.h"
#include "LayeredMaterialNode.h"

#include <maya/MString.h>
#include <maya/MGlobal.h>

static const MString LAYER_STACK_GROUP_NAME = "LayerStackShadingGroup";
MStatus LayeredShadingGroup::Create()
{
	MStatus status;

	MString cmd = "sets -renderable true -noSurfaceShader true -empty -name " + LAYER_STACK_GROUP_NAME + ";";
	MGlobal::displayInfo("[LayerStack] Executing command: " + cmd);
	status = MGlobal::executeCommand(cmd, mName);
	MGlobal::displayInfo("[LayerStack] Result: " + mName);

	return status;
}

MStatus LayeredShadingGroup::AssignMaterial(LayeredMaterialNode* pRoot)
{
	MStatus status;
	if (!pRoot)
	{
		// Create and initialize it if null is passed in.
		pRoot = new LayeredMaterialNode(NT_SURFACE);
		status = pRoot->Create();
		if (status != MStatus::kSuccess)
		{
			MGlobal::displayInfo("Failed to create root surface shader.");
			return status;
		}
	}

	MString cmd = "connectAttr -f " + pRoot->mInstanceName + ".outColor " + mName + ".surfaceShader";
	MGlobal::displayInfo("[LayerStack] Executing command: " + cmd);
	status = MGlobal::executeCommand(cmd);

	mMaterialRoot = pRoot;
	return status;
}
