---@meta

-- QuickForgeEngine Lua API definitions for Lua language servers.
-- This file is documentation only; ScriptComponent does not load it.

---@class Vector2
---@field x number
---@field y number
Vector2 = {}
---@return Vector2
function Vector2.new(x, y) end
---@return number
function Vector2:Length() end
---@return Vector2
function Vector2:Normalize() end

---@class Vector3
---@field x number
---@field y number
---@field z number
Vector3 = {}
---@return Vector3
function Vector3.new(x, y, z) end
---@return number
function Vector3:Length() end
---@return Vector3
function Vector3:Normalize() end
---@return number
function Vector3.Dot(a, b) end
---@return Vector3
function Vector3.Cross(a, b) end

---@class Vector4
---@field x number
---@field y number
---@field z number
---@field w number
Vector4 = {}
---@return Vector4
function Vector4.new(x, y, z, w) end
---@return number
function Vector4:Length() end
---@return Vector4
function Vector4:Normalize() end

---@class Transform
---@field scale Vector3
---@field rotate Vector3
---@field translate Vector3
Transform = {}
function Transform:AddForward(speed) end
function Transform:AddRight(speed) end

---@class PhysicsComponent
---@field velocity Vector3
---@field acceleration Vector3
---@field mass number
---@field friction number

---@class MaterialComponent
---@field color Vector4
---@field albedoColor Vector4
---@field metallic number
---@field smoothness number
---@field uvTransform Transform

---@type Transform|nil
transform = nil
---@type PhysicsComponent|nil
force = nil

---@param ... any
function DebugLog(...) end
---@return number
function GetDeltaTime() end
---@return integer
function GetThisEntityId() end
function destroy() end
function delete() end

this = {}
---@return integer
function this.GetEntityId() end

---@param name string
---@return integer|nil
function GetEntity(name) end
---@param entityId integer
---@return Transform|nil
function GetTransform(entityId) end
---@param entityId integer
---@return PhysicsComponent|nil
function GetForce(entityId) end
---@param entityId integer
---@return PhysicsComponent|nil
function GetPhysics(entityId) end
---@param entityId integer
---@return MaterialComponent|nil
function GetMaterial(entityId) end
---@param entityId integer
---@param value Vector3
---@return boolean
function SetTranslate(entityId, value) end
---@param entityId integer
---@param value Vector3
---@return boolean
function SetRotate(entityId, value) end
---@param entityId integer
---@param value Vector3
---@return boolean
function SetScale(entityId, value) end
---@param name string
---@return integer
function SimpleCreateEntity(name) end
---@param path string
function LoadScene(path) end

QFE = { Input = {}, Math = {} }

---@return Vector2
function QFE.Input.GetKeyMoveDir() end
---@param action string
---@return boolean
function QFE.Input.GetKeyPress(action) end
---@param action string
---@return boolean
function QFE.Input.GetKeyTrigger(action) end
---@param action string
---@return boolean
function QFE.Input.GetKeyRelease(action) end
---@param button integer
---@return boolean
function QFE.Input.GetMousePress(button) end
---@param button integer
---@return boolean
function QFE.Input.GetMouseTrigger(button) end
---@param button integer
---@return boolean
function QFE.Input.GetMouseRelease(button) end
---@return Vector2
function QFE.Input.GetMouseScreenPos() end
---@return Vector2
function QFE.Input.GetMouseMoveDir() end
---@return number
function QFE.Input.GetMouseWheelDir() end
---@param button integer
---@return boolean
function QFE.Input.GetGamePadPress(button) end
---@param button integer
---@return boolean
function QFE.Input.GetGamePadTrigger(button) end
---@param button integer
---@return boolean
function QFE.Input.GetGamePadRelease(button) end
---@return Vector2
function QFE.Input.GetGamePadLeftStickDir() end
---@return Vector2
function QFE.Input.GetGamePadRightStickDir() end

---@type number
QFE.Math.pi = math.pi
---@return number
function QFE.Math.Lerp(from, to, t) end
---@return number
function QFE.Math.Slerp(from, to, t) end
---@return number
function QFE.Math.SimpleEaseIn(from, to, t) end
---@return number
function QFE.Math.EaseIn(from, to, t) end
---@return number
function QFE.Math.EaseOut(from, to, t) end
---@return number
function QFE.Math.EaseInOut(from, to, t) end
---@return number
function QFE.Math.Rand(minimum, maximum) end

