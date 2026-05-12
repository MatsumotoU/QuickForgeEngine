#include "engine/include/assets/Script/CsharpCompiler.h"
#include <fstream>
#include <sstream>
#include <iostream>

#include "engine/include/core/EngineDefines.h"

using namespace QFE;

void QFE::GenerateCsproj(const std::string& dir, const std::string& outputPath) {
    std::vector<std::string> csFiles;
	QFE_LOG("Generating .csproj file at: " + outputPath);
	QFE_LOG("Looking for .cs files in: " + dir);

    // ディレクトリが存在するか、かつディレクトリであるかをチェック
    if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.path().extension() == ".cs") {
                csFiles.push_back(entry.path().filename().string());
				QFE_LOG("Found C# script: " + entry.path().filename().string());
            }
        }
    } else {
		QFE_LOG("Scripts directory does not exist: " + dir, LogLevel::Error);
        return; // ディレクトリが存在しない場合は処理を中断
    }

    std::ofstream ofs(outputPath);
    ofs << "<Project Sdk=\"Microsoft.NET.Sdk\">\n";
    ofs << "  <PropertyGroup>\n";
    ofs << "    <TargetFramework>netstandard2.0</TargetFramework>\n";
    ofs << "    <OutputType>Library</OutputType>\n";
    ofs << "    <RootNamespace>MyGameScripts</RootNamespace>\n";
    ofs << "    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>\n";
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
	// ログファイルのパスを作成
	std::string logPath = std::filesystem::path(csprojPath).parent_path().string() + "/build_log.txt";
	QFE_LOG("Compiling C# project. Log will be saved to: " + logPath);
	
	// コマンド実行（出力をログファイルにリダイレクト）
	std::string command = "dotnet build \"" + csprojPath + "\" -c Release -o \"" + std::filesystem::path(outputDllPath).parent_path().string() + "\" > \"" + logPath + "\" 2>&1";
	QFE_LOG("Executing command: " + command);

	int result = system(command.c_str());
	QFE_LOG("Command executed with result code: " + std::to_string(result));

	// 失敗時（または常に）ログを表示
	if (result != 0) {
		std::ifstream logFile(logPath);
		std::stringstream buffer;
		buffer << logFile.rdbuf();
		std::string logContent = buffer.str();
		
#ifdef QFE_OPTIMIZE_OFF
		QFE_LOG("C# Build Failed. Log output:", LogLevel::Error);
		QFE_LOG(logContent, LogLevel::Error);
#else
		std::cerr << "C# Build Failed:\n" << logContent << std::endl;
#endif

		QFE_REPORT_USER_ERROR("C# Build Failed. Check log for details.", UserError::DeveloperError);
	}
}
