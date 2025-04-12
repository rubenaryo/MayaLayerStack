#include <ai.h>
#include <sstream>
#include <string>
#include <iomanip>

AI_SHADER_NODE_EXPORT_METHODS(MLSDielectricNodeMtd);

enum LayerStackDielectricParams {
    p_eta,
    p_alpha,
};

node_parameters
{
    AiParameterFlt("IOR", 1.5f);
    AiParameterFlt("roughness", 0.01f);
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
    float eta = AiShaderEvalParamFlt(p_eta);
    float alpha = AiShaderEvalParamFlt(p_alpha);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{eta=" << eta << ";";
    ss << "alpha=" << alpha << "}";

    std::string outstd = ss.str();

    AtString out(outstd.c_str());

    sg->out.STR() = out;
}
