#include "engine/include/window/windowEventsManager/EventSystems/IEvent.h"
#include <cassert>

IEvent::IEvent(nlohmann::json& eventData) :eventData_(eventData){}
