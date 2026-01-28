#include "engine/include/window/windowEventsManager/EventSystems/IEvent.h"
#include <cassert>
using namespace QFE;
IEvent::IEvent(nlohmann::json& eventData) :eventData_(eventData){}
