#pragma once
#include <vector>
#include <string>
#include <externals/imgui/ImGuizmo-1.83/ImSequencer.h>

namespace QFE {
	template<typename T>
	struct SequencerItem {
		int start;
		int end;
		int type;
		T data;
	};

	template<typename T>
	class ImSequencerWrapper : public ImSequencer::SequenceInterface {
	public:
		ImSequencerWrapper(int frameMin, int frameMax) :
			frameMin_(frameMin), frameMax_(frameMax) {
		}

		int GetFrameMin() const override {
			return frameMin_;
		}
		int GetFrameMax() const override {
			return frameMax_;
		}
		int GetItemCount() const override {
			return static_cast<int>(items_.size());
		}
		int GetItemTypeCount() const override {
			return 1;
		}
		void Get(int index, int** start, int** end, int* type, unsigned int* color) override {
			if (index < 0 || index >= static_cast<int>(items_.size())) {
				if (start) { *start = nullptr; }
				if (end) { *end = nullptr; }
				if (type) { *type = 0; }
				if (color) { *color = 0xFFFFFFFF; } // デフォルトの色
				return;
			}
			SequencerItem<T>& item = items_[index];
			if (start) { *start = &item.start; }
			if (end) { *end = &item.end; }
			if (type) { *type = item.type; }
			if (color) { *color = 0xFF00FF00; } // アイテムの色（例: 緑）
		}

		const char* GetItemTypeName(int /*typeIndex*/) const override {
			return "Clip";
		}
		const char* GetItemLabel(int index) const override {
			itemLabel_ = "Item " + std::to_string(index);
			return itemLabel_.c_str();
		}

		void Add(int type) override {
			SequencerItem<T> newItem;
			newItem.start = frameMin_;
			newItem.end = frameMin_ + 10; // デフォルトの長さ
			newItem.type = type;
			items_.push_back(newItem);
		}
		void Del(int index) override {
			if (index >= 0 && index < static_cast<int>(items_.size())) {
				items_.erase(items_.begin() + index);
			}
		}


	private:
		int frameMin_;
		int frameMax_;
		std::vector<SequencerItem<T>> items_;
		mutable std::string itemLabel_;
	};
}
