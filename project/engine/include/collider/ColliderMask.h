#pragma once
#include <string>
#include <vector>
#include <utility>

class ColliderTagMask final {
public:
	ColliderTagMask();
	~ColliderTagMask();

	void AddTagMaskPair(const std::string& tag1, const std::string& tag2);
	void EraseTagMaskPair(const std::string& tag1, const std::string& tag2);
	bool IsCollidable(const std::string& tag1, const std::string& tag2) const;

	const std::vector<std::pair<std::string, std::string>>& GetTagMaskPairs() const;
private:
	std::vector<std::pair<std::string, std::string>> tagMaskPairs_;
};