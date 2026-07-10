#pragma once

#ifdef __cplusplus
#include <stdint.h>
#include "math/vector/Vector4.h"
#include "math/vector/Vector3.h"
#include "math/vector/Vector2.h"
#include "math/matrix/Matrix4x4.h"
#include "math/matrix/Matrix3x3.h"
#include "math/matrix/Matrix2x2.h"

using float32_t4 = QFE::MATH::Vector4;
using float32_t3 = QFE::MATH::Vector3;
using float32_t2 = QFE::MATH::Vector2;
using float32_t = float;
using float32_t4x4 = QFE::MATH::Matrix4x4;
using float32_t3x3 = QFE::MATH::Matrix3x3;
using float32_t2x2 = QFE::MATH::Matrix2x2;
#endif

struct SphireForGPU {
	#ifdef __cplusplus
	SphireForGPU()
		: center{ 0.0f, 0.0f, 0.0f }
		, radius(1.0f) {
	}
#endif
	float32_t3 center;
	float32_t radius;
};

struct CameraForGPU {
#ifdef __cplusplus
	CameraForGPU()
		: cameraPosition{ 0.0f, 0.0f, 0.0f }
		, padding(0.0f) {
	}
#endif
	float32_t3 cameraPosition;
	float32_t padding;
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
		, uvTransform(QFE::MATH::Matrix4x4::MakeIndentity4x4())
		, shininess(0.0f){
	}
#endif

	float32_t4 color;
	int32_t enableLighting;
#ifdef __cplusplus
	float32_t padding[3];
#endif
	float32_t4x4 uvTransform;
	float32_t shininess;
};

struct OffsetBuffer {
#ifdef __cplusplus
	OffsetBuffer()
		: offset{ 0.0f, 0.0f, 0.0f, 0.0f } {
	}
#endif

	float32_t4 offset;
};

struct BoxFilterOffset {
#ifdef __cplusplus
	BoxFilterOffset()
		: screenResolution{ 1280.0f, 720.0f } {
	}
#endif
	float32_t2 screenResolution;
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

struct Particle {
#ifdef __cplusplus
	Particle()
		: translate{ 0.0f, 0.0f, 0.0f }
		, scale{ 1.0f, 1.0f, 1.0f } {
	}
#endif 
	float32_t3 translate;
	float32_t3 scale;
	float32_t lifeTime;
	float32_t3 velocity;
	float32_t currentTime;
	float32_t4 color;
};

struct ParView {
#ifdef __cplusplus
	ParView()
		: viewProjection(QFE::MATH::Matrix4x4::MakeIndentity4x4())
		, billboardMatrix(QFE::MATH::Matrix4x4::MakeIndentity4x4()) {
	}
#endif
	float32_t4x4 viewProjection;
	float32_t4x4 billboardMatrix;
};

struct ParticleForGPU {
#ifdef __cplusplus
	ParticleForGPU()
		: WVP(QFE::MATH::Matrix4x4::MakeIndentity4x4())
		, World(QFE::MATH::Matrix4x4::MakeIndentity4x4())
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
		: WVP(QFE::MATH::Matrix4x4::MakeIndentity4x4())
		, World(QFE::MATH::Matrix4x4::MakeIndentity4x4()) {
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
