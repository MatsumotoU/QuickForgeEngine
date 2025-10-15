#pragma once
#include "../IEditorUI.h"

class KeyConfigEdit final : public IEditorUI {
public:
	KeyConfigEdit() = default;
	~KeyConfigEdit() = default;
	void Initialize() override;
	void Update() override;
	void Draw() override;
	void Run() override;
private:
	char inputBuf_[256] = {};
	bool isEditPopupOpen_ = false;
	std::string editActionName_;
	size_t editKeyIndex_ = 0;
	bool isAddPopupOpen_ = false;
	void AddKeyConfig(const char* actionName, uint32_t keyId);
	void ClearKeyConfig(const char* actionName);
};