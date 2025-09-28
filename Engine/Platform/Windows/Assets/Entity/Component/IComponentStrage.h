#pragma once
class IComponentStrage{
public:
	virtual ~IComponentStrage() = default;
	virtual void RemoveComponent(uint32_t id) = 0;
};