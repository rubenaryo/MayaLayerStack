#include "LayeredMaterialNode.h"

#include <maya/Mstring.h>

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

MStatus ExecuteNodeCreation(NodeType t, MString& outName, const char* pDesiredName)
{
	MString cmd = "createNode " + GetNodeTypeName(t);
	if (pDesiredName)
	{
		cmd += " -n " + MString(pDesiredName);
	}

	MGlobal::displayInfo("[LayerStack] Executing Command: " + cmd);
	return MGlobal::executeCommand(cmd, outName);
}

MStatus ExecuteVec3ParamSet(MString& mayaName, const std::string& paramName, nlohmann::json& jsonVec3)
{
	//setAttr "mlsLayerMetal1.albedo" - type double3 0.5229 0.564635 0.747;
	MString cmd = "setAttr \"" + mayaName + "." + MString(paramName.c_str()) + "\" -type double3 "
		+ jsonVec3[0].get<float>() + MString(" ")
		+ jsonVec3[1].get<float>() + MString(" ")
		+ jsonVec3[1].get<float>() + MString(";");
	
	MGlobal::displayInfo("[LayerStack] Executing Command: " + cmd);
	return MGlobal::executeCommand(cmd);
}

MStatus ExecuteFloatParamSet(MString& mayaName, const std::string& paramName, float val)
{
	//setAttr "mlsLayerMetal1.kappa" 1.977012;
	MString cmd = "setAttr \"" + mayaName + "." + MString(paramName.c_str()) + MString("\" ") + val;

	MGlobal::displayInfo("[LayerStack] Executing Command: " + cmd);
	return MGlobal::executeCommand(cmd);
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

NodeType GetNodeTypeFromString(const std::string& str)
{
	if (str == "surface")
	{
		return NT_SURFACE;
	}
	else if (str == "add")
	{
		return NT_ADD;
	}
	else if (str == "volumetric")
	{
		return NT_VOLUMETRIC;
	}
	else if (str == "dielectric")
	{
		return NT_DIELECTRIC;
	}
	else if (str == "metal")
	{
		return NT_METAL;
	}
	else
	{
		return NT_INVALID;
	}
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
	,	mChildrenCapacity(0)
{
	mChildrenCapacity = GetNodeTypeChildCount(t);
	if (mChildrenCapacity > 0)
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

MStatus LayeredMaterialNode::Create(const char* pDesiredName)
{
	return ExecuteNodeCreation(mType, mInstanceName, pDesiredName);
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

MStatus LayeredMaterialNode::InitFromJSON(MString& jsonData, MString& desiredMaterialName)
{
	using json = nlohmann::json;

	MStatus status = MStatus::kSuccess;
	
	const char* jsonStr = jsonData.asChar();
	const char* matNameStr = desiredMaterialName.asChar();

	const char* TEST_STR = "'root': {'type': 'root', 'children' : ['layer_1'] }, 'layer_1' : {'type': 'surface', 'parent' : 'root', 'children' : ['layer_2'] , 'params' : {'name': 'New Multi-Layer Material'}}, 'layer_2' : {'type': 'add', 'parent' : 'layer_1', 'children' : ['layer_3', 'layer_4'] , 'params' : {'name': 'Add_2'}, 'top_layer' : 'layer_3', 'bottom_layer' : 'layer_4'}, 'layer_3' : {'type': 'metal', 'parent' : 'layer_2', 'children' : [] , 'params' : {'albedo': [0.758169949054718, 0.0, 0.0] , 'ior' : 3.2535947714999223, 'kappa' : 1.0, 'roughness' : 0.1, 'name' : 'NewMetal'}, 'position' : 'top'}, 'layer_4' : {'type': 'dielectric', 'parent' : 'layer_2', 'children' : [] , 'params' : {'ior': 1.5, 'roughness' : 0.1, 'name' : 'NewDielectric'}, 'position' : 'bottom'}";

	json materialData;
	
	try
	{
		materialData = json::parse(jsonStr);
	}
	catch (json::parse_error& e)
	{
		MGlobal::displayError("[LayerStack] JSON PARSE ERROR: " + MString(e.what()));
	}


	//std::string masterStr = materialData.get<std::string>().c_str();
	
	// TODO: Maybe just do a table lookup?
	json::iterator it = materialData.begin();
	bool found = false;
	for (;it != materialData.end(); it++)
	{
		if (!it->is_object())
			continue;

		auto params = (*it)["params"];
		auto nameParam  = params["name"];
		if (!nameParam.is_string())
			continue;

		std::string name = nameParam.get<std::string>();
		if (name == matNameStr)
		{
			found = true;
			break;
		}
	}

	if (!found)
	{
		MGlobal::displayError("[LayerStack] Failed to find material from jsonData");
		return MStatus::kNotFound;
	}

	status = InitChildrenFromJSON(materialData, it.value());

	return status;
}

MStatus LayeredMaterialNode::InitChildrenFromJSON(nlohmann::json& jsonData, nlohmann::json& parent)
{
	using json = nlohmann::json;

	MStatus status;

	auto children = parent["children"];
	json::iterator it = children.begin();

	static const int MAX_ITER_COUNT = 3;
	int counter = 0;
	for (; it != children.end(); it++)
	{
		if (counter++ > MAX_ITER_COUNT)
		{
			MGlobal::displayWarning("[LayerStack] Warning: Max iteration count exceeded when iterating through children. Force exiting...");
			break;
		}

		if (mChildrenCount >= GetChildCapacity())
			break; // Can accept no more children

		if (!it->is_string())
			continue;
		
		std::string json_name = it->get<std::string>();
		auto child_node = jsonData[json_name];
		std::string node_type = child_node["type"].get<std::string>();
		NodeType type = GetNodeTypeFromString(node_type);

		LayeredMaterialNode* node = new LayeredMaterialNode(type);
		status = node->Create();
		
		if (child_node.find("params") != child_node.end())
			status = node->SetParamsFromJSON(child_node["params"]);
			
		status = SetChild((NodeChildIndex)mChildrenCount, node);
		status = node->InitChildrenFromJSON(jsonData, child_node);
	}

	return status;
}

MStatus LayeredMaterialNode::SetParamsFromJSON(nlohmann::json& paramsStruct)
{
	using json = nlohmann::json;

	MStatus status = MStatus::kSuccess;
	json::iterator it = paramsStruct.begin();
	for (; it != paramsStruct.end(); ++it)
	{
		const std::string& paramName = it.key(); // param name
		json& paramValue = it.value();

		if (paramValue.is_array()) // Assume vec3
		{
			ExecuteVec3ParamSet(mInstanceName, paramName, paramValue);
		}
		else if (paramValue.is_number()) // Assume float
		{
			ExecuteFloatParamSet(mInstanceName, paramName, paramValue.get<float>());
		}
		else
		{
			continue; // Names and such
		}
	}

	return status;
}

MStatus LayeredMaterialNode::SetChild(NodeChildIndex index, LayeredMaterialNode* pChild)
{
	if (index >= GetChildCapacity())
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
	mChildrenCount++;
	return ret;
}

