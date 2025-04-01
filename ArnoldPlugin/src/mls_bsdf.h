#pragma once
#include "util.h"

struct LayerStackBSDF
{
    std::vector<AtRGB> albedos;
    std::vector<float> etas;
    std::vector<float> kappas;
    std::vector<float> alphas;

    // geometry, don't need to set at creating bsdf
    /* parameters */
    AtVector N, wo;
    /* set in bsdf_init */
    AtVector Ng, Ns;

    LayerStackBSDF() : albedos(0), etas(0), kappas(0), alphas(0), N(), wo(), Ng(), Ns() {
        etas.push_back(1.0f);
        kappas.push_back(0.0f);
    }

    LayerStackBSDF(const LayerStackBSDF& b) :
        albedos(b.albedos), etas(b.etas), kappas(b.kappas), alphas(b.kappas), N(b.N), wo(b.wo), Ng(b.Ng), Ns(b.Ns) 
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
    AtRGB* coeffs,
    float* alphas,
    int& nb_valid);

