#pragma once

#include <maya/MGlobal.h>
#include <maya/MString.h>
#include <maya/MStatus.h>

#include <vector>
#include "external/nlohmann/json.hpp"

enum NodeType
{
	NT_INVALID = -1,
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
MStatus ExecuteNodeCreation(NodeType t, MString& outName, const char* pDesiredName = nullptr);
MString GetNodeTypeOutputParamName(NodeType t);
size_t GetNodeTypeChildCount(NodeType t);

struct LayeredMaterialNode
{
	LayeredMaterialNode(NodeType t);
	~LayeredMaterialNode();

	MStatus Create(const char* pDesiredName = nullptr);
	MStatus Delete();
	void Reset();

	MStatus InitDefaultMaterialTree();
	MStatus InitFromJSON(MString& jsonData, MString& desiredMaterialName);
	MStatus InitChildrenFromJSON(nlohmann::json& jsonData, nlohmann::json& parent, const std::string& rootMaterialName);
	MStatus SetChild(NodeChildIndex index, LayeredMaterialNode* pChild);
	MStatus SetParamsFromJSON(nlohmann::json& paramsStruct);

	size_t GetChildCapacity() const { return mChildrenCapacity; }

	LayeredMaterialNode** mChildren;
	MString mInstanceName;
	NodeType mType;
	size_t mChildrenCount;
	size_t mChildrenCapacity;
	bool bCreated;
};

MStatus LinkNodes(LayeredMaterialNode& parent, LayeredMaterialNode& child, NodeChildIndex childIndex);