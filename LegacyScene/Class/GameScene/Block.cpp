#include "Block.h"
#include "Utility/MyEasing.h"
#include <cassert>

void Block::Initialize(EngineCore* engineCore, Camera* camera, BlockType type) {
	engineCore_ = engineCore;
	camera_ = camera;

	/*dirtModel_ = std::make_unique<Model>(engineCore_, camera_);
	grassModel_ = std::make_unique<Model>(engineCore_, camera_);
	StoneModel_ = std::make_unique<Model>(engineCore_, camera_);

	dirtModel_->LoadModel("Resources/Model/blocks/dirt", "dirt.obj", COORDINATESYSTEM_HAND_LEFT);
	grassModel_->LoadModel("Resources/Model/blocks/grass", "grass.obj", COORDINATESYSTEM_HAND_LEFT);
	StoneModel_->LoadModel("Resources/Model/blocks/stone", "stone.obj", COORDINATESYSTEM_HAND_LEFT);*/

	isDraw_ = true;
	type_ = type;

	color_ = { 1.0f,1.0f,1.0f,1.0f };
}

void Block::Update() {
	MyEasing::SimpleEaseIn(&transform_.scale.y, 1.0f, 0.1f);
	/*MyEasing::SimpleEaseIn(&dirtModel_->transform_.scale.y, 1.0f, 0.1f);
	MyEasing::SimpleEaseIn(&grassModel_->transform_.scale.y, 1.0f, 0.1f);
	MyEasing::SimpleEaseIn(&StoneModel_->transform_.scale.y, 1.0f, 0.1f);

	dirtModel_->Update();
	grassModel_->Update();
	StoneModel_->Update();*/
}

//void Block::Draw() {
//	if (isDraw_) {
//		switch (type_)
//		{
//		case BlockType::Dirt:
//			dirtModel_->Draw();
//			break;
//		case BlockType::Grass:
//			grassModel_->Draw();
//			break;
//		case BlockType::Stone:
//			StoneModel_->Draw();
//			break;
//		default:
//			assert(false && "Invalid BlockType");
//			break;
//		}
//	}
//}

void Block::BuildUpSpawn() {
	if (isDraw_) {
		return;
	}

	/*dirtModel_->transform_.scale.y = 0.0f;
	grassModel_->transform_.scale.y = 0.0f;
	StoneModel_->transform_.scale.y = 0.0f;*/
	isDraw_ = true;
}

//Transform& Block::GetTransform() {
//	switch (type_)
//	{
//	case BlockType::Dirt:
//		return dirtModel_->transform_;
//		break;
//	case BlockType::Grass:
//		return grassModel_->transform_;
//		break;
//	case BlockType::Stone:
//		return StoneModel_->transform_;
//		break;
//	default:
//		assert(false && "Invalid BlockType");
//		return dirtModel_->transform_;
//		break;
//	}
//}

const BlockType& Block::GetType() const {
	return type_;
}

void Block::SetIsDraw(bool isDraw) {
	isDraw_ = isDraw;
}

void Block::SetColor(const Vector4& color) {
	color_ = color;

	/*switch (type_)
	{
	case BlockType::Dirt:
		return dirtModel_->SetColor(color);
		break;
	case BlockType::Grass:
		return grassModel_->SetColor(color);
		break;
	case BlockType::Stone:
		return StoneModel_->SetColor(color);
		break;
	default:
		assert(false && "Invalid BlockType");
		return dirtModel_->SetColor(color);
		break;
	}*/
}

void Block::SetType(BlockType type) {
	type_ = type;
}

void Block::SetTransform(const Transform& transform) {
	transform_ = transform;
		/*dirtModel_->transform_ = transform;
		grassModel_->transform_ = transform;
		StoneModel_->transform_ = transform;*/
}
