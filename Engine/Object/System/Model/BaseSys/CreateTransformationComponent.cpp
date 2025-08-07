#include "CreateTransformationComponent.h"
#include "Base/EngineCore.h"

void CreateTransformationComponent::Create(EngineCore* engineCore, TransformComponent& transformComponent) {
	if (!engineCore) {
		return; // エンジンコアが無効な場合は何もしない
	}

	// トランスフォームコンポーネントのリソースを作成
	transformComponent.transformationBuffer_.CreateResource(engineCore->GetDirectXCommon()->GetDevice());

	// シェーダーに送るデータを更新
	transformComponent.transformationBuffer_.GetData()->World = Matrix4x4::MakeIndentity4x4();
	transformComponent.transformationBuffer_.GetData()->WVP = Matrix4x4::MakeIndentity4x4();
}
