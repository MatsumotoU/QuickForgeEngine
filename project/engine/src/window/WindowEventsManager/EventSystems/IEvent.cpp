#include "IEvent.h"
#include <cassert>

IEvent::IEvent(nlohmann::json& eventData) :eventData_(eventData){}