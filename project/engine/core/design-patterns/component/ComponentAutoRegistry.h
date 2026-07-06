#pragma once
#include <unordered_map>
#include <string>
#include <functional>
#include <memory>
#include <vector>

#include "ComponentStorage.h"

namespace QFE {
    class IComponentStorage;

    class ComponentAutoRegistry {
    public:
        using Creator = std::function<std::unique_ptr<IComponentStorage>()>;

        // 登録に必要な情報をまとめる構造体
        struct RegistryEntry {
            size_t typeId;
            std::string name;
            Creator creator;
        };

        static ComponentAutoRegistry& Instance() {
            static ComponentAutoRegistry instance;
            return instance;
        }

        // 修正：hash_codeも一緒に受け取る
        void Register(size_t typeId, const std::string& name, Creator creator) {
            entries_.push_back({ typeId, name, creator });
        }

        // 追加：EntityManagerが初期化時にすべてのエントリーを参照するための関数
        const std::vector<RegistryEntry>& GetEntries() const {
            return entries_;
        }

    private:
        std::vector<RegistryEntry> entries_; // マップではなく、全登録を舐めるためのリストにする
    };

    template <typename T>
    struct ComponentRegisterer {
        ComponentRegisterer(const std::string& name) {
            // 型の hash_code も一緒に裏側で自動登録する
            ComponentAutoRegistry::Instance().Register(
                typeid(T).hash_code(),
                name,
                []() { return std::make_unique<ComponentStorage<T>>(); }
            );
        }
    };
}