#pragma once

#include <maya/MObject.h>
#include <maya/MStatus.h>
#include <maya/MString.h>

struct LayeredMaterialNode;

struct LayeredShadingGroup
{
	MStatus Create(MString& materialName);
	MStatus AssignMaterial(LayeredMaterialNode* pRoot, MString& materialName);

	LayeredMaterialNode* mMaterialRoot = nullptr;
	MString mName;
};