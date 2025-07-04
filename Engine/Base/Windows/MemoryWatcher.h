#pragma once
#include <Windows.h>
#include <Psapi.h>

class MemoryWatcher {
public:
	MemoryWatcher();

public:
	void DrawImGui();

public:
	size_t GetProcessMemoryUsageMB();
	
};