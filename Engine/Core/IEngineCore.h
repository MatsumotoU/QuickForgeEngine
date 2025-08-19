#pragma once
class IEngineCore {
public:
	IEngineCore() = default;
	virtual ~IEngineCore() = default;
	
	virtual void Initialize() = 0;
	virtual void MainLoop() = 0;
	virtual void Shutdown() = 0;
};