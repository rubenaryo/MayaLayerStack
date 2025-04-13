#pragma once

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MStatus.h>

#include <vector>

enum NodeType
{
	NT_SURFACE = 0,
	NT_ADD,
	NT_VOLUMETRIC,
	NT_DIELECTRIC,
	NT_METAL,
	NT_COUNT
};

enum NodeChildIndex
{
	TOP_LAYER = 0,
	BOTTOM_LAYER = 1,
	PARAM = 0 // For surface shader and shading groups... Just use the first index.
};

MString GetNodeTypeName(NodeType t);
MStatus ExecuteNodeCreation(NodeType t, MString& outName);
MString GetNodeTypeOutputParamName(NodeType t);
size_t GetNodeTypeChildCount(NodeType t);

// Really only an instance name..
struct LayeredMaterialNode
{
	LayeredMaterialNode(NodeType t);
	~LayeredMaterialNode();

	MStatus Create();

	MStatus InitDefaultMaterialTree();
	MStatus SetChild(NodeChildIndex index, LayeredMaterialNode* pChild);

	LayeredMaterialNode** mChildren;
	MString mInstanceName;
	NodeType mType;
	size_t mChildrenCount;
};

MStatus LinkNodes(LayeredMaterialNode& parent, LayeredMaterialNode& child, NodeChildIndex childIndex);