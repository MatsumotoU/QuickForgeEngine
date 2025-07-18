#include "FontDrawer.h"
//#include <msdfgen.h>
//#include <msdfgen-ext.h>

#include <stdexcept>

FontDrawer::FontDrawer() {
}

//Microsoft::WRL::ComPtr<ID3D12Resource> FontDrawer::LoadFont(const std::string& path) {
//
//	msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
//	if (!ft) {
//		throw std::runtime_error("Failed to initialize FreeType");
//	}
//	msdfgen::FontHandle* font = msdfgen::loadFont(ft, path.c_str());
//	if (!font) {
//		msdfgen::deinitializeFreetype(ft);
//		throw std::runtime_error("Failed to load font: " + path);
//	}
//	// フォントの処理をここに追加
//	Microsoft::WRL::ComPtr<ID3D12Resource> result{};
//	// ここでフォントデータを処理して、DirectXのリソースに変換する必要があります。
//	// 例えば、msdfgenを使ってフォントのグリフを生成し、テクスチャとしてDirectXにアップロードするなど。
//	// ここでは、フォントのグリフを生成する例を示します。
//	msdfgen::Shape shape;
//	msdfgen::loadGlyph(shape, font, 65); // 'A'のグリフを読み込む例
//	msdfgen::Bitmap<float> msdfBitmap;
//	msdfgen::generateMSDF(msdfBitmap, shape, 48.0f, 48.0f, 1.0f); // MSDFを生成
//
//	msdfgen::destroyFont(font);
//	msdfgen::deinitializeFreetype(ft);
//
//	return result;
//}