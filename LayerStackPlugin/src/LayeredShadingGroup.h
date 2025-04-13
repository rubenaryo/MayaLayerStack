#pragma once

#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MString.h>

struct LayeredMaterialNode;

struct LayeredShadingGroup
{
	MStatus Create();
	MStatus AssignMaterial(LayeredMaterialNode* pRoot = nullptr);

	LayeredMaterialNode* mMaterialRoot = nullptr;
	MString mName;
};