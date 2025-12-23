#pragma once
#include "ISceneEntityCommand.h"
class CameraManager;

class WvpTransformationCommand final : public ISceneEntityCommand {
public:
	WvpTransformationCommand() = delete;
	explicit WvpTransformationCommand(EntityManager& entityManager,CameraManager& cameraMana);
	~WvpTransformationCommand() override = default;

	/// 全オブジェクトのワールド・ビュー・プロジェクション行列を更新する
	void Execute() override;
	/// ワールド・ビュー・プロジェクション行列の更新は元に戻せない
	void Undo() override;

	std::string GetCommandName() const override;

private:
	CameraManager& cameraManager_;
};