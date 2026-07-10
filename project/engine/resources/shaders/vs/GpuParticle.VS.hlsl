#include "../ShaderStructs/GpuParticle.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

StructuredBuffer<Particle> gParticle : register(t0);
ConstantBuffer<ParView> gParView : register(b0);

struct VertexShaderInput
{
    float32_t4 position : POSITION0;
    float32_t2 texcoord : TEXCOORD0;
    float32_t3 normal : NORMAL0;
};

VertexShaderOutput main(VertexShaderInput input, uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    Particle particle = gParticle[instanceId];
    float32_t4x4 worldMatrix = gParView.billboardMatrix;
    worldMatrix[0] *= particle.scale.x;
    worldMatrix[1] *= particle.scale.y;
    worldMatrix[2] *= particle.scale.z;
    worldMatrix[3].xyz = particle.translate.xyz;
    output.position = mul(input.position, mull(worldMatrix,gParView.viewProjection));
    output.texcoord = input.texcoord;
    output.color = particle.color;
    return output;
}