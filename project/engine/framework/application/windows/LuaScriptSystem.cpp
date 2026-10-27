#define SOL_ALL_SAFETIES_ON 1
#include "framework/application/LuaScriptSystem.h"

#ifdef _MSC_VER
// sol2's member-pointer dispatch triggers this legacy ABI notice in current
// MSVC. The warning is not actionable for code built with one toolset.
#pragma warning(disable : 4371)
#endif

#include <sol2/sol/sol.hpp>

#include "EngineDefines.h"
#include "components/MaterialComponent.h"
#include "components/ObjectInfoComponent.h"
#include "components/PhysicsComponent.h"
#include "components/ScriptComponent.h"
#include "components/TransformComponent.h"
#include "design-patterns/EntityManager.h"
#include "input/interface/InputInterface.h"
#include "math/MathInclude.h"
#include "scene/SceneManager.h"

#include <algorithm>
#include <cstdint>
#include <numbers>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

namespace {
	using QFE::FRAMEWORK::LuaScriptSystem;

	std::string LuaValueToString(const sol::object& value) {
		if (value.is<std::string>()) return value.as<std::string>();
		if (value.is<const char*>()) return value.as<const char*>();
		if (value.is<bool>()) return value.as<bool>() ? "true" : "false";
		if (value.is<std::int64_t>()) return std::to_string(value.as<std::int64_t>());
		if (value.is<double>()) return std::to_string(value.as<double>());
		if (value == sol::lua_nil) return "nil";
		return "<" + std::string(sol::type_name(value.lua_state(), value.get_type())) + ">";
	}

	std::filesystem::path ResolveScriptPath(
		const std::filesystem::path& root,
		const std::string& configuredPath) {
		std::filesystem::path relative(configuredPath);
		if (relative.extension().empty()) relative += ".lua";
		if (relative.is_absolute()) return relative.lexically_normal();

		const std::filesystem::path normalized = relative.lexically_normal();
		if (!normalized.empty() && *normalized.begin() == "resources") {
			return normalized;
		}
		return (root / normalized).lexically_normal();
	}

	std::optional<std::filesystem::file_time_type> GetLastWriteTime(
		const std::filesystem::path& path) {
		std::error_code error;
		const auto result = std::filesystem::last_write_time(path, error);
		if (error) return std::nullopt;
		return result;
	}
}

class QFE::FRAMEWORK::LuaScriptSystem::Impl final {
public:
	struct Context final {
		QFE::EntityManager* entityManager = nullptr;
		QFE::SCENE::SceneManager* sceneManager = nullptr;
		QFE::INPUT::InputInterface* input = nullptr;
		Impl* owner = nullptr;
		std::uint32_t entityId = 0;
		float deltaTime = 0.0f;
	};

	struct Instance final {
		Instance() = default;
		Instance(const Instance&) = delete;
		Instance& operator=(const Instance&) = delete;
		Instance(Instance&&) noexcept = default;
		Instance& operator=(Instance&&) noexcept = default;

		std::uint32_t entityId = 0;
		std::int32_t executionOrder = 0;
		std::string configuredPath;
		std::filesystem::path resolvedPath;
		std::optional<std::filesystem::file_time_type> lastWriteTime;
		Context context;
		std::unique_ptr<sol::state> state;
		bool loaded = false;
	};

	void Initialize(std::filesystem::path scriptRoot) {
		scriptRoot_ = std::move(scriptRoot);
		initialized_ = true;
	}

	void Reset() {
		for (auto& [entityId, instance] : instances_) {
			(void)entityId;
			Call(*instance, "OnDestroy");
		}
		instances_.clear();
		lastSceneRevision_.reset();
		requestedScene_.reset();
	}

