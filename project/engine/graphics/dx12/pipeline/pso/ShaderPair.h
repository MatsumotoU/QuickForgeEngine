#pragma once
#include <string>

namespace QFE::GRAPHIC::INTERNAL {
	class ShaderPair final {
	public:
		ShaderPair() = default;
		ShaderPair(const std::wstring& vsPath, const std::wstring& psPath) : vsPath_(vsPath), psPath_(psPath) {}
		~ShaderPair() = default;
	public:
		std::wstring vsPath_;
		std::wstring psPath_;
	};
}