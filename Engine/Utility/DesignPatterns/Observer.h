#pragma once
template <typename T>
class Observer {
public:
    virtual ~Observer() = default;
    virtual void OnNotify(const T& value) = 0;
};