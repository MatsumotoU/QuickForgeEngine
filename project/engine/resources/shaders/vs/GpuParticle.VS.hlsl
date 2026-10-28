#include "../ShaderStructs/GpuParticle.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

StructuredBuffer<ParticleSimulationData> gParticle : register(t0);
ConstantBuffer<ParticleViewForGPU> gParticleView : register(b0);

VertexShaderOutput main(MeshVertexData input, uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    ParticleSimulationData particle = gParticle[instanceId];
    if (particle.alive == 0)
    {
        output.position = float4(2.0f, 2.0f, 2.0f, 1.0f);
        output.texcoord = input.texcoord;
        output.color = 0.0f;
        return output;
    }

    float normalizedAge = saturate(particle.age / max(particle.lifetime, 0.0001f));
    float size = lerp(particle.startSize, particle.endSize, normalizedAge);
    float sine;
    float cosine;
    sincos(particle.rotation, sine, cosine);
    float2 localPosition = input.position.xy * size;
    localPosition = float2(
        localPosition.x * cosine - localPosition.y * sine,
        localPosition.x * sine + localPosition.y * cosine);

    float3 forward = gParticleView.cameraPosition - particle.position;
    float forwardLengthSquared = dot(forward, forward);
    forward = forwardLengthSquared > 0.000001f
        ? forward * rsqrt(forwardLengthSquared)
        : float3(0.0f, 0.0f, -1.0f);
    float3 right = cross(float3(0.0f, 1.0f, 0.0f), forward);
    float rightLengthSquared = dot(right, right);
    right = rightLengthSquared > 0.000001f
        ? right * rsqrt(rightLengthSquared)
        : float3(1.0f, 0.0f, 0.0f);
    float3 up = normalize(cross(forward, right));
    float3 worldPosition = particle.position + right * localPosition.x + up * localPosition.y;

    output.position = mul(float4(worldPosition, 1.0f), gParticleView.viewProjection);
    output.texcoord = input.texcoord;
    output.color = lerp(particle.startColor, particle.endColor, normalizedAge);
    return output;
}
