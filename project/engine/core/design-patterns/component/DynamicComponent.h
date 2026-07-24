#pragma once

#include "IComponentStorage.h"

#include <cstdint>
#include <string>
#include <unordered_map>

namespace QFE {
	class IEntityManager;

	/// DLL境界を越えて登録されるコンポーネントの操作テーブル。
	struct DynamicComponentDescriptor {
		uint32_t apiVersion = 1;
		const char* stableName = nullptr;
		const char* displayName = nullptr;
		void* (*create)() = nullptr;
		void (*destroy)(void*) = nullptr;
		void (*reflect)(void*, Archive&) = nullptr;
		void (*update)(uint32_t, float, IEntityManager*, void*) = nullptr;
	};

	/// DLLが公開するコンポーネント目録。
	struct PluginComponentManifest {
		uint32_t apiVersion = 1;
		const DynamicComponentDescriptor* components = nullptr;
		size_t componentCount = 0;
	};

	using GetPluginComponentManifestFunc = bool(*)(PluginComponentManifest*);

	/// 実体の型を知らずにDLLコンポーネントを所有するストレージ。
	class DynamicComponentStorage final : public IComponentStorage {
	public:
		explicit DynamicComponentStorage(const DynamicComponentDescriptor& descriptor);
		~DynamicComponentStorage() override;

		DynamicComponentStorage(const DynamicComponentStorage&) = delete;
		DynamicComponentStorage& operator=(const DynamicComponentStorage&) = delete;

		void AddDefaultComponent(uint32_t entityId) override;
		void RemoveComponent(uint32_t id) override;
		bool HasComponent(uint32_t id) const override;
		void Clear() override;
		void ReflectComponent(uint32_t id, Archive& ar) override;
		std::string GetStorageTypeName() const override;
		void* GetRawPtr(uint32_t id) override;

		void Update(uint32_t id, float deltaTime, IEntityManager* entityManager);
		const std::string& GetStableName() const;
		const std::string& GetDisplayName() const;

	private:
		DynamicComponentDescriptor descriptor_;
		std::string stableName_;
		std::string displayName_;
		std::unordered_map<uint32_t, void*> components_;
	};
}
