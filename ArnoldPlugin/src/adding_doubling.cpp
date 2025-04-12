#include "util.h"
#include <fstream>

struct TIR {
    TIR(const std::string& path) {
        std::ifstream in(path);
        // Warn if not loaded
        if (in.bad() || in.fail()) {
            return;
        }
        ok = true;
        this->load(in);
    }

    void load(std::ifstream& in) {
        // Read sizes
        in.read((char*)&Nt, sizeof(int));
        in.read((char*)&Na, sizeof(int));
        in.read((char*)&Nn, sizeof(int));
        //SLog(EInfo, "Loading TIR texture of dimension %dx%dx%dx", Nt, Na, Nn);
        sizes[0] = Nt;
        sizes[1] = Na;
        sizes[2] = Nn;

        // Read data range (min / max)
        float mM[6];
        in.read((char*)mM, 6 * sizeof(float));
        tm = mM[0]; tM = mM[1];
        am = mM[2]; aM = mM[3];
        nm = mM[4]; nM = mM[5];
        //SLog(EInfo, "Range [%f, %f; %f, %f; %f, %f]", tm, tM, am, aM, nm, nM);

        // Read data
        const int size = Nt * Na * Nn;
        buff.assign(size, 0.0f);
        in.read((char*)buff.data(), size * sizeof(float));
    }

    inline float operator() (float t, float a, float n) const {

        // Floating point index
        float ta = Nt * (t - tm) / (tM - tm);
        float aa = Na * (a - am) / (aM - am);
        float na = Nn * (n - nm) / (nM - nm);

        // Integer index
        int ti = floor(ta);
        int ai = floor(aa);
        int ni = floor(na);

        // Ensure the indexes stays in the limits
        ti = _clamp<int>(ti, 0, Nt - 1);
        ai = _clamp<int>(ai, 0, Na - 1);
        ni = _clamp<int>(ni, 0, Nn - 1);

        //*
        // Clamp the interpolation weights
        float alphas[3] = { ta - ti, aa - ai, na - ni };
        for (int i = 0; i < 3; ++i) {
            alphas[i] = _clamp<float>(alphas[i], 0.0f, 1.0f);
        }

        // Index of the middle point
        const int indices[3] = { ti, ai, ni };
        const int index = ni + Nn * (ai + Na * ti);

        // Result vector and norm
        float v = 0.0f;
        float V = 0.0f;

        // For every possible combinaison of index shift per dimension,
        // fetch the value in memory and do linear interpolation.
        // We fetch using shift of 0 and 1.
        //
        //     v(i+di, j+di, k+dk, l+dl),  where dk in [0,1]
        //
        const unsigned int D = pow(2, 3);
        for (unsigned int d = 0; d < D; ++d) {

            float alpha = 1.0; // Global alpha
            int   cid_s = 0;   // Id shift

            // Evaluate the weight of the sample d which correspond to
            // one for the shifted configuration:
            // The weight is the product of the weights per dimension.
            //
            for (int i = 0; i < 3; ++i) {
                bool  bitset = ((1 << i) & d);
                float calpha = (bitset) ? alphas[i] : 1.0 - alphas[i];

                // Correct the shift to none if we go out of the grid
                if (indices[i] + 1 >= sizes[i]) {
                    bitset = false;
                }

                alpha *= calpha;
                cid_s = cid_s * sizes[i] + ((bitset) ? 1 : 0);
            }

            const float tmp = buff[index + cid_s];
            if (!std::isnan(tmp)) {
                v += alpha * tmp;
                V += alpha;
            }
        }

        return v;
    }

private:
    std::vector<float> buff;
    int sizes[3];
    int Nt, Na, Nn;
    float tm, tM, aM, am, nm, nM;
public:
    bool ok = false;
};

TIR m_TIR("TIR.bin");

float fresnelDielectric(float cosTi, float eta) {
    cosTi = AiClamp(cosTi, -1.f, 1.f);
    if (cosTi < 0.0f)
    {
        eta = 1.f / eta;
        cosTi = -cosTi;
    }

    float sinTi = sqrtf(1.f - cosTi * cosTi);
    float sinTt = sinTi / eta;
    if (sinTt >= 1.f)
        return 1.f;

    float cosTt = sqrtf(1.f - sinTt * sinTt);

    float rPa = (cosTi - eta * cosTt) / (cosTi + eta * cosTt);
    float rPe = (eta * cosTi - cosTt) / (eta * cosTi + cosTt);
    return (rPa * rPa + rPe * rPe) * .5f;
}

