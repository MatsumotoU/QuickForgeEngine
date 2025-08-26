#pragma once
#include "../Data/DescriptorGenerateConfig.h"

class CheckGenerateConfig final {
public:
	// ディスクリプタ生成設定をチェックする関数
	static bool IsValid(const DescriptorGenerateConfig& config);
};