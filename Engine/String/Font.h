#pragma once
#include <string>
#include "Math/Vector/Vector2.h"

#include "FontLoader.h"
#include "Base/DirectX/Resource/ShaderBuffers/ConstantBuffer.h"
#include "Base/DirectX/WVPResource.h"

class EngineCore;

class Font final {
public:
	Font() = delete;
	Font(EngineCore* engineCore) : engineCore_(engineCore) {};

public:
	/// <summary>
	/// フォントの初期化
	/// </summary>
	/// <param name="fontFilePath">フォントファイルのパス</param>
	/// <param name="fontSize">フォントサイズ</param>
	void Initialize(const std::string& fontFilePath, float fontSize);
	/// <summary>
	/// フォントの描画
	/// </summary>
	/// <param name="text">描画するテキスト</param>
	/// <param name="position">描画位置</param>
	void Draw(const char& text, const Vector2& position);

private:
	

private:
	EngineCore* engineCore_ = nullptr;

	WVPResource wvpResource_;

	FontAtlasData fontAtlasData_;
	ConstantBuffer<FontDataToShader> fontDataBuffer_;
};