	void Update(
		QFE::SCENE::SceneManager& sceneManager,
		QFE::INPUT::InputInterface& input,
		float deltaTime) {
		if (!initialized_) Initialize("resources/Scripts");

		const std::uint64_t sceneRevision = sceneManager.GetSceneRevision();
		if (lastSceneRevision_.has_value() && *lastSceneRevision_ != sceneRevision) {
			Reset();
		}
		lastSceneRevision_ = sceneRevision;

		QFE::EntityManager& entityManager = sceneManager.GetCurrentSceneEntityManager();
		struct DesiredScript final {
			std::uint32_t entityId;
			std::int32_t executionOrder;
			std::string configuredPath;
		};
		std::vector<DesiredScript> desired;
		std::unordered_set<std::uint32_t> desiredEntities;

		entityManager.Each<QFE::SCENE::ScriptComponent>(
			[&](std::uint32_t entityId, QFE::SCENE::ScriptComponent& component) {
				if (!component.enabled || component.scriptFunctionName.empty()) return;
				desired.push_back({ entityId, component.executionOrder, component.scriptFunctionName });
				desiredEntities.insert(entityId);
			});

		for (auto iterator = instances_.begin(); iterator != instances_.end();) {
			if (!desiredEntities.contains(iterator->first)) {
				Call(*iterator->second, "OnDestroy");
				iterator = instances_.erase(iterator);
			} else {
				++iterator;
			}
		}

		for (const DesiredScript& script : desired) {
			auto iterator = instances_.find(script.entityId);
			const bool needsNewInstance = iterator == instances_.end() ||
				iterator->second->configuredPath != script.configuredPath;
			if (needsNewInstance) {
				if (iterator != instances_.end()) {
					Call(*iterator->second, "OnDestroy");
					instances_.erase(iterator);
				}
				auto instance = std::make_unique<Instance>();
				instance->entityId = script.entityId;
				instance->executionOrder = script.executionOrder;
				instance->configuredPath = script.configuredPath;
				instance->resolvedPath = ResolveScriptPath(scriptRoot_, script.configuredPath);
				instance->context = { &entityManager, &sceneManager, &input, this, script.entityId, deltaTime };
				Load(*instance);
				instances_.emplace(script.entityId, std::move(instance));
			} else {
				Instance& instance = *iterator->second;
				instance.executionOrder = script.executionOrder;
				instance.context.entityManager = &entityManager;
				instance.context.sceneManager = &sceneManager;
				instance.context.input = &input;
				instance.context.deltaTime = deltaTime;
			}
		}

		std::vector<Instance*> ordered;
		ordered.reserve(instances_.size());
		for (auto& [entityId, instance] : instances_) {
			(void)entityId;
			ordered.push_back(instance.get());
		}
		std::ranges::sort(ordered, [](const Instance* lhs, const Instance* rhs) {
			if (lhs->executionOrder != rhs->executionOrder) {
				return lhs->executionOrder < rhs->executionOrder;
			}
			return lhs->entityId < rhs->entityId;
			});

		for (Instance* instance : ordered) {
			instance->context.deltaTime = deltaTime;
			const auto currentWriteTime = GetLastWriteTime(instance->resolvedPath);
			if (reloadAllRequested_ || currentWriteTime != instance->lastWriteTime) {
				Call(*instance, "OnDestroy");
				Load(*instance);
			}
			RefreshEntityGlobals(*instance);
			Call(*instance, "Update", deltaTime);
		}
		reloadAllRequested_ = false;

		if (requestedScene_.has_value()) {
			const std::filesystem::path requested = std::move(*requestedScene_);
			requestedScene_.reset();
			Reset();
			sceneManager.LoadCurrentSceneFromJson(requested.string());
		}
	}

	void RequestReloadAll() {
		reloadAllRequested_ = true;
	}

