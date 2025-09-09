#include "AiLeveler.h"

AiLeveler::AiLeveler() {
	aiLevel_ = 10;
	lifeWeight_ = 1.0f;
	attackWeight_ = 0.0f;
	uniqueWeight_ = 0.0f;
}

void AiLeveler::LoadAIConfig(const std::string& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        // ファイルが開けなかった場合の処理
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::vector<std::string> tokens;
        std::stringstream ss(line);
        std::string token;
        while (std::getline(ss, token, ',')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 4) {
            aiLevel_ = static_cast<uint32_t>(std::stoul(tokens[0]));
            lifeWeight_ = std::stof(tokens[1]);
            attackWeight_ = std::stof(tokens[2]);
            uniqueWeight_ = std::stof(tokens[3]);
        }
    }
}
