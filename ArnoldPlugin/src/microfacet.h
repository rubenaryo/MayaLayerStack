#pragma once
#include <ai.h>

float distributionGGX(float cosTheta, float alpha)
{
    if (cosTheta < 1e-6f)
        return 0.0f;

    float a2 = alpha * alpha;
    float nom = a2;
    float denom = cosTheta * cosTheta * (a2 - 1.0f) + 1.0f;
    denom = denom * denom * AI_PI;

    return nom / denom;
}

AtVector sampleGGX(AtVector2 rng, float alpha)
{
    AtVector2 p = ToConcentricDisk(rng);
    AtVector wh = AtVector(p.x, p.y, sqrtf(1.f - AiV2Dot(p, p)));
    return AiV3Normalize(wh * AtVector(alpha, alpha, 1.f));
}

float smithShlickGGX(float cosTheta, float alpha)
{
    float k = alpha * 0.5f;
    return cosTheta / (cosTheta * (1.0f - k) + k);
}

float geometrySmith(float cosThetaO, float cosThetaI, float alpha)
{
    return smithShlickGGX(std::abs(cosThetaO), alpha) * smithShlickGGX(std::abs(cosThetaI), alpha);
}

