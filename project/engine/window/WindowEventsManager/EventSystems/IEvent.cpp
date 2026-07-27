#include "IEvent.h"
#include <cassert>

using namespace QFE;
IEvent::IEvent(nlohmann::json& eventData) :eventData_(eventData){}
