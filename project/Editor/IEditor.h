#pragma once
class IEditor {
public:
	virtual ~IEditor() = default;
	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Draw() = 0;
};