	[[nodiscard]] std::size_t GetLoadedScriptCount() const {
		return static_cast<std::size_t>(std::ranges::count_if(
			instances_, [](const auto& item) { return item.second->loaded; }));
	}

private:
	void RegisterMath(sol::state& lua) {
		using QFE::MATH::EulerTransform;
		using QFE::MATH::Vector2;
		using QFE::MATH::Vector3;
		using QFE::MATH::Vector4;

		lua.new_usertype<Vector2>("Vector2",
			sol::constructors<Vector2(), Vector2(float, float)>(),
			"x", &Vector2::x, "y", &Vector2::y,
			"Length", &Vector2::Length, "Normalize", &Vector2::Normalize,
			sol::meta_function::addition, [](const Vector2& a, const Vector2& b) { return a + b; },
			sol::meta_function::subtraction, [](const Vector2& a, const Vector2& b) { return a - b; },
			sol::meta_function::multiplication, [](const Vector2& value, float scalar) { return value * scalar; });

		lua.new_usertype<Vector3>("Vector3",
			sol::constructors<Vector3(), Vector3(float, float, float)>(),
			"x", &Vector3::x, "y", &Vector3::y, "z", &Vector3::z,
			"Length", &Vector3::Length, "Normalize", sol::resolve<Vector3() const>(&Vector3::Normalize),
			"Dot", [](const Vector3& a, const Vector3& b) { return Vector3::Dot(a, b); },
			"Cross", [](const Vector3& a, const Vector3& b) { return Vector3::Cross(a, b); },
			sol::meta_function::addition, [](const Vector3& a, const Vector3& b) { return a + b; },
			sol::meta_function::subtraction, [](const Vector3& a, const Vector3& b) { return a - b; },
			sol::meta_function::multiplication, [](const Vector3& value, float scalar) { return value * scalar; });

		lua.new_usertype<Vector4>("Vector4",
			sol::constructors<Vector4(), Vector4(float, float, float, float)>(),
			"x", &Vector4::x, "y", &Vector4::y, "z", &Vector4::z, "w", &Vector4::w,
			"Length", &Vector4::Length, "Normalize", &Vector4::Normalize,
			sol::meta_function::addition, [](const Vector4& a, const Vector4& b) { return a + b; },
			sol::meta_function::subtraction, [](const Vector4& a, const Vector4& b) { return a - b; },
			sol::meta_function::multiplication, [](const Vector4& value, float scalar) { return value * scalar; });

		lua.new_usertype<EulerTransform>("Transform",
			sol::constructors<EulerTransform()>(),
			"scale", &EulerTransform::scale,
			"rotate", &EulerTransform::rotate,
			"translate", &EulerTransform::translate,
			"AddForward", [](EulerTransform& transform, float speed) {
				transform.translate += QFE::MATH::TransformForwardDirection(transform) * speed;
			},
			"AddRight", [](EulerTransform& transform, float speed) {
				transform.translate += QFE::MATH::TransformRightDirection(transform) * speed;
			});
	}

	void RegisterComponents(sol::state& lua) {
		using QFE::COMPONENTS::PhysicsComponent;
		using QFE::SCENE::MaterialComponent;
		using QFE::SCENE::ObjectInfoComponent;
		using QFE::SCENE::TransformComponent;

		lua.new_usertype<TransformComponent>("TransformComponent",
			sol::no_constructor, "transform", &TransformComponent::transform);
		lua.new_usertype<PhysicsComponent>("PhysicsComponent",
			sol::no_constructor,
			"velocity", &PhysicsComponent::velocity,
			"acceleration", &PhysicsComponent::acceleration,
			"mass", &PhysicsComponent::mass,
			"friction", &PhysicsComponent::friction);
		lua.new_usertype<MaterialComponent>("MaterialComponent",
			sol::no_constructor,
			"color", &MaterialComponent::albedoColor,
			"albedoColor", &MaterialComponent::albedoColor,
			"metallic", &MaterialComponent::metallic,
			"smoothness", &MaterialComponent::smoothness,
			"uvTransform", &MaterialComponent::uvTransform);
		lua.new_usertype<ObjectInfoComponent>("ObjectInfoComponent",
			sol::no_constructor,
			"name", &ObjectInfoComponent::name,
			"uuid", sol::readonly(&ObjectInfoComponent::uuid),
			"hierarchyGroup", &ObjectInfoComponent::hierarchyGroup);
	}

