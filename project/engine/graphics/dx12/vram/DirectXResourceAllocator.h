#pragma once
#include <vector>
#include <unordered_map>
#include <string>
#include <typeindex>
#include <utility>
#include <d3d12.h>
#include "graphics/dx12/GraphicEngineHandleTypes.h"
#include "resources/DirectXResourceContainer.h"
#include "string/MyString.h"

namespace QFE::GRAPHIC {

    class DirectXResourceAllocator {
    private:
        struct SizeBucket {
            std::vector<DirectXResourceHandle> resources;
            size_t currentOffset = 0;
        };

        // std::pair<std::type_index, size_t> をunordered_mapのキーにするためのカスタムハッシュ
        struct TypeNumHash {
            std::size_t operator()(const std::pair<std::type_index, size_t>& p) const {
                auto h1 = p.first.hash_code();
                auto h2 = std::hash<size_t>{}(p.second);
                return h1 ^ (h2 << 1); // ハッシュを結合
            }
        };

    public:
        void Initialize(DirectXResourceContainer* container, ID3D12Device* device);

        /// @brief 指定した型のCBVを割り当てる関数。
        template<typename T>
        DirectXResourceHandle AllocateConstantBuffer(const std::string& name = "ConstantBuffer") {
            size_t size = sizeof(T);

            auto& bucket = cbvPools_[size];

            if (bucket.currentOffset >= bucket.resources.size()) {
                Grow(bucket, size, size, false, 0, name);
            }

            return bucket.resources[bucket.currentOffset++];
        }

        /// @brief 指定した型のSRV（構造化バッファ）を割り当てる関数。
        template<typename T>
        DirectXResourceHandle AllocateStructuredBuffer(size_t num, const std::string& name = "StructuredBuffer") {
            size_t stride = sizeof(T);
            size_t totalSize = stride * num;

            // ★「型情報」と「要素数」のペアをキーにする
            auto key = std::make_pair(std::type_index(typeid(T)), num);
            auto& bucket = srvPools_[key];

            if (bucket.currentOffset >= bucket.resources.size()) {
                // SRVフラグをtrueにし、要素数を渡してGrow
                Grow(bucket, totalSize, stride, true, num, name);
            }

            return bucket.resources[bucket.currentOffset++];
        }

		/// @brief 生のバイトサイズでCBVを割り当てる関数。
		DirectXResourceHandle AllocateRawConstantBuffer(size_t byteSize, const std::string& name = "RawBuffer");
		/// @brief 生のバイトサイズでSRV（構造化バッファ）を割り当てる関数。
		DirectXResourceHandle AllocateRawStructuredBuffer(size_t byteSize, size_t stride, const std::string& name = "RawStructuredBuffer");

        void ResetFrame();

    private:
        void Grow(SizeBucket& bucket, size_t totalSize, size_t stride, bool isSRV, size_t elementCount, const std::string& name);

		bool isInitialized_ = false;

        DirectXResourceContainer* container_ = nullptr;
        ID3D12Device* device_ = nullptr;

        // CBVは「256Bアライメント済みの全体サイズ」で小分け
        std::unordered_map<size_t, SizeBucket> cbvPools_;

        // SRVは「型(T) と 要素数(num)」のコンボで完全に分離管理
        std::unordered_map<std::pair<std::type_index, size_t>, SizeBucket, TypeNumHash> srvPools_;
    };
}