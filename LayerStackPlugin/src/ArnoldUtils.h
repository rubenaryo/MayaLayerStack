#pragma once

#include <maya/MGlobal.h>

// Function to load a custom Arnold shader DLL
MStatus LoadArnoldShader(const MString& shaderDLLPath);

