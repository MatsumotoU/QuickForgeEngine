#include "engine/include/assets/Script/QFElinker/LuaScriptOnQFESetMyMath.h"
#include "engine/include/core/Math/Vector/Vector3.h"
#include "engine/include/core/Math/MyMath.h"

void QFE::Script::MyLuaMath::LuaScriptOnQFESetMyMath(sol::state* luaState) {
	sol::table qfe = luaState->get<sol::table>("QFE");
	sol::table math = qfe.create_named("Math");

	math.set("pi", 3.14159265358979323846f);
	math.set_function("SimpleEaseIn", [](float from,float to,float speed) {
		return MyMath::SimpleEaseIn(from, to, speed);
		});
	math.set_function("Leap", [](float a, float b, float t) {
		return a * t + b * (1.0f - t);
		});
	math.set_function("Slerp", [](float from, float to, float t) {
		return MyMath::Slerp(from, to, t);
		});
	math.set_function("Slerp",[](const Vector3& v1, const Vector3& v2, float t) {
		return Vector3::Slerp(v1, v2, t);
		});
	math.set_function("EaseIn", [](float from, float to, float t) {
		return MyMath::EaseIn(from, to, t);
		});
	math.set_function("EaseOut", [](float from, float to, float t) {
		return MyMath::EaseOut(from, to, t);
		});
	math.set_function("EaseInOut", [](float from, float to, float t) {
		return MyMath::EaseInOut(from, to, t);
		});
	math.set_function("Rand", [](sol::object minObj, sol::object maxObj) {
		float min = 0.0f;
		float max = 1.0f;
		if (minObj.is<int>()) {
			min = static_cast<float>(minObj.as<int>());
		} else if (minObj.is<double>()) {
			min = static_cast<float>(minObj.as<double>());
		} else if (minObj.is<float>()) {
			min = minObj.as<float>();
		}
		if (maxObj.is<int>()) {
			max = static_cast<float>(maxObj.as<int>());
		} else if (maxObj.is<double>()) {
			max = static_cast<float>(maxObj.as<double>());
		} else if (maxObj.is<float>()) {
			max = maxObj.as<float>();
		}
		return MyMath::Rand(min, max);
		});

	math.set_function("LookAtFromDir", [](const Vector3& dir) {
		Vector3 lookAt;
		float yaw = atan2f(dir.x, dir.z);
		float pitch = asinf(-dir.y / dir.Length());
		lookAt.y = yaw;
		lookAt.x = pitch;
		return lookAt;
		});
}