float fresnelConductor(float cosI, float eta, float k) {
    Vec2c etak(eta, k);
    Vec2c cosThetaI(AiClamp(cosI, 0.f, 1.f), 0.f);

    Vec2c sin2ThetaI(1.f - cosThetaI.LengthSqr(), 0.f);
    Vec2c sin2ThetaT = sin2ThetaI / (etak * etak);
    Vec2c cosThetaT = (Vec2c(1.f, 0.f) - sin2ThetaT).Sqrt();

    Vec2c rPa = (etak * cosThetaI - cosThetaT) / (etak * cosThetaI + cosThetaT);
    Vec2c rPe = (cosThetaI - etak * cosThetaT) / (cosThetaI + etak * cosThetaT);
    return (rPa.LengthSqr() + rPe.LengthSqr()) * .5f;
}

void evalFresnel(float ct, const AtRGB& albedo, float alpha, float eta, float kappa,
    AtRGB& Rij, AtRGB& Tij) {
    Rij = (kappa == 0.0f) ? fresnelDielectric(ct, eta) * AtRGB(1.0f) :
        albedo * fresnelConductor(ct, eta, kappa);
    Tij = (kappa == 0.0f) ? (AtRGB(1.0) - Rij) * albedo : AtRGB(0.0);
}

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
    int &nb_valid) {

    // Variables
    float cti = cosNI;
    AtRGB R0i(0.0f), Ri0(0.0f), T0i(1.0f), Ti0(1.0f);
    float s_r0i = 0.0f, s_ri0 = 0.0f, s_t0i = 0.0f, s_ti0 = 0.0f;
    float j0i = 1.0f, ji0 = 1.0f;

    // Iterate over the layers
    for (int i = 0; i < nb_layers; ++i) {

        /* Extract layer data */
        float eta_1 = m_etas[i];
        float eta_2 = m_etas[i + 1];
        float kappa_2 = m_kappas[i + 1];
        float eta = eta_2 / eta_1;
        float kappa = kappa_2 / eta_1;
        float alpha = m_alphas[i];
        float n12 = eta;
        float depth = m_depths[i];
        //float depth = 0.0f;

        AtRGB R12, T12, R21, T21;
        float s_r12 = 0.0f, s_r21 = 0.0f, s_t12 = 0.0f, s_t21 = 0.0f, j12 = 1.0f, j21 = 1.0f, ctt;
        if (depth > 0.0f) {
            /* Mean doesn't change with volumes */
            ctt = cti;

            /* Evaluate transmittance */
            const AtRGB sigma_t = m_sigma_a[i] + m_sigma_s[i];
            T12 = (AtRGB(1.0f) + m_sigma_s[i] * depth / ctt) * AiRGBExp(-(depth / ctt) * sigma_t);
            //T12 = (AtRGB(1.0f) + AtRGB(100.0F) * 0.001f / ctt) * AiRGBExp(-(0.001F / ctt) * AtRGB(100.F));
            //T12 = AtRGB(1.0f);
            T21 = T12;
            R12 = AtRGB(0.0f);
            R21 = AtRGB(0.0f);

            /* Fetch precomputed variance for HG phase function */
            s_t12 = alpha;
            s_t21 = alpha;

        }
        else {
            /* Evaluate off-specular transmission */
            float sti = sqrt(1.0f - cti * cti);
            float stt = sti / n12;
            if (stt <= 1.0f) {
                //const float scale = _clamp<float>((1.0f-alpha)*(sqrt(1.0f-alpha) + alpha), 0.0f, 1.0f);
                //stt = scale*stt + (1.0f-scale)*sti;
                ctt = sqrt(1.0f - stt * stt);
            }
            else {
                ctt = -1.0f;
            }

            /* Ray is not block by conducting interface or total reflection */
            const bool has_transmissive = ctt > 0.0f && kappa == 0.f;

            /* Evaluate interface variance term */
            s_r12 = roughnessToVariance(alpha);
            s_r21 = s_r12;

            /* For dielectric interfaces, evaluate the transmissive roughnesses */
            if (has_transmissive) {
                const float _ctt = 1.0f; // The scaling factor overblurs the BSDF at grazing
                const float _cti = 1.0f; // angles (we cannot account for the deformation of
                // the lobe for those configurations.

                s_t12 = roughnessToVariance(alpha * 0.5f * fabs(_ctt * n12 - _cti) / (_ctt * n12));
                s_t21 = roughnessToVariance(alpha * 0.5f * fabs(_cti / n12 - _ctt) / (_cti / n12));
                j12 = (ctt / cti) * n12; // Scale due to the interface
                j21 = (cti / ctt) / n12;
            }

            /* Evaluate FGD using a modified roughness accounting for top layers */
            auto temp_alpha = varianceToRoughness(s_t0i + s_r12);

            /* Evaluate r12, r21, t12, t21 */
            evalFresnel(cti, m_albedos[i], temp_alpha, eta, kappa, R12, T12);
            if (has_transmissive) {
                R21 = R12;
                T21 = T12 /* (n12*n12) */; // We don't need the IOR scaling since we are
            }
            else {
                R21 = AtRGB(0.0f);
                T21 = AtRGB(0.0f);
                T12 = AtRGB(0.0f);
            }

            /* Evaluate TIR using the decoupling approximation */
            if (i > 0 && m_TIR.ok) {
                float eta_0 = m_etas[i - 1];
                float n10 = (eta_0 / eta_1);

                const float _TIR = m_TIR(cti, temp_alpha, n10);
                Ri0 += (1.0f - _TIR) * Ti0;
                Ri0 = AiRGBClamp(Ri0, 0.0, 1.0);
                Ti0 *= _TIR;
            }
        }

        /* Multiple scattering forms */
        const AtRGB denom = (AtRGB(1.0f) - Ri0 * R12);
        const AtRGB m_R0i = (average(denom) <= 0.0f) ? AtRGB(0.0f) : (T0i * R12 * Ti0) / denom;
        const AtRGB m_Ri0 = (average(denom) <= 0.0f) ? AtRGB(0.0f) : (T21 * Ri0 * T12) / denom;
        const AtRGB m_Rr = (average(denom) <= 0.0f) ? AtRGB(0.0f) : (Ri0 * R12) / denom;

        /* Evaluate the adding operator on the energy */
        const AtRGB e_R0i = R0i + m_R0i;
        const AtRGB e_T0i = (T0i * T12) / denom;
        const AtRGB e_Ri0 = R21 + m_Ri0;
        const AtRGB e_Ti0 = (T21 * Ti0) / denom;

        /* Scalar forms for the spectral quantities */
        const float r0i = average(R0i);
        const float e_r0i = average(e_R0i);
        const float e_ri0 = average(e_Ri0);
        const float m_r0i = average(m_R0i);
        const float m_ri0 = average(m_Ri0);
        const float m_rr = average(m_Rr);
        const float r21 = average(R21);

        /* Evaluate the adding operator on the normalized variance */
        float _s_r0i = (r0i * s_r0i + m_r0i * (s_ti0 + j0i * (s_t0i + s_r12 + m_rr * (s_r12 + s_ri0))));// e_r0i;
        float _s_t0i = j12 * s_t0i + s_t12 + j12 * (s_r12 + s_ri0) * m_rr;
        float _s_ri0 = (r21 * s_r21 + m_ri0 * (s_t12 + j12 * (s_t21 + s_ri0 + m_rr * (s_r12 + s_ri0))));// e_ri0;
        float _s_ti0 = ji0 * s_t21 + s_ti0 + ji0 * (s_r12 + s_ri0) * m_rr;
        _s_r0i = (e_r0i > 0.0f) ? _s_r0i / e_r0i : 0.0f;
        _s_ri0 = (e_ri0 > 0.0f) ? _s_ri0 / e_ri0 : 0.0f;

        /* Store the coefficient and variance */
        if (m_r0i > 0.0f) {
            coeffs[i] = m_R0i;
            alphas[i] = varianceToRoughness(s_ti0 + j0i * (s_t0i + s_r12 + m_rr * (s_r12 + s_ri0)));
        }
        else {
            coeffs[i] = AtRGB(0.0f);
            alphas[i] = 0.0f;
        }

        /* Update energy */
        R0i = e_R0i;
        T0i = e_T0i;
        Ri0 = e_Ri0;
        Ti0 = e_Ti0;

        /* Update mean */
        cti = ctt;

        /* Update variance */
        s_r0i = _s_r0i;
        s_t0i = _s_t0i;
        s_ri0 = _s_ri0;
        s_ti0 = _s_ti0;

        /* Update jacobian */
        j0i *= j12;
        ji0 *= j21;

        /* Escape if a conductor is present */
        if (kappa > 0.f) {
            nb_valid = i + 1;
            return;
        }
    }

    nb_valid = nb_layers;
}