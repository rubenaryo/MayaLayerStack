#include "ArnoldUtils.h"

#include <ai.h>
#include <ai_nodes.h>
//#include <ai_shader.h>
#include <ai_shaderglobals.h>

MStatus LoadArnoldShader(const MString& shaderDLLPath)
{
	AiLoadPlugins(shaderDLLPath.asChar());
	return MS::kSuccess;
}
