#include "engine/include/assets/Script/CsharpCompiler.h"

using namespace QFE;

void QFE::GenerateCsproj(const std::string& dir, const std::string& outputPath) {
    std::vector<std::string> csFiles;
    for (const auto& entry : std::filesystem::directory_iterator(dir)) {
        if (entry.path().extension() == ".cs") {
            csFiles.push_back(entry.path().filename().string());
        }
    }

    std::ofstream ofs(outputPath);
    ofs << "<Project Sdk=\"Microsoft.NET.Sdk\">\n";
    ofs << "  <PropertyGroup>\n";
    ofs << "    <TargetFramework>netstandard2.0</TargetFramework>\n";
    ofs << "    <OutputType>Library</OutputType>\n";
    ofs << "    <RootNamespace>MyGameScripts</RootNamespace>\n";
    ofs << "  </PropertyGroup>\n";
    ofs << "  <ItemGroup>\n";
    for (const auto& file : csFiles) {
        ofs << "    <Compile Include=\"" << file << "\" />\n";
    }
    ofs << "  </ItemGroup>\n";
    ofs << "</Project>\n";
    ofs.close();
}

void QFE::CompileCSharpProject(const std::string& csprojPath, const std::string& outputDllPath) {
	std::string command = "dotnet build " + csprojPath + " -c Release -o " + std::filesystem::path(outputDllPath).parent_path().string();
	int result = system(command.c_str());
	if (result != 0) {
		throw std::runtime_error("Failed to compile C# project.");
	}
}
