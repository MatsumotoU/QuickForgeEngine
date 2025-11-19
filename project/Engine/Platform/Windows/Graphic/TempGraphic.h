#pragma once
#include "Utility/DesignPatterns/Singleton.h"
#include "Graphic/ShaderBuffer/ConstantBuffer.h"
#include "Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

class TempGraphic : public Singleton<TempGraphic>{
	friend class Singleton<TempGraphic>;
public:
	void Initialize();
	void Update();
	void Draw();
	void Finalize();

	void Echo(Vector3 pos, float power);

	ConstantBuffer<EchoSphere>* GetEchoSphereBuffer() { return &echoSphereBuffer_; }

private:
	float pow;
	ConstantBuffer<EchoSphere> echoSphereBuffer_;
};