#pragma once
#include "ISceneEntityCommand.h"
#include "engine/include/core/Math/Transform.h"
#include <nlohmann/json.hpp>

namespace QFE {
	/// @brief ビルボード更新コマンド
	class BillboardUpdateCommand : public ISceneEntityCommand {
	public:
		BillboardUpdateCommand() = delete;
		explicit BillboardUpdateCommand(EntityManager& em, const Transform& cameraTransform)
			: ISceneEntityCommand(em), cameraTransform_(cameraTransform) {
		}
		virtual ~BillboardUpdateCommand() = default;
		// コピー・ムーブ禁止
		BillboardUpdateCommand(const BillboardUpdateCommand&) = delete;
		BillboardUpdateCommand& operator=(const BillboardUpdateCommand&) = delete;
		BillboardUpdateCommand(BillboardUpdateCommand&&) = delete;
		BillboardUpdateCommand& operator=(BillboardUpdateCommand&&) = delete;

		// コマンド実行
		virtual void Execute() override;
		virtual void Undo() override;
		virtual std::string GetCommandName() const override { return "BillboardUpdateCommand"; }
	private:
		const Transform& cameraTransform_;
		// カメラ軸ビルボード処理
		void CameraAxisBillboard(Transform& targetTransform,const Vector3& rotateOffset);
		// カメラポイントビルボード処理
		void CameraPointBillboard(Transform& targetTransform, const Vector3& rotateOffset);
	};
}