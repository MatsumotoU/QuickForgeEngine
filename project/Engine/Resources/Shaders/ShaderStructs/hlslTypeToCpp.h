#pragma once

#ifdef __cplusplus
#include <stdint.h>
#include "Core/Math/Vector/Vector4.h"
#include "Core/Math/Vector/Vector3.h"
#include "Core/Math/Vector/Vector2.h"
#include "Core/Math/Matrix/Matrix4x4.h"
#include "Core/Math/Matrix/Matrix3x3.h"
#include "Core/Math/Matrix/Matrix2x2.h"

using float32_t4 = Vector4;
using float32_t3 = Vector3;
using float32_t2 = Vector2;
using float32_t = float;
using float32_t4x4 = Matrix4x4;
using float32_t3x3 = Matrix3x3;
using float32_t2x2 = Matrix2x2;
#endif

struct EchoSphere {
#ifdef __cplusplus
	EchoSphere()
		: sphereCenter{ 0.0f, 0.0f, 0.0f }
		, sphereRadius(0.0f)
		, sphereThickness(0.2f) {}
#endif
	float32_t3 sphereCenter;
	float32_t  sphereRadius;
	float32_t  sphereThickness;
};

struct ColorCorrectionOffset {
#ifdef __cplusplus
	ColorCorrectionOffset()
		: exposure(1.0f)
		, contrast(1.0f)
		, saturation(1.0f)
		, gamma(1.0f)
		, hue(0.0f) {
	}
#endif

	float32_t exposure;
	float32_t contrast;
	float32_t saturation;
	float32_t gamma;
	float32_t hue;
#ifdef __cplusplus
	float32_t padding[3];
#endif
};

struct DirectionalLight {
#ifdef __cplusplus
	DirectionalLight()
		: color{ 1.0f, 1.0f, 1.0f, 1.0f }
		, direction{ 0.0f, -1.0f, 0.0f }
		, intensity(1.0f) {
	}
#endif

	float32_t4 color;
	float32_t3 direction;
	float32_t intensity;
};

struct Material {
#ifdef __cplusplus
	Material()
		: color{ 1.0f, 1.0f, 1.0f, 1.0f }
		, enableLighting(1)
		, padding{ 0.0f, 0.0f, 0.0f }
		, uvTransform(Matrix4x4::MakeIndentity4x4()) {
	}
#endif

	float32_t4 color;
	int32_t enableLighting;
#ifdef __cplusplus
	float32_t padding[3];
#endif
	float32_t4x4 uvTransform;
};

struct OffsetBuffer {
#ifdef __cplusplus
	OffsetBuffer()
		: offset{ 0.0f, 0.0f, 0.0f, 0.0f } {
	}
#endif

	float32_t4 offset;
};

struct PixcelOffset {
#ifdef __cplusplus
	PixcelOffset()
		: screenResolution{ 1280.0f, 720.0f }
		, pixcelSize(0.001f)
		, time(0.0f) {
	}
#endif
	float32_t2 screenResolution;
	float32_t pixcelSize;
	float32_t time;
};

struct ParticleForGPU {
#ifdef __cplusplus
	ParticleForGPU()
		: WVP(Matrix4x4::MakeIndentity4x4())
		, World(Matrix4x4::MakeIndentity4x4())
		, color{ 1.0f,1.0f,1.0f,1.0f } {
	}
#endif 

	float32_t4x4 WVP;
	float32_t4x4 World;
	float32_t4 color;
};

struct PrimitiveVertexData {
#ifdef __cplusplus
	PrimitiveVertexData()
		: position{ 0.0f, 0.0f, 0.0f, 1.0f }
		, texcoord{ 0.0f, 0.0f }
		, color{ 1.0f, 1.0f, 1.0f, 1.0f } {
	}
#endif

	float32_t4 position;
	float32_t4 color;
	float32_t2 texcoord;	
};

struct TransformationMatrix {
#ifdef __cplusplus
	TransformationMatrix()
		: WVP(Matrix4x4::MakeIndentity4x4())
		, World(Matrix4x4::MakeIndentity4x4()) {
	}
#endif

	float32_t4x4 WVP;
	float32_t4x4 World;
};

struct VertexData {
#ifdef __cplusplus
	VertexData()
		: position{ 0.0f, 0.0f, 0.0f, 1.0f }
		, texcoord{ 0.0f, 0.0f }
		, normal{ 0.0f, 0.0f, 1.0f }
		, padding{0.0f,0.0f,0.0f} {
	}
#endif

	float32_t4 position;
	float32_t2 texcoord;
	float32_t3 normal;
#ifdef __cplusplus
	float32_t padding[3];
#endif
};

struct VignetteOffset {
#ifdef __cplusplus
	VignetteOffset()
		: screenResolution{ 1280.0f, 720.0f }
		, VignetteRadius(0.75f)
		, VignetteSoftness(0.25f)
		, VignetteIntensity(1.0f)
		, padding1(0.0f)
		, padding2(0.0f)
		, padding3(0.0f){
	}
#endif

	float32_t2 screenResolution;
	float32_t VignetteRadius;
	float32_t VignetteSoftness;
	float32_t VignetteIntensity;
#ifdef __cplusplus
	float32_t padding1;
	float32_t padding2;
	float32_t padding3;
#endif
};

struct Constants {
	float32_t2 AtlasSize;
	float32_t DistanceRange;
#ifdef __cplusplus
	float32_t padding;
#endif
};

struct GlyphForGPU
{
	float32_t4x4 WVP;
	float32_t4 texCoords;
};