	void RegisterFunctions(Instance& instance) {
		sol::state& lua = *instance.state;
		Context* context = &instance.context;
		RegisterMath(lua);
		RegisterComponents(lua);

		lua.set_function("DebugLog", [context](sol::variadic_args arguments) {
			std::ostringstream stream;
			bool first = true;
			for (const sol::object value : arguments) {
				if (!first) stream << '\t';
				stream << LuaValueToString(value);
				first = false;
			}
			QFE_LOG("[Lua:" + std::to_string(context->entityId) + "] " + stream.str(), QFE::LogLevel::EngineInfo);
		});
		lua.set_function("GetDeltaTime", [context]() { return context->deltaTime; });
		lua.set_function("GetThisEntityId", [context]() { return context->entityId; });
		lua.set_function("destroy", [context]() {
			if (context->entityManager != nullptr) context->entityManager->RemoveEntity(context->entityId);
		});
		lua["delete"] = lua["destroy"];

		sol::table thisEntity = lua.create_named_table("this");
		thisEntity.set_function("GetEntityId", [context]() { return context->entityId; });

		lua.set_function("GetEntity", [context](const std::string& name) -> sol::optional<std::uint32_t> {
			if (context->entityManager == nullptr) return sol::nullopt;
			sol::optional<std::uint32_t> result = sol::nullopt;
			context->entityManager->Each<QFE::SCENE::ObjectInfoComponent>(
				[&](std::uint32_t entityId, QFE::SCENE::ObjectInfoComponent& objectInfo) {
					if (!result.has_value() && objectInfo.name == name) result = entityId;
				});
			return result;
		});
		lua.set_function("GetTransform", [context](std::uint32_t entityId) {
			if (context->entityManager == nullptr) return static_cast<QFE::MATH::EulerTransform*>(nullptr);
			auto* component =
				context->entityManager->GetComponentPtr<QFE::SCENE::TransformComponent>(entityId);
			return component != nullptr ? &component->transform : nullptr;
		});
		lua.set_function("GetForce", [context](std::uint32_t entityId) {
			return context->entityManager != nullptr
				? context->entityManager->GetComponentPtr<QFE::COMPONENTS::PhysicsComponent>(entityId)
				: nullptr;
		});
		lua.set_function("GetPhysics", [context](std::uint32_t entityId) {
			return context->entityManager != nullptr
				? context->entityManager->GetComponentPtr<QFE::COMPONENTS::PhysicsComponent>(entityId)
				: nullptr;
		});
		lua.set_function("GetMaterial", [context](std::uint32_t entityId) {
			return context->entityManager != nullptr
				? context->entityManager->GetComponentPtr<QFE::SCENE::MaterialComponent>(entityId)
				: nullptr;
		});
		lua.set_function("SetTranslate", [context](std::uint32_t entityId, const QFE::MATH::Vector3& value) {
			if (context->entityManager == nullptr) return false;
			auto* transform = context->entityManager->GetComponentPtr<QFE::SCENE::TransformComponent>(entityId);
			if (transform == nullptr) return false;
			transform->transform.translate = value;
			return true;
		});
		lua.set_function("SetRotate", [context](std::uint32_t entityId, const QFE::MATH::Vector3& value) {
			if (context->entityManager == nullptr) return false;
			auto* transform = context->entityManager->GetComponentPtr<QFE::SCENE::TransformComponent>(entityId);
			if (transform == nullptr) return false;
			transform->transform.rotate = value;
			return true;
		});
		lua.set_function("SetScale", [context](std::uint32_t entityId, const QFE::MATH::Vector3& value) {
			if (context->entityManager == nullptr) return false;
			auto* transform = context->entityManager->GetComponentPtr<QFE::SCENE::TransformComponent>(entityId);
			if (transform == nullptr) return false;
			transform->transform.scale = value;
			return true;
		});
		lua.set_function("SimpleCreateEntity", [context](const std::string& name) {
			if (context->entityManager == nullptr) return std::uint32_t{ 0 };
			const std::uint32_t entityId = context->entityManager->CreateEntity();
			context->entityManager->EmplaceComponent<QFE::SCENE::ObjectInfoComponent>(entityId);
			context->entityManager->EmplaceComponent<QFE::SCENE::TransformComponent>(entityId);
			context->entityManager->GetComponent<QFE::SCENE::ObjectInfoComponent>(entityId).name = name;
			return entityId;
		});
		lua.set_function("LoadScene", [context](const std::string& sceneName) {
			if (context->owner == nullptr) return;
			std::filesystem::path path(sceneName);
			if (path.extension().empty()) path += ".json";
			if (!path.is_absolute() && !path.string().starts_with("resources")) {
				path = std::filesystem::path("resources/scene") / path;
			}
			context->owner->requestedScene_ = path.lexically_normal();
		});

		sol::table qfe = lua.create_named_table("QFE");
		sol::table input = lua.create_table();
		input.set_function("GetKeyMoveDir", [context]() {
			return context->input != nullptr ? context->input->GetKeyMoveDir() : QFE::MATH::Vector2{};
		});
		input.set_function("GetKeyPress", [context](const std::string& action) {
			return context->input != nullptr && context->input->GetKeyPress(action);
		});
		input.set_function("GetKeyTrigger", [context](const std::string& action) {
			return context->input != nullptr && context->input->GetKeyTrigger(action);
		});
		input.set_function("GetKeyRelease", [context](const std::string& action) {
			return context->input != nullptr && context->input->GetKeyRelease(action);
		});
		input.set_function("GetMousePress", [context](std::int8_t button) {
			return context->input != nullptr && context->input->GetMousePress(button);
		});
		input.set_function("GetMouseTrigger", [context](std::int8_t button) {
			return context->input != nullptr && context->input->GetMouseTrigger(button);
		});
		input.set_function("GetMouseRelease", [context](std::int8_t button) {
			return context->input != nullptr && context->input->GetMouseRelease(button);
		});
		input.set_function("GetMouseScreenPos", [context]() {
			return context->input != nullptr ? context->input->GetMouseScreenPos() : QFE::MATH::Vector2{};
		});
		input.set_function("GetMouseMoveDir", [context]() {
			return context->input != nullptr ? context->input->GetMouseMove() : QFE::MATH::Vector2{};
		});
		input.set_function("GetMouseWheelDir", [context]() {
			return context->input != nullptr ? context->input->GetMouseWheelDir() : 0.0f;
		});
		input.set_function("GetGamePadPress", [context](std::uint16_t button) {
			return context->input != nullptr && context->input->GetGamePadPress(button);
		});
		input.set_function("GetGamePadTrigger", [context](std::uint16_t button) {
			return context->input != nullptr && context->input->GetGamePadTrigger(button);
		});
		input.set_function("GetGamePadRelease", [context](std::uint16_t button) {
			return context->input != nullptr && context->input->GetGamePadRelease(button);
		});
		input.set_function("GetGamePadLeftStickDir", [context]() {
			return context->input != nullptr ? context->input->GetGamePadLeftStickDir() : QFE::MATH::Vector2{};
		});
		input.set_function("GetGamePadRightStickDir", [context]() {
			return context->input != nullptr ? context->input->GetGamePadRightStickDir() : QFE::MATH::Vector2{};
		});
		qfe["Input"] = input;

		sol::table math = lua.create_table();
		math["pi"] = std::numbers::pi_v<float>;
		math.set_function("Lerp", &QFE::MATH::Lerp);
		math.set_function("Slerp", sol::resolve<float(float, float, float)>(&QFE::MATH::Slerp));
		math.set_function("SimpleEaseIn", sol::resolve<float(float, float, float)>(&QFE::MATH::SimpleEaseIn));
		math.set_function("EaseIn", [](float from, float to, float t) { return QFE::MATH::EaseIn(from, to, t); });
		math.set_function("EaseOut", [](float from, float to, float t) { return QFE::MATH::EaseOut(from, to, t); });
		math.set_function("EaseInOut", [](float from, float to, float t) { return QFE::MATH::EaseInOut(from, to, t); });
		math.set_function("Rand", [](sol::optional<float> minimum, sol::optional<float> maximum) {
			return QFE::MATH::Rand(minimum.value_or(0.0f), maximum.value_or(1.0f));
		});
		qfe["Math"] = math;
	}

