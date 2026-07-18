#include "../ShaderStructs/hlslTypeToCpp.h"

ConstantBuffer<uint32_t> g_particleCount : register(b0);
ConstantBuffer<PerFrame> gPerFrame : register(b1);
RWStructuredBuffer<Particle> g_particles : register(u0);

[numthreads(8, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    // スレッドIDがパーティクル数を超えている場合は処理しない
    uint index = DTid.x;
    if (index >= g_particleCount)
    {
        return;
    }
    // パーティクルのアルファ値が0でない場合のみ更新処理を行う
    if(g_particles[index].color.a != 0.0f)
    {
        g_particles[index].translate += g_particles[index].velocity;
        g_particles[index].currentTime += gPerFrame.deltaTime;
        float32_t alpha = 1.0f - (g_particles[index].currentTime / g_particles[index].lifeTime);
        g_particles[index].color.a = saturate(alpha);
    }
}