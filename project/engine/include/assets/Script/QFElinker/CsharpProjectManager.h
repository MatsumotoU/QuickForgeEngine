#pragma once
#include <string>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>

class CsharpProjectManager final {
public:
	void Initialize();
	void GenerateProject(const std::string& projectName);
	void Finalize();
private:
	
};