#pragma once
#include "pch.h"
#include <algorithm>
#include "Observer.h"

template <typename T, typename ValueType>
class Subject {
public:
    void AddObserver(Observer<ValueType>* observer) {
        observers_.push_back(observer);
    }
    void RemoveObserver(Observer<ValueType>* observer) {
        observers_.erase(std::remove(observers_.begin(), observers_.end(), observer), observers_.end());
    }
protected:
    void NotifyObservers(const ValueType& value) {
        for (auto* observer : observers_) {
            if (observer) observer->OnNotify(value);
        }
    }
private:
    std::vector<Observer<ValueType>*> observers_;
};