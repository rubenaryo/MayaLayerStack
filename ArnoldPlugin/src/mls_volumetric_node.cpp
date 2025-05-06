#include <ai.h>
#include <sstream>
#include <string>
#include <iomanip>

AI_SHADER_NODE_EXPORT_METHODS(MLSVolumetricNodeMtd);

enum LayerStackVolumetricParams {
    p_albedo,
    p_depth,
    p_g
};

node_parameters
{
    AiParameterRGB("albedo", 0.0f, 0.62f, 1.0f);
    AiParameterFlt("depth", 0.1f);
    AiParameterFlt("g", 0.7f);
}

node_initialize
{
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
    AtRGB albedo = AiShaderEvalParamRGB(p_albedo);
    float depth = AiShaderEvalParamFlt(p_depth);
    float g = AiShaderEvalParamFlt(p_g);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{albedo=" << albedo.r << "," << albedo.g << "," << albedo.b << ";";
    ss << "depth=" << depth << ";";
    ss << "g=" << g << "}";

    std::string outstd = ss.str();

    AtString out(outstd.c_str());

    sg->out.STR() = out;
}
