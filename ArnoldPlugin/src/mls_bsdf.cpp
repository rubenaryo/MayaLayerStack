#include "mls_bsdf.h"
#include "microfacet.h"
#include "util.h"
#include "randoms.h"

AI_BSDF_EXPORT_METHODS(LayerStackBSDFMtd);

bsdf_init
{
    LayerStackBSDF* data = (LayerStackBSDF*)AiBSDFGetData(bsdf);

    // store forward facing smooth normal for bump shadowing
    data->Ns = (sg->Ngf == sg->Ng) ? sg->Ns : -sg->Ns;
    
    // store geometric normal to clip samples below the surface
    data->Ng = sg->Ngf;

    data->N = sg->Nf;
    data->wo = -sg->Rd;
    
    // initialize the BSDF lobes. in this case we just have a single
    // diffuse lobe with no specific flags or label
    static const AtBSDFLobeInfo lobe_info[1] = { 
        {AI_RAY_DIFFUSE_REFLECT, 0, AtString()}
    };
    AiBSDFInitLobes(bsdf, lobe_info, 1);
    
    // specify that we will only reflect light in the hemisphere around N
    AiBSDFInitNormal(bsdf, data->N, true);
}

bsdf_sample
{
    LayerStackBSDF* data = (LayerStackBSDF*)AiBSDFGetData(bsdf);

    // discard rays below the hemisphere
    const float cosNO = AiV3Dot(data->N, data->wo);
    if (cosNO <= 0.f)
        return AI_BSDF_LOBE_MASK_NONE;

    // compute coeffs and alphas using adding-doubling
    AtRGB coeffs[2];
    float alphas[2];
    int nb_valid = 0;
    computeAddingDoubling(cosNO, 2, data->albedos, data->etas, data->kappas, data->alphas,
        coeffs, alphas, nb_valid);

    /* Convert Spectral coefficients to floats to select BRDF lobe to sample */
    std::vector<float> weights(nb_valid);
    float cum_w = 0.0;
    for (int i = 0; i < nb_valid; ++i) {
        weights[i] = average(coeffs[i]);
        cum_w += weights[i];
    }

    /* Select a random BRDF lobe */
    float sel_w = rand1() * cum_w - weights[0];
    int sel_i = 0;
    for (sel_i = 0; sel_w > 0.0 && sel_i < nb_valid; sel_i++) {
        sel_w -= weights[sel_i + 1];
    }

    // compute wi
    AtVector m = sampleGGX(rnd, alphas[sel_i]);
    AtVector U, V;
    AiV3BuildLocalFrame(U, V, data->N);
    AtVector m_World = m.x * U + m.y * V + m.z * data->N;
    AtVector wi = reflect(data->wo, m_World);
    const float cosNI = AiV3Dot(data->N, wi);

    if (cosNI <= 0.0f) {
        return AI_BSDF_LOBE_MASK_NONE;
    }
    
    // F
    AtRGB f(0.0f, 0.0f, 0.0f);

    /* Evaluate the MIS 'pdf' using the balance heuristic */
    float pdf = 0.0;
    for (int i = 0; i < nb_valid; ++i) {
        // Fetch current roughness
        const float a = alphas[i];

        // Evaluate microfacet model
        const float D = distributionGGX(AiV3Dot(data->N, m_World), a);
        const float G = geometrySmith(cosNI, cosNO, a);
        const float G1 = smithShlickGGX(cosNI, a);

        // f
        if (!AiColorIsSmall(coeffs[i])) {
            f += D * G * coeffs[i] / (4.0f * cosNO);
        }

        // pdf
        float DG1 = D * G1 / (4.0f * cosNO);
        pdf += (weights[i] / cum_w) * DG1;
    }

    if (pdf <= 0.0f) {
        return AI_BSDF_LOBE_MASK_NONE;
    }

    // return output direction vectors, we don't compute differentials here
    out_wi = AtVectorDv(wi);

    // specify that we sampled the first (and only) lobe
    out_lobe_index = 0;

    // return weight and pdf
    out_lobes[out_lobe_index] = AtBSDFLobeSample(f / pdf, 0, pdf);
    
    // indicate that we have valid lobe samples for all the requested lobes,
    // which is just one lobe in this case
    return lobe_mask;
}

bsdf_eval
{
    LayerStackBSDF* data = (LayerStackBSDF*)AiBSDFGetData(bsdf);

    // discard rays below the hemisphere
    const float cosNI = AiV3Dot(data->N, wi);
    const float cosNO = AiV3Dot(data->N, data->wo);
    if (cosNI <= 0.f || cosNO <= 0.f)
       return AI_BSDF_LOBE_MASK_NONE;

    // half
    AtVector H = AiV3Normalize(data->wo + wi);

    // result
    AtRGB f(0.0f, 0.0f, 0.0f);
    float pdf = 0.0;
    float cum_w = 0.0;

    // compute coeffs and alphas using adding-doubling
    AtRGB coeffs[2];
    float alphas[2];
    int nb_valid = 0;
    computeAddingDoubling(cosNO, 2, data->albedos, data->etas, data->kappas, data->alphas,
        coeffs, alphas, nb_valid);

    /* Sum the contribution of all the interfaces */
    for (int i = 0; i < nb_valid; ++i) {

        // Skip zero contributions
        if (AiColorIsSmall(coeffs[i])) {
            continue;
        }

        // Fetch current roughness
        const float a = alphas[i];

        // Evaluate microfacet model
        const float D = distributionGGX(AiV3Dot(data->N, H), a);
        const float G = geometrySmith(cosNI, cosNO, a);
        const float G1 = smithShlickGGX(cosNI, a);

        // Add to the contribution
        f += D * G * coeffs[i] / (4.0f * cosNO);

        // pdf
        float weight = average(coeffs[i]);
        cum_w += weight;
        float DG1 = D * G1 / (4.0f * cosNO);
        pdf += weight * DG1;
    }

    if (cum_w > 0.0f) {
        pdf /= cum_w;
    }
    else {
        return AI_BSDF_LOBE_MASK_NONE;
    }
    
    // return weight and pdf, same as in bsdf_sample
    int lobe_index = 0;
    out_lobes[lobe_index] = AtBSDFLobeSample(f / pdf, 0, pdf);
    
    return lobe_mask;
}

AtBSDF* LayerStackBSDFCreate(const AtShaderGlobals* sg, const LayerStackBSDF& lsbsdf)
{
    AtBSDF* bsdf = AiBSDF(sg, AI_RGB_WHITE, LayerStackBSDFMtd, sizeof(LayerStackBSDF));
    LayerStackBSDF* data = (LayerStackBSDF*)AiBSDFGetData(bsdf);
    new(data) LayerStackBSDF();
    *data = lsbsdf;
    return bsdf;
}