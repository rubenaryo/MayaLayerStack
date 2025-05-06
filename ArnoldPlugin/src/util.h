#pragma once

#include <ai.h>
#include <ai_shader_bsdf.h>
#include <ai_shaderglobals.h>
#include <cmath>
#include <vector>

#define USE_BEST_FIT

template<typename T>
inline T _clamp(T x, T m, T M) {
    return std::min<T>(std::max<T>(x, m), M);
}
template<typename T>
inline T sqr(T x) {
    return x * x;
}

inline bool isInvalid(AtRGB c) {
	return (c.r < 0 || c.g < 0 || c.b < 0 || isnan(c.r) || isnan(c.g) || isnan(c.b));
}

inline AtVector2 ToConcentricDisk(AtVector2 uv) {
	if (uv.x == 0.0f && uv.y == 0.0f)
		return AtVector2(0.f, 0.f);

	AtVector2 v = uv * 2.0f - 1.0f;

	float phi, r;
	if (v.x * v.x > v.y * v.y)
	{
		r = v.x;
		phi = AI_PI * v.y / v.x * 0.25f;
	}
	else
	{
		r = v.y;
		phi = AI_PI * 0.5f - AI_PI * v.x / v.y * 0.25f;
	}
	return AtVector2(r * std::cos(phi), r * std::sin(phi));
}

inline AtVector reflect(const AtVector& wo, const AtVector& n) {
	return AiV3Dot(wo, n) * 2 * n - wo;
}

inline float average(const AtVector& v) {
    return 0.333333333333333 * (v.x + v.y + v.z);
}

inline float average(const AtRGB& v) {
    return 0.333333333333333 * (v.r + v.g + v.b);
}

inline AtBSDFLobeMask LobeMask(int idx) {
    return 1 << idx;
}

inline AtRGB AiRGBExp(const AtRGB& c) {
	return AtRGB(expf(c.r), expf(c.g), expf(c.b));
}

inline float roughnessToVariance(float a) {
#ifdef USE_BEST_FIT
    a = _clamp<float>(a, 0.0, 0.9999);
    float a3 = powf(a, 1.1);
    return a3 / (1.0f - a3);
#else
    return a / (1.0f - a);
#endif
}

inline float varianceToRoughness(float v) {
#ifdef USE_BEST_FIT
    return powf(v / (1.0f + v), 1.0f / 1.1f);
#else
    return v / (1.0f + v);
#endif
}

inline float gToVariance(float g) {
	g = _clamp<float>(g, 0.0001, 1.0);
	return powf((1 - g) / g, 0.8f) / (1 + g);
}

inline void computeSigma(AtRGB albedo, float ld, AtRGB& sigma_a, AtRGB& sigma_s) {
	AtRGB sigma_t = AtRGB(1 / ld);
	sigma_s = albedo * sigma_t;
	sigma_a = sigma_t - sigma_s;
}

struct Vec2c
{
	Vec2c(float real, float img) : real(real), img(img) {}

	Vec2c operator + (const Vec2c& r) const
	{
		return Vec2c(real + r.real, img + r.img);
	}

	Vec2c operator - (const Vec2c& r) const
	{
		return Vec2c(real - r.real, img - r.img);
	}

	Vec2c operator * (const Vec2c& r) const
	{
		float pReal = real * r.real;
		float pImg = img * r.img;
		return Vec2c(pReal - pImg, pReal + pImg);
	}

	Vec2c operator * (float v) const
	{
		return Vec2c(real * v, img * v);
	}

	Vec2c operator / (const Vec2c& r) const
	{
		float pReal = real * r.real;
		float pImg = img * r.img;
		float scale = 1.f / r.LengthSqr();
		return Vec2c(pReal + pImg, pReal - pImg) * scale;
	}

	Vec2c Sqrt() const
	{
		float n = std::sqrt(LengthSqr());
		float t1 = std::sqrt(.5f * (n + std::abs(real)));
		float t2 = .5f * img / t1;

		if (n == 0)
			return Vec2c(0.f, 0.f);

		if (real >= 0)
			return Vec2c(t1, t2);
		else
			return Vec2c(std::abs(t2), std::copysign(t1, img));
	}

	float LengthSqr() const
	{
		return real * real + img * img;
	}

	float real;
	float img;
};