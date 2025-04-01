#include <ai.h>
#include "mls_bsdf.h"

AI_SHADER_NODE_EXPORT_METHODS(LayerStackMtd);

enum LayerStackParams { 
    p_albedo_0,
    p_eta_0,
    p_kappa_0,
    p_alpha_0,
    p_albedo_1,
    p_eta_1,
    p_kappa_1,
    p_alpha_1
};

node_parameters
{
    AiParameterRGB("albedo_0", 1.0f, 0.0f, 0.0f);
    AiParameterFlt("eta_0", 1.5f);
    AiParameterFlt("kappa_0", 0.0f);
    AiParameterFlt("alpha_0", 0.001f);
    AiParameterRGB("albedo_1", 0.7f, 0.7f, 0.7f);
    AiParameterFlt("eta_1", 0.47f);
    AiParameterFlt("kappa_1", 2.9f);
    AiParameterFlt("alpha_1", 0.2f);
}

node_initialize
{
    LayerStackBSDF bsdf;
    AiNodeSetLocalData(node, (void*)(&bsdf));
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
    AtRGB albedo_0  = AiShaderEvalParamRGB(p_albedo_0);
    float eta_0     = AiShaderEvalParamFlt(p_eta_0);
    float kappa_0   = AiShaderEvalParamFlt(p_kappa_0);
    float alpha_0   = AiShaderEvalParamFlt(p_alpha_0);
    AtRGB albedo_1  = AiShaderEvalParamRGB(p_albedo_1);
    float eta_1     = AiShaderEvalParamFlt(p_eta_1);
    float kappa_1   = AiShaderEvalParamFlt(p_kappa_1);
    float alpha_1   = AiShaderEvalParamFlt(p_alpha_1);
    

    LayerStackBSDF lsbsdf;
    lsbsdf.albedos.push_back(albedo_0);
    lsbsdf.etas.push_back(eta_0);
    lsbsdf.kappas.push_back(kappa_0);
    lsbsdf.alphas.push_back(alpha_0);
    lsbsdf.albedos.push_back(albedo_1);
    lsbsdf.etas.push_back(eta_1);
    lsbsdf.kappas.push_back(kappa_1);
    lsbsdf.alphas.push_back(alpha_1);

    if (sg->Rt & AI_RAY_SHADOW)
        return;

    sg->out.CLOSURE() = LayerStackBSDFCreate(sg, lsbsdf);
}

node_loader
{
   if (i > 0)
      return false;
   node->methods = LayerStackMtd;
   node->output_type = AI_TYPE_CLOSURE;
   node->name = "layerstack";
   node->node_type = AI_NODE_SHADER;
   strcpy(node->version, AI_VERSION);
   return true;
}