	void RefreshEntityGlobals(Instance& instance) {
		if (!instance.state || instance.context.entityManager == nullptr) return;
		QFE::EntityManager& entities = *instance.context.entityManager;
		const std::uint32_t entityId = instance.entityId;
		auto* transform = entities.GetComponentPtr<QFE::SCENE::TransformComponent>(entityId);
		(*instance.state)["transform"] = transform != nullptr ? &transform->transform : nullptr;
		(*instance.state)["force"] = entities.GetComponentPtr<QFE::COMPONENTS::PhysicsComponent>(entityId);
	}

	void Load(Instance& instance) {
		instance.loaded = false;
		instance.lastWriteTime = GetLastWriteTime(instance.resolvedPath);
		if (!instance.lastWriteTime.has_value()) {
			QFE_LOG("Lua script not found: " + instance.resolvedPath.generic_string(), QFE::LogLevel::Error);
			return;
		}

		try {
			instance.state = std::make_unique<sol::state>();
			instance.state->open_libraries(
				sol::lib::base,
				sol::lib::package,
				sol::lib::math,
				sol::lib::string,
				sol::lib::table,
				sol::lib::coroutine,
				sol::lib::utf8);

			RegisterFunctions(instance);
			RefreshEntityGlobals(instance);
			sol::table package = (*instance.state)["package"];
			const std::string root = scriptRoot_.generic_string();
			package["path"] = root + "/?.lua;" + root + "/?/init.lua";

			sol::protected_function_result result = instance.state->safe_script_file(
				instance.resolvedPath.string(), sol::script_pass_on_error);
			if (!result.valid()) {
				const sol::error error = result;
				QFE_LOG("Lua load error [" + instance.resolvedPath.generic_string() + "]: " +
					std::string(error.what()), QFE::LogLevel::Error);
				return;
			}

			instance.loaded = true;
			if (HasFunction(instance, "Init")) {
				Call(instance, "Init");
			} else {
				Call(instance, "OnCreate");
			}
		}
		catch (const std::exception& exception) {
			QFE_LOG("Lua initialization error [" + instance.resolvedPath.generic_string() + "]: " +
				exception.what(), QFE::LogLevel::Error);
			instance.state.reset();
		}
	}

