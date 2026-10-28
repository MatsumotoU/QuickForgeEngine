#include "../ShaderStructs/Particle.hlsli"
#include "../ShaderStructs/hlslTypeToCpp.h"

StructuredBuffer<ParticleForGPU> gParticle : register(t0);

VertexShaderOutput main(MeshVertexData input,uint32_t instanceId : SV_InstanceID)
{
    VertexShaderOutput output;
    output.position = mul(input.position, gParticle[instanceId].WVP);
    output.texcoord = input.texcoord;
    output.color = gParticle[instanceId].color;
    
    return output;
}