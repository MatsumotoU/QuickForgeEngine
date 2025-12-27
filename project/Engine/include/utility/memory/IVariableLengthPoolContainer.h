#pragma once
#include <cstddef>

// * コンテナ状態確認 * //
class IVariableLengthPoolContainer {
public:
	virtual ~IVariableLengthPoolContainer() = default;
	// プール内のオブジェクト数を取得
	virtual size_t Size() const = 0;
	// 使用中のオブジェクト数を取得
	virtual size_t UsedSize() const = 0;
	// プールが空かどうかを確認
	virtual bool IsEmpty() const = 0;
	// プールが満杯かどうかを確認
	virtual bool IsFull() const = 0;
	// 空きインデックス数を取得
	virtual size_t FreeSize() const = 0;
	// 型名を取得
	virtual const char* GetTypeName() const = 0;
};