	bool HasFunction(const Instance& instance, const char* functionName) const {
		if (!instance.loaded || !instance.state) return false;
		return (*instance.state)[functionName].is<sol::protected_function>();
	}

	template <typename... Args>
	bool Call(Instance& instance, const char* functionName, Args&&... arguments) {
		if (!instance.loaded || !instance.state) return false;
		sol::object object = (*instance.state)[functionName];
		if (!object.is<sol::protected_function>()) return false;
		sol::protected_function function = object.as<sol::protected_function>();
		sol::protected_function_result result = function(std::forward<Args>(arguments)...);
		if (!result.valid()) {
			const sol::error error = result;
			QFE_LOG("Lua runtime error [" + instance.resolvedPath.generic_string() + ":" +
				functionName + "]: " + std::string(error.what()), QFE::LogLevel::Error);
			return false;
		}
		return true;
	}

	std::filesystem::path scriptRoot_ = "resources/Scripts";
	std::unordered_map<std::uint32_t, std::unique_ptr<Instance>> instances_;
	std::optional<std::uint64_t> lastSceneRevision_;
	std::optional<std::filesystem::path> requestedScene_;
	bool initialized_ = false;
	bool reloadAllRequested_ = false;
};

QFE::FRAMEWORK::LuaScriptSystem::LuaScriptSystem() : impl_(std::make_unique<Impl>()) {}
QFE::FRAMEWORK::LuaScriptSystem::~LuaScriptSystem() = default;
QFE::FRAMEWORK::LuaScriptSystem::LuaScriptSystem(LuaScriptSystem&&) noexcept = default;
QFE::FRAMEWORK::LuaScriptSystem& QFE::FRAMEWORK::LuaScriptSystem::operator=(LuaScriptSystem&&) noexcept = default;

void QFE::FRAMEWORK::LuaScriptSystem::Initialize(std::filesystem::path scriptRoot) {
	impl_->Initialize(std::move(scriptRoot));
}

void QFE::FRAMEWORK::LuaScriptSystem::Update(
	QFE::SCENE::SceneManager& sceneManager,
	QFE::INPUT::InputInterface& input,
	float deltaTime) {
	impl_->Update(sceneManager, input, deltaTime);
}

void QFE::FRAMEWORK::LuaScriptSystem::RequestReloadAll() {
	impl_->RequestReloadAll();
}

void QFE::FRAMEWORK::LuaScriptSystem::Reset() {
	impl_->Reset();
}

void QFE::FRAMEWORK::LuaScriptSystem::Shutdown() {
	impl_->Reset();
}

std::size_t QFE::FRAMEWORK::LuaScriptSystem::GetLoadedScriptCount() const {
	return impl_->GetLoadedScriptCount();
}
