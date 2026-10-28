#pragma once

#ifdef __cplusplus
#include <stdint.h>
#include <cstdint>
#include <cstddef>
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

struct LineDrawParameters {
	float32_t4x4 worldMatrix;
	float32_t4 color;
	float32_t width;
};

// ライン描画用の頂点バッファ。セマンティクスは HLSL 側だけに付ける。
struct LineVertex {
#ifdef __cplusplus
	float32_t4 position;
	float32_t4 color;
#else
	float32_t4 position : POSITION0;
	float32_t4 color : COLOR0;
#endif
};

// レイトレーシング用平行光の StructuredBuffer と定数バッファ。
struct DirectionalLightGpu {
	float32_t4 directionAndIntensity; // xyz: 光の進行方向、w: 強さ
};

struct DirectionalLightConstants {
	uint32_t count;
};

// レイトレーシング反射のためのインスタンス情報
struct RaytracingInstanceMeta {
#ifdef __cplusplus
	RaytracingInstanceMeta()
		: materialIndex(1u)
		, vertexBase(0u)
		, vertexCount(0u)
		, primitiveBase(0u)
		, baseColor{ 1.0f, 1.0f, 1.0f, 1.0f }
		, uvTransform(QFE::MATH::Matrix4x4::MakeIdentity4x4())
		, metallic(0.0f)
		, smoothness(0.0f)
		, padding{ 1.0f, 0.0f } {}
#endif
	uint32_t materialIndex;
	uint32_t vertexBase;     // global UV / vertex 配列の先頭オフセット（頂点単位）
	uint32_t vertexCount;    // 頂点数（必要なら）
	uint32_t primitiveBase;  // global primitive 配列の先頭オフセット（三角形単位）
	float32_t4 baseColor;
#ifdef __cplusplus
	float32_t4x4 uvTransform;
#else
	row_major float32_t4x4 uvTransform;
#endif
	float32_t metallic;
	float32_t smoothness;
#ifdef __cplusplus
	float32_t padding[2];
#else
	float32_t2 padding;
#endif
};

#ifdef __cplusplus
// 従来の C++ API 名も引き続き使用できる。
using InstanceMetaCPU = RaytracingInstanceMeta;
#endif

// レイトレーシングのヒット点で補間する頂点属性。
// StructuredBuffer のストライドを C++ / HLSL 間で一致させる。
struct RaytracingVertexAttribute {
	float32_t2 texcoord;
	float32_t3 normal;
};

#ifdef __cplusplus
static_assert(sizeof(RaytracingVertexAttribute) == 20);
static_assert(sizeof(LineVertex) == 32);
static_assert(sizeof(DirectionalLightGpu) == 16);
static_assert(sizeof(DirectionalLightConstants) == 4);
static_assert(sizeof(RaytracingInstanceMeta) == 112);
static_assert(offsetof(RaytracingInstanceMeta, uvTransform) == 32);
static_assert(offsetof(RaytracingInstanceMeta, padding) == 104);
#endif

struct FrameTimeParameters {
	float32_t time;
	float32_t deltaTime;
};

struct ParticleSphereEmitterParameters {
	float32_t3 translate;
	float32_t radius;
	uint32_t count;
	float32_t frequency;
	float32_t frequencyTime;
	uint32_t emit;
};

struct SkinningPaletteEntry {
	float32_t4x4 skeletonSpaceMatrix;
	float32_t4x4 skeletonSpaceInverseTransposeMatrix;
};

struct OutlineOffset {
	float32_t4x4 viewProjectionInverse;
};

struct SkinningVertex {
	float32_t4 position;
	float32_t2 texcoord;
	float32_t3 normal;
};

struct SkinningVertexInfluence {
	float32_t weight;
	int32_t index[4];
};

struct SkinningDispatchParameters {
	uint32_t numVertices;
};

struct CameraForGPU {
#ifdef __cplusplus
	CameraForGPU()
		: cameraPosition{ 0.0f, 0.0f, 0.0f }
		, padding(0.0f)
		, inverseViewProjection(QFE::MATH::Matrix4x4::MakeIdentity4x4()) {
	}
#endif
	float32_t3 cameraPosition;
	float32_t padding;
#ifdef __cplusplus
	float32_t4x4 inverseViewProjection;
#else
	row_major float32_t4x4 inverseViewProjection;
#endif
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

struct MaterialGpuData {
#ifdef __cplusplus
	MaterialGpuData()
		: color{ 1.0f, 1.0f, 1.0f, 1.0f }
		, uvTransform(QFE::MATH::Matrix4x4::MakeIdentity4x4())
		, smoothness(0.0f)
		, metallic(0.0f)
		, receiveShadow(1.0f) {
	}
#endif

