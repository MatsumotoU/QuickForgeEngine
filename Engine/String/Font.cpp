#include "Font.h"
#include "Base/EngineCore.h"

void Font::Initialize(const std::string& fontFilePath, float fontSize) {
	fontSize;
	fontAtlasData_ = FontLoader::LoadFontData(fontFilePath);
	
	fontDataBuffer_.CreateResource(engineCore_->GetDirectXCommon()->GetDevice());
	fontDataBuffer_.GetData()->AtlasSize = Vector2(fontAtlasData_.width, fontAtlasData_.height);
	fontDataBuffer_.GetData()->DistanceRange = fontAtlasData_.distanceRange;

	wvpResource_.Initialize(engineCore_->GetDirectXCommon(),1);
}

//void Font::Draw(const char& text, const Vector2& position) {
//}
