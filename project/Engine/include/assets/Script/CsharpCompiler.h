#pragma once
#include <fstream>
#include <filesystem>
#include <vector>
#include <string>

namespace QFE {

	void GenerateCsproj(const std::string& dir, const std::string& outputPath);

	void CompileCSharpProject(const std::string& csprojPath, const std::string& outputDllPath);

}
