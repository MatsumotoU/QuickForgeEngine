#include "engine/include/scene/SceneCommand/BillboardUpdateCommand.h"
#include "engine/include/core/Entity/EntityManager.h"
#include "engine/include/camera/Data/BillboardComponent.h"
#include "engine/include/core/Math/Transform.h"

using namespace QFE::Component;

void QFE::BillboardUpdateCommand::Execute() {
	// ビルボードコンポーネントとトランスフォームコンポーネントのストレージが存在しない場合は処理を抜ける
	if (!entityManager_.HasComponentStrage<BillboardComponent>() ||
		!entityManager_.HasComponentStrage<Transform>()) {
		return;
	}

	entityManager_.GetComponentStrage<BillboardComponent>().Each([&](uint32_t entityId, BillboardComponent& billboardComp) {
		// トランスフォームコンポーネントが存在しない場合はスキップ
		if (!entityManager_.HasComponent<Transform>(entityId)) {
			return;
		}
		// トランスフォームコンポーネントを取得
		Transform& transform = entityManager_.GetComponent<Transform>(entityId);
		// ビルボードタイプに応じた処理を実行
		if (billboardComp.type_ == BillboardType::AXIAL) {
			CameraAxisBillboard(transform, billboardComp.rotateOffset_); // カメラの向きに同期
		} else if (billboardComp.type_ == BillboardType::POINT) {
			CameraPointBillboard(transform, billboardComp.rotateOffset_); // カメラの位置を向く
		}
		});
}

void QFE::BillboardUpdateCommand::Undo() {
	/// システム側で自動更新されるため、特に処理は不要
}

void QFE::BillboardUpdateCommand::CameraAxisBillboard(Transform& targetTransform, const Vector3& rotateOffset) {
	// カメラと同じ回転にする
	targetTransform.rotate = cameraTransform_.rotate + rotateOffset;
}

void QFE::BillboardUpdateCommand::CameraPointBillboard(Transform& targetTransform, const Vector3& rotateOffset) {
	// カメラの位置とターゲットの位置から向きを計算
	Vector3 direction = (cameraTransform_.translate - targetTransform.translate).Normalize();
	// Y軸を基準に向きを計算
	float yaw = atan2f(direction.x, direction.z);
	float pitch = asinf(-direction.y);
	// ラジアンを度に変換して回転に設定
	targetTransform.rotate = Vector3{ pitch , yaw , 0.0f } + rotateOffset;
}
