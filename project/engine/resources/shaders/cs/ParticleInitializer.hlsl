#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<uint32_t> g_particleCount : register(b0);
ConstantBuffer<Particle> g_initialParticleData : register(b1);
RWStructuredBuffer<Particle> g_particles : register(u0);
RWStructuredBuffer<int32_t> gFreeCounter : register(u1);

[numthreads(8, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID){
    // スレッドIDがパーティクル数を超えている場合は処理しない
    uint index = DTid.x;
    if (index >= g_particleCount)
        return;
    // パーティクルの初期化
    g_particles[index] = g_initialParticleData;
    if (index == 0)
    {
        gFreeCounter[0] = 0;
    }
}