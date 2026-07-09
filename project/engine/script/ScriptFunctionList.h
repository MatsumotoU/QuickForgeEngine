#pragma once
#include <string>
#include <vector>
#include "design-patterns/EntityManager.h"

namespace QFE::SCRIPT {
	/// @brief スクリプト関数の型定義
	using RawScriptFuncPtr = void(*)(uint32_t entityId, float dt, QFE::IEntityManager* entityManager);	
	/// @brief スクリプト関数の情報を保持する構造体
	struct ScriptFunctionInfo {
		const char* functionName; ///< スクリプト関数の名前
		const char* displayName; ///< スクリプト関数の表示名
		RawScriptFuncPtr functionPtr; ///< スクリプト関数のポインタ
	};

	/// @brief エンティティIDとコンポーネント型からコンポーネントを取得するための便利関数
    template <typename T>
    inline T& GetComponent(uint32_t entityId, QFE::IEntityManager* entityManager) {

        // 1. 型 T から、安全な「文字列の名前」を自動で取得
        const char* compName = T::GetTypeName().c_str();

        // 2. EXE側の全自動化された窓口を呼び出す
        void* rawPtr = entityManager->GetComponentRaw(entityId, compName);

        assert(rawPtr != nullptr && "Component not found or not attached!");

        // 3. 安全に型をキャストして戻す
        return *reinterpret_cast<T*>(rawPtr);
    }
}

extern "C" {
	/// @brief スクリプト関数の目録を取得する関数の型定義
	typedef size_t(*GetManifestFunc)(QFE::SCRIPT::ScriptFunctionInfo** outFunctionList);
}

