#include "../ShaderStructs/hlslTypeToCpp.h"
#include "../utility/RandomGenerator.hlsli"

ConstantBuffer<EmitterSphere> gEmitter : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);
RWStructuredBuffer<Particle> gParticles : register(u0);
RWStructuredBuffer<int32_t> gFreeCounter : register(u1);

[numthreads(1,1,1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    if (gEmitter.emit != 0)
    {
        uint32_t particleCapacity;
        uint32_t particleStride;
        gParticles.GetDimensions(particleCapacity, particleStride);

        RandomGenerator randomGenerator;
        randomGenerator.seed = (dispatchThreadID + gPerFrame.time) * gPerFrame.time;
        
        for(uint32_t countIndex = 0; countIndex < gEmitter.count; ++countIndex)
        {
            int32_t particleIndex;
            InterlockedAdd(gFreeCounter[0], 1, particleIndex);
            if(particleIndex >= particleCapacity)
            {
                break;
            }
            gParticles[particleIndex].scale = randomGenerator.Generate3d();
            gParticles[particleIndex].translate = gEmitter.translate + randomGenerator.Generate3d() * gEmitter.radius;
            gParticles[particleIndex].velocity = randomGenerator.Generate3d(); 
            gParticles[particleIndex].lifeTime = max(randomGenerator.Generate1d(), 0.001f);
            gParticles[particleIndex].currentTime = 0.0f;
            gParticles[particleIndex].color = float32_t4(randomGenerator.Generate3d(), 1.0f);
        }
    }
}
