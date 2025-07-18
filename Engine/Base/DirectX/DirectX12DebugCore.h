#pragma once
#define NOMINMAX

class D3DResourceLeakChecker;

class DirectX12DebugCore final {
public:
	DirectX12DebugCore();
	~DirectX12DebugCore();

private:
	D3DResourceLeakChecker* resourceLeakChecker_;

};