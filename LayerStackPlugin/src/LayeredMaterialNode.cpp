#include "LayeredMaterialNode.h"

MString GetNodeTypeName(NodeType t)
{
	if (t < 0 || t >= NT_COUNT)
		return MString("INVALID");

	static const char* sNodeNames[NT_COUNT] =
	{
		"mlsLayeredSurface",	// NT_SURFACE
		"mlsLayerAdd",			// NT_ADD
		"mlsLayerVolumetric",	// NT_VOLUMETRIC
		"mlsLayerDielectric",	// NT_DIELECTRIC
		"mlsLayerMetal"			// NT_METAL
	};

	return MString(sNodeNames[t]);
}

MStatus ExecuteNodeCreation(NodeType t, MString& outName)
{
	MString cmd = "createNode " + GetNodeTypeName(t);
	MGlobal::displayInfo("[LayerStack] Executing Command: " + cmd);
	return MGlobal::executeCommand(cmd, outName);
}

MString GetNodeTypeOutputParamName(NodeType t)
{
	switch (t)
	{
	case NT_SURFACE:
		return MString("outColor");
	case NT_ADD:
	case NT_VOLUMETRIC:
	case NT_DIELECTRIC:
	case NT_METAL:
		return MString("outParam");
	}
	return MString("INVALID");
}

// Depends on the index, since it's either the top or bottom layer
MString GetNodeTypeInputParamName(NodeType t, NodeChildIndex index)
{
	switch (t)
	{
	case NT_SURFACE:
		return MString("param");
	case NT_ADD:
		return index == TOP_LAYER ? MString("top") : MString("bottom");
	case NT_VOLUMETRIC:
	case NT_DIELECTRIC:
	case NT_METAL:
		MGlobal::displayError("[LayerStack] ERROR: Attempted to add a node input to a node type that does not support it!");
	}

	return MString("INVALID");
}

size_t GetNodeTypeChildCount(NodeType t)
{
	switch (t)
	{
	case NT_SURFACE:
		return 1;
	case NT_ADD:
		return 2;

	// These accept material parameters
	case NT_VOLUMETRIC:
	case NT_DIELECTRIC:
	case NT_METAL:
		return 0;
	}
	return 0;
}

MStatus LinkNodes(LayeredMaterialNode& parent, LayeredMaterialNode& child, NodeChildIndex childIndex)
{
	MString cmd = "connectAttr -f " +
		child.mInstanceName + "." + GetNodeTypeOutputParamName(child.mType) + " " +
		parent.mInstanceName + "." + GetNodeTypeInputParamName(parent.mType, childIndex);

	MGlobal::displayInfo("[LayerStack] Executing Command: " + cmd);
	return MGlobal::executeCommand(cmd);
}


LayeredMaterialNode::LayeredMaterialNode(NodeType t)
	:	mChildren(nullptr)
	,	mInstanceName("")
	,	mType(t)
	,	mChildrenCount(0)
{
	mChildrenCount = GetNodeTypeChildCount(t);
	if (mChildrenCount > 0)
	{
		mChildren = new LayeredMaterialNode*[mChildrenCount]();
	}
}

LayeredMaterialNode::~LayeredMaterialNode()
{
	if (mChildren)
	{
		delete[] mChildren;
		mChildren = nullptr;
	}
}

MStatus LayeredMaterialNode::Create()
{
	return ExecuteNodeCreation(mType, mInstanceName);
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

// For convenience in initializing example materials.
MStatus LayeredMaterialNode::InitDefaultMaterialTree()
{
	LayeredMaterialNode* addNode0		= new LayeredMaterialNode(NT_ADD);
	LayeredMaterialNode* volumetricNode	= new LayeredMaterialNode(NT_VOLUMETRIC);
	LayeredMaterialNode* dielectricNode	= new LayeredMaterialNode(NT_DIELECTRIC);

	if (!addNode0 || !volumetricNode || !dielectricNode)
	{
		MGlobal::displayError("[LayerStack] ERROR: Failed to allocate add, volumetric, or dielectric nodes.");
	}

	MStatus status = addNode0->Create();
	LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to create add");

	status = dielectricNode->Create();
	LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to create dielectric node.");

	status = volumetricNode->Create();
	LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to create volumetric node.");

	status = SetChild(PARAM, addNode0);
	LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to set add0 as child of surface");

	status = addNode0->SetChild(TOP_LAYER, dielectricNode);
	LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to set diel as child of add0");

	status = addNode0->SetChild(BOTTOM_LAYER, volumetricNode);
	LAYERSTACK_CHECK_STATUS_LOG_AND_RETURN(status, "Failed to set vol as child of add0");

	return status;
}

MStatus LayeredMaterialNode::SetChild(NodeChildIndex index, LayeredMaterialNode* pChild)
{
	if (index >= mChildrenCount )
		return MStatus::kFailure;

	LayeredMaterialNode* pCurrChild = mChildren[index];
	if (pCurrChild)
	{
		// TODO: unlink gracefully
	}

	MStatus ret = MStatus::kSuccess;
	if (pChild)
	{
		ret = LinkNodes(*this, *pChild, index);
	}

	mChildren[index] = pChild;
	return ret;
}