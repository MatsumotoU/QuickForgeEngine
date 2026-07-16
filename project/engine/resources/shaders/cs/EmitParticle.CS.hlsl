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
        RandomGenerator randomGenerator;
        randomGenerator.seed = (DTid + gPerFrame.time) * gPerFrame.time;
        
        for(uint32_t countIndex = 0; countIndex < gEmitter.count; ++countIndex)
        {
            int32_t particleIndex;
            InterlockedAdd(gFreeCounter[0], 1, particleIndex);
            if(particleIndex >= gParticles.Length)
            {
                break;
            }
            gParticles[particleIndex].scale = randomGenerator.Generate3d();
            gParticles[particleIndex].position = gEmitter.position + randomGenerator.Generate3d() * gEmitter.radius;
            gParticles[particleIndex].velocity = randomGenerator.Generate3d(); 
            gParticles[particleIndex].lifetime = randomGenerator.Generate1d();
            gParticles[particleIndex].color = randomGenerator.Generate3d();
        }
    }
}