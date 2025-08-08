#include "EntityLoader.h"
#include "Object/Entity/EntityManager.h"
#include "Base/EngineCore.h"
#include "Utility/SimpleJson.h"

#include "EntitySerializer.h"

void EntityLoader::LoadEntities(EngineCore* engineCore, EntityManager& entityManager, const std::string& fileName) {
	std::string filePath = engineCore->GetDirectoryManager().GetDirectoryPath(DirectoryManager::AssetDirectory) + fileName;
	std::ifstream inFile(filePath);
	if (!inFile.is_open()) {
		throw std::runtime_error("Failed to open file for reading: " + filePath);
	}
	nlohmann::json jsonData;
	try {
		inFile >> jsonData;
	}
	catch (const nlohmann::json::parse_error& e) {
		throw std::runtime_error("JSON parse error: " + std::string(e.what()));
	}
	inFile.close();
	if (jsonData.is_null()) {
		throw std::runtime_error("Failed to deserialize JSON data from file: " + fileName);
	}
	EntitySerializer::Deserialize(engineCore, entityManager, jsonData);
}

void EntityLoader::SaveEntities(EngineCore* engineCore, EntityManager& entityManager, const std::string& fileName) {
	nlohmann::json jsonData = EntitySerializer::Serialize(entityManager);
	std::string jsonString = jsonData.dump(4); // 4はインデントのスペース数
	std::string filePath = engineCore->GetDirectoryManager().GetDirectoryPath(DirectoryManager::SceneDirectory) + fileName;

	// fileNameに.jsonが含まれていない場合は追加する
	if (filePath.find(".json") == std::string::npos) {
		filePath = filePath + ".json";
	}
	
	std::ofstream outFile(filePath);
	if (!outFile.is_open()) {
		throw std::runtime_error("Failed to open file for writing: " + filePath);
	}
	outFile << jsonString;
	outFile.close();
	if (!outFile) {
		throw std::runtime_error("Failed to write JSON data to file: " + filePath);
	}
}