	float32_t4 color;
	float32_t4x4 uvTransform;
	float32_t smoothness;
	float32_t metallic;
	float32_t receiveShadow;
};

struct GrayscaleParameters {
#ifdef __cplusplus
	GrayscaleParameters()
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

struct ParticleSimulationData {
	float32_t3 position;
	float32_t age;
	float32_t3 velocity;
	float32_t lifetime;
	float32_t startSize;
	float32_t endSize;
	float32_t rotation;
	float32_t angularVelocity;
	float32_t4 startColor;
	float32_t4 endColor;
	uint32_t alive;
	uint32_t padding0;
	uint32_t padding1;
	uint32_t padding2;
};

struct ParticleEmitterForGPU {
	float32_t3 emitterPosition;
	float32_t deltaTime;
	float32_t3 acceleration;
	uint32_t maxParticles;
	float32_t3 velocityMin;
	uint32_t spawnStart;
	float32_t3 velocityMax;
	uint32_t spawnCount;
	float32_t4 startColor;
	float32_t4 endColor;
	float32_t lifetimeMin;
	float32_t lifetimeMax;
	float32_t startSize;
	float32_t endSize;
	float32_t emitterRadius;
	float32_t time;
	float32_t angularVelocityMin;
	float32_t angularVelocityMax;
	uint32_t randomSeed;
	uint32_t reset;
	uint32_t particleOffset;
	uint32_t padding1;
};

struct ParticleViewForGPU {
	float32_t4x4 viewProjection;
	float32_t3 cameraPosition;
	float32_t padding;
};

#ifdef __cplusplus
static_assert(sizeof(ParticleSimulationData) == 96);
static_assert(sizeof(ParticleEmitterForGPU) == 144);
static_assert(sizeof(ParticleViewForGPU) == 80);
#endif

struct ParticleViewMatrices {
#ifdef __cplusplus
	ParticleViewMatrices()
		: viewProjection(QFE::MATH::Matrix4x4::MakeIdentity4x4())
		, billboardMatrix(QFE::MATH::Matrix4x4::MakeIdentity4x4()) {
	}
#endif
	float32_t4x4 viewProjection;
	float32_t4x4 billboardMatrix;
};

struct ParticleForGPU {
#ifdef __cplusplus
	ParticleForGPU()
		: WVP(QFE::MATH::Matrix4x4::MakeIdentity4x4())
		, World(QFE::MATH::Matrix4x4::MakeIdentity4x4())
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
		, color{ 1.0f, 1.0f, 1.0f, 1.0f }
		, texcoord{ 0.0f, 0.0f } {
	}
#endif

	float32_t4 position;
	float32_t4 color;
	float32_t2 texcoord;	
};

struct ObjectTransformGpuData {
#ifdef __cplusplus
	ObjectTransformGpuData()
		: WVP(QFE::MATH::Matrix4x4::MakeIdentity4x4())
		, World(QFE::MATH::Matrix4x4::MakeIdentity4x4()) {
	}
#endif

	float32_t4x4 WVP;
	float32_t4x4 World;
};

struct MeshVertexData {
#ifdef __cplusplus
	MeshVertexData()
		: position{ 0.0f, 0.0f, 0.0f, 1.0f }
		, texcoord{ 0.0f, 0.0f }
		, normal{ 0.0f, 0.0f, 1.0f }
		, padding{0.0f,0.0f,0.0f} {
	}
#endif

#ifdef __cplusplus
	float32_t4 position;
	float32_t2 texcoord;
	float32_t3 normal;
	float32_t padding[3];
#else
	float32_t4 position : POSITION0;
	float32_t2 texcoord : TEXCOORD0;
	float32_t3 normal : NORMAL0;
#endif
};

#ifdef __cplusplus
static_assert(sizeof(MeshVertexData) == 48);
static_assert(offsetof(MeshVertexData, texcoord) == 16);
static_assert(offsetof(MeshVertexData, normal) == 24);
#endif

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

struct MsdfFontParameters {
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
