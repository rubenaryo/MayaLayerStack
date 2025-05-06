#pragma once
#include "util.h"

struct LayerStackBSDF
{
    std::vector<AtRGB> albedos;
    std::vector<float> etas;
    std::vector<float> kappas;
    std::vector<float> alphas;
    std::vector<float> depths;
    std::vector<AtRGB> sigma_a;
    std::vector<AtRGB> sigma_s;

    // geometry, don't need to set at creating bsdf
    /* parameters */
    AtVector N, wo;
    /* set in bsdf_init */
    AtVector Ng, Ns;

    LayerStackBSDF() : 
        albedos(0), etas(0), kappas(0), alphas(0), depths(0), sigma_a(0), sigma_s(0), 
        N(), wo(), Ng(), Ns() 
    {
        etas.push_back(1.0f);
        kappas.push_back(0.0f);
    }

    LayerStackBSDF(const LayerStackBSDF& b) :
        albedos(b.albedos), etas(b.etas), kappas(b.kappas), alphas(b.kappas), 
        depths(b.depths), sigma_a(b.sigma_a), sigma_s(b.sigma_s), 
        N(b.N), wo(b.wo), Ng(b.Ng), Ns(b.Ns)
    {}

    LayerStackBSDF& operator=(const LayerStackBSDF& b) {
        albedos.clear();
        albedos.assign(b.albedos.begin(), b.albedos.end());
        etas.clear();
        etas.assign(b.etas.begin(), b.etas.end());
        kappas.clear();
        kappas.assign(b.kappas.begin(), b.kappas.end());
        alphas.clear();
        alphas.assign(b.alphas.begin(), b.alphas.end());
        depths.clear();
        depths.assign(b.depths.begin(), b.depths.end());
        sigma_a.clear();
        sigma_a.assign(b.sigma_a.begin(), b.sigma_a.end());
        sigma_s.clear();
        sigma_s.assign(b.sigma_s.begin(), b.sigma_s.end());
        N = b.N;
        wo = b.wo;
        Ng = b.Ng;
        Ns = b.Ns;
        return *this;
    }
};

AtBSDF* LayerStackBSDFCreate(const AtShaderGlobals* sg, const LayerStackBSDF& lsbsdf);

void computeAddingDoubling(
    float cosNI, const int nb_layers,
    const std::vector<AtRGB>& m_albedos,
    const std::vector<float>& m_etas,
    const std::vector<float>& m_kappas,
    const std::vector<float>& m_alphas,
    const std::vector<float>& m_depths,
    const std::vector<AtRGB>& m_sigma_a,
    const std::vector<AtRGB>& m_sigma_s,
    AtRGB* coeffs,
    float* alphas,
    int& nb_valid);

