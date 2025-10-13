#pragma once
#include <string>
#include <unordered_map>

class ResourceDirectoryManager final {
public:
	ResourceDirectoryManager();
	~ResourceDirectoryManager() = default;

	std::string GetResourceDirectory(const std::string& resourceType) const;

private:
	std::unordered_map<std::string, std::string> resourceDirectories_;
};