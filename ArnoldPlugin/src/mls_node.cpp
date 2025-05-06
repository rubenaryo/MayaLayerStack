#include <ai.h>
#include <fstream>
#include <string>
#include <sstream>
#include "mls_bsdf.h"

AI_SHADER_NODE_EXPORT_METHODS(MLSNodeMtd);

struct MaterialParam {
    std::string key;
    std::vector<float> values;
};

std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> parts;
    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        parts.push_back(str.substr(start, end - start));
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    parts.push_back(str.substr(start));  // 剩下最后一段
    return parts;
}

MaterialParam parse_param(const std::string& param_str) {
    MaterialParam param;

    size_t sep = param_str.find('=');
    if (sep == std::string::npos)
        return param; // 空结构体返回

    param.key = param_str.substr(0, sep);
    std::string value_str = param_str.substr(sep + 1);

    if (param.key == "albedo") {
        // value 形如 "0.3,0.4,0.5"
        std::vector<std::string> comps = split(value_str, ",");
        for (const std::string& s : comps) {
            param.values.push_back(std::stof(s));
        }
    }
    else {
        // 只有一个 float
        param.values.push_back(std::stof(value_str));
    }

    return param;
}

enum LayerStackParams {
    /*p_albedo_0,
    p_eta_0,
    p_kappa_0,
    p_alpha_0,
    p_albedo_m,
    p_depth_m,
    p_albedo_1,
    p_eta_1,
    p_kappa_1,
    p_alpha_1*/
    p_param
};

node_parameters
{
    /*AiParameterRGB("albedo_0", 1.0f, 1.0f, 1.0f);
    AiParameterFlt("eta_0", 1.5f);
    AiParameterFlt("kappa_0", 0.0f);
    AiParameterFlt("alpha_0", 0.001f);
    AiParameterRGB("albedo_m", 0.0f, 1.0f, 0.0f);
    AiParameterFlt("depth_m", 0.01f);
    AiParameterRGB("albedo_1", 1.0f, 0.7f, 0.7f);
    AiParameterFlt("eta_1", 0.47f);
    AiParameterFlt("kappa_1", 2.9f);
    AiParameterFlt("alpha_1", 0.2f);*/
    AiParameterStr("param", "");
}

node_initialize
{
    LayerStackBSDF * bsdf = new LayerStackBSDF;
    AiNodeSetLocalData(node, bsdf);
}

node_update
{
}

node_finish
{
}

shader_evaluate
{
    if (sg->Rt & AI_RAY_SHADOW)
        return;
    /*AtRGB albedo_0 = AiShaderEvalParamRGB(p_albedo_0);
    float eta_0 = AiShaderEvalParamFlt(p_eta_0);
    float kappa_0 = AiShaderEvalParamFlt(p_kappa_0);
    float alpha_0 = AiShaderEvalParamFlt(p_alpha_0);
    AtRGB albedo_m = AiShaderEvalParamRGB(p_albedo_m);
    float depth_m = AiShaderEvalParamFlt(p_depth_m);
    AtRGB albedo_1 = AiShaderEvalParamRGB(p_albedo_1);
    float eta_1 = AiShaderEvalParamFlt(p_eta_1);
    float kappa_1 = AiShaderEvalParamFlt(p_kappa_1);
    float alpha_1 = AiShaderEvalParamFlt(p_alpha_1);*/
    
    AtString params = AiShaderEvalParamStr(p_param);
    std::string paramstr(params.c_str());

    LayerStackBSDF lsbsdf;
    /*lsbsdf.albedos.push_back(albedo_0);
    lsbsdf.etas.push_back(eta_0);
    lsbsdf.kappas.push_back(kappa_0);
    lsbsdf.alphas.push_back(alpha_0);
    lsbsdf.depths.push_back(0);
    lsbsdf.sigma_a.push_back(AtRGB(0));
    lsbsdf.sigma_s.push_back(AtRGB(0));

    lsbsdf.albedos.push_back(albedo_m);
    lsbsdf.etas.push_back(1.0f);
    lsbsdf.kappas.push_back(0);
    lsbsdf.alphas.push_back(0);
    lsbsdf.depths.push_back(depth_m);
    AtRGB sigma_a, sigma_s;
    computeSigma(albedo_m, 0.1, sigma_a, sigma_s);
    lsbsdf.sigma_a.push_back(sigma_a);
    lsbsdf.sigma_s.push_back(sigma_s);

    lsbsdf.albedos.push_back(albedo_1);
    lsbsdf.etas.push_back(eta_1);
    lsbsdf.kappas.push_back(kappa_1);
    lsbsdf.alphas.push_back(alpha_1);
    lsbsdf.depths.push_back(0);
    lsbsdf.sigma_a.push_back(AtRGB(0));
    lsbsdf.sigma_s.push_back(AtRGB(0));*/

    // 步骤1：去掉首尾的 `{` 和 `}`
    if (!paramstr.empty() && paramstr.front() == '{') paramstr.erase(0, 1);
    if (!paramstr.empty() && paramstr.back() == '}') paramstr.pop_back();

    // 步骤2：按 "}{" 分割
    std::vector<std::string> layers = split(paramstr, "}{");

    // 步骤3：每层再按 `;` 分割
    //std::ofstream outFile("E:/CIS6600_/SIG_TOOL/plugin/output.txt", std::ios::app);
    for (const std::string& layer : layers) {
        std::vector<std::string> params_layer = split(layer, ";");
        AtRGB albedo(1.0);
        float eta = 1.0;
        float kappa = 0.0;
        float alpha = 0.0;
        float depth = 0.0;
        float g = 0.0;
        AtRGB sigma_a(0.0);
        AtRGB sigma_s(0.0);
        
        for (const std::string& p : params_layer) {
            MaterialParam mp = parse_param(p);
            if (mp.key == "albedo") {
                albedo.r = mp.values[0];
                albedo.g = mp.values[1];
                albedo.b = mp.values[2];
            }
            else if (mp.key == "eta") {
                eta = mp.values[0];
            }
            else if (mp.key == "kappa") {
                kappa = mp.values[0];
            }
            else if (mp.key == "alpha") {
                alpha = mp.values[0];
            }
            else if (mp.key == "depth") {
                depth = mp.values[0];
            }
            else if (mp.key == "g") {
                g = mp.values[0];
            }
        }
        if (depth > 0.0) {
            computeSigma(albedo, 1.0, sigma_a, sigma_s);
            alpha = gToVariance(g);
            eta = lsbsdf.etas.back();
        }
        
        lsbsdf.albedos.push_back(albedo);
        lsbsdf.etas.push_back(eta);
        lsbsdf.kappas.push_back(kappa);
        lsbsdf.alphas.push_back(alpha);
        lsbsdf.depths.push_back(depth);
        lsbsdf.sigma_a.push_back(sigma_a);
        lsbsdf.sigma_s.push_back(sigma_s);
    }

    //outFile.close();

    sg->out.CLOSURE() = LayerStackBSDFCreate(sg, lsbsdf);
}