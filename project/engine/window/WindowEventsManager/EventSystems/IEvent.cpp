#include "IEvent.h"
#include <cassert>

namespace QFE::WINDOW {

IEvent::IEvent(nlohmann::json& eventData) :eventData_(eventData){}

}
