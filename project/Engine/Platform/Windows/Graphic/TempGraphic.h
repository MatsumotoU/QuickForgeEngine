#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include "Graphic/ShaderBuffer/ConstantBuffer.h"
#include "Graphic/ShaderBuffer/StructuredBuffer.h"
#include "Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

namespace {
	const int kMaxEchoSpheres = 500;
}

class TempGraphic : public Singleton<TempGraphic>{
	friend class Singleton<TempGraphic>;
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	void Echo(Vector3 pos, float thickness, float expandSpeed);

	void EchoFromAudioData(uint32_t audioHandle,Vector3 pos,float power);

	ConstantBuffer<EchoSphereInfo>* GetEchoSphereBuffer() { return &echoSphereBuffer_; }
	StructuredBuffer<EchoSphere>* GetEchoSphereStructuredBuffer() { return &echoSphereStructuredBuffer_; }

private:
	float pow;
	ConstantBuffer<EchoSphereInfo> echoSphereBuffer_;
	StructuredBuffer<EchoSphere> echoSphereStructuredBuffer_;
};