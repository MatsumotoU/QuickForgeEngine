#include "LuaScriptOnQFESetStructBase.h"
#include "Core/Math/Vector/Vector2.h"
#include "Core/Math/Vector/Vector3.h"
#include "Core/Math/Vector/Vector4.h"
#include "Core/Math/Transform.h"

#include "Scene/Data/SceneObjectData.h"
#include "Physics/Force.h"
#include "Resources/Shaders/ShaderStructs/hlslTypeToCpp.h"

void QFE::Script::Base::SetOnQFESetStructBase(sol::state* luaState) {
	// シーン上のオブジェクトデータ
	luaState->new_usertype<SceneObjectData>("SceneObjectData",
		"name", &SceneObjectData::name,
		"tag", &SceneObjectData::tag
	);

	// Math
	luaState->new_usertype<Vector2>("Vector2",
		sol::constructors<Vector2(), Vector2(float, float)>(),
		"x", &Vector2::x,
		"y", &Vector2::y,

		"Length", &Vector2::Length,
		"Normalize", &Vector2::Normalize
	);
	luaState->new_usertype<Vector3>("Vector3",
		sol::constructors<Vector3(), Vector3(float, float, float)>(),
		"x", &Vector3::x,
		"y", &Vector3::y,
		"z", &Vector3::z,

		"Length", &Vector3::Length,
		"Normalize", sol::resolve<Vector3() const>(&Vector3::Normalize),
		"Dot", & Vector3::Dot,
		"Cross", & Vector3::Cross,

		sol::meta_function::addition, [](const Vector3& a, const Vector3& b) { return a + b; },
		sol::meta_function::subtraction, [](const Vector3& a, const Vector3& b) { return a - b; },
		sol::meta_function::unary_minus, [](const Vector3& v) { return -v; },
		sol::meta_function::multiplication, [](const Vector3& v, float scalar) { return v * scalar; },
		sol::meta_function::multiplication, [](float scalar, const Vector3& v) { return v * scalar; },
		sol::meta_function::multiplication, [](const Vector3& a, const Vector3& b) { return Vector3(a.x * b.x, a.y * b.y, a.z * b.z); },
		sol::meta_function::division, [](const Vector3& v, float scalar) { return v / scalar; }
	);
	luaState->new_usertype<Vector4>("Vector4",
		sol::constructors<Vector4(), Vector4(float, float, float, float)>(),
		"x", &Vector4::x,
		"y", &Vector4::y,
		"z", &Vector4::z,
		"w", &Vector4::w,

		"Length", &Vector4::Length,
		"Normalize", &Vector4::Normalize
	);
	luaState->new_usertype<Transform>("Transform",
		sol::constructors<Transform()>(),
		"scale", &Transform::scale,
		"rotate", &Transform::rotate,
		"translate", &Transform::translate,
		"AddForward", &Transform::AddForward,
		"AddRight", &Transform::AddRight
	);

	// 物理力
	luaState->new_usertype<Force>("Force",
		sol::constructors<Force()>(),
		"velocity", &Force::velocity,
		"acceleration", &Force::acceleration,
		"mass", &Force::mass,
		"friction", &Force::friction,
		"gravityStrength", &Force::gravityStrength,
		"isGravity", &Force::isGravity
	);

	// マテリアル
	luaState->new_usertype<Material>("Material",
		sol::constructors<Material()>(),
		"color", &Material::color,
		"enableLighting", &Material::enableLighting,
		"uvTransform", &Material::uvTransform
	);
}
