#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<ParticleEmitterForGPU> gEmitter : register(b0);
RWStructuredBuffer<Particle> gParticles : register(u0);

uint Hash(uint value)
{
    value ^= value >> 16;
    value *= 0x7feb352du;
    value ^= value >> 15;
    value *= 0x846ca68bu;
    value ^= value >> 16;
    return value;
}

float Random01(inout uint state)
{
    state = Hash(state);
    return (float) state / 4294967295.0f;
}

float3 RandomInSphere(inout uint state)
{
    float3 direction = float3(
        Random01(state) * 2.0f - 1.0f,
        Random01(state) * 2.0f - 1.0f,
        Random01(state) * 2.0f - 1.0f);
    float lengthSquared = dot(direction, direction);
    if (lengthSquared < 0.000001f)
    {
        direction = float3(0.0f, 1.0f, 0.0f);
    }
    else
    {
        direction *= rsqrt(lengthSquared);
    }
    return direction * pow(Random01(state), 1.0f / 3.0f);
}

bool IsSpawnIndex(uint index)
{
    if (gEmitter.spawnCount == 0 || gEmitter.maxParticles == 0)
    {
        return false;
    }
    uint relativeIndex = (index + gEmitter.maxParticles - gEmitter.spawnStart) % gEmitter.maxParticles;
    return relativeIndex < gEmitter.spawnCount;
}

[numthreads(64, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint index = dispatchThreadId.x;
    if (index >= gEmitter.maxParticles)
    {
        return;
    }

	uint bufferIndex = gEmitter.particleOffset + index;
	Particle particle = gParticles[bufferIndex];
    if (gEmitter.reset != 0)
    {
        particle.alive = 0;
    }

    if (IsSpawnIndex(index))
    {
        uint state = Hash(gEmitter.randomSeed ^ index ^ asuint(gEmitter.time * 1000.0f));
        particle.position = gEmitter.emitterPosition + RandomInSphere(state) * gEmitter.emitterRadius;
        particle.age = 0.0f;
        particle.velocity = lerp(gEmitter.velocityMin, gEmitter.velocityMax, float3(
            Random01(state), Random01(state), Random01(state)));
        particle.lifetime = lerp(gEmitter.lifetimeMin, gEmitter.lifetimeMax, Random01(state));
        particle.startSize = gEmitter.startSize;
        particle.endSize = gEmitter.endSize;
        particle.rotation = Random01(state) * 6.28318530718f;
        particle.angularVelocity = lerp(
            gEmitter.angularVelocityMin, gEmitter.angularVelocityMax, Random01(state));
        particle.startColor = gEmitter.startColor;
        particle.endColor = gEmitter.endColor;
        particle.alive = 1;
        particle.padding0 = 0;
        particle.padding1 = 0;
        particle.padding2 = 0;
    }
    else if (particle.alive != 0)
    {
        particle.age += gEmitter.deltaTime;
        if (particle.age >= particle.lifetime)
        {
            particle.alive = 0;
        }
        else
        {
            particle.position += particle.velocity * gEmitter.deltaTime
                + gEmitter.acceleration * (0.5f * gEmitter.deltaTime * gEmitter.deltaTime);
            particle.velocity += gEmitter.acceleration * gEmitter.deltaTime;
            particle.rotation += particle.angularVelocity * gEmitter.deltaTime;
        }
    }

	gParticles[bufferIndex] = particle;
}
