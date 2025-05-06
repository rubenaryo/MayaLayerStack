#include <ai.h>
#include <sstream>
#include <string>
#include <iomanip>

AI_SHADER_NODE_EXPORT_METHODS(MLSMetalNodeMtd);

enum LayerStackMetalParams {
    p_albedo,
    p_eta,
    p_kappa,
    p_alpha,
};

node_parameters
{
    AiParameterRGB("albedo", 1.0f, 0.7f, 0.7f);
    AiParameterFlt("IOR", 0.5f);
    AiParameterFlt("kappa", 3.0f);
    AiParameterFlt("roughness", 0.2f);
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
    float eta = AiShaderEvalParamFlt(p_eta);
    float kappa = AiShaderEvalParamFlt(p_kappa);
    float alpha = AiShaderEvalParamFlt(p_alpha);

    std::stringstream ss;
    ss << std::fixed << std::setprecision(3);
    ss << "{albedo=" << albedo.r << "," << albedo.g << "," << albedo.b << ";";
    ss << "eta=" << eta << ";";
    ss << "kappa=" << kappa << ";";
    ss << "alpha=" << alpha << "}";

    std::string outstd = ss.str();

    AtString out(outstd.c_str());

    sg->out.STR() = out;
}
