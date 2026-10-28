# QuickForgeEngine Lua scripts

`ScriptComponent.scriptFunctionName` に、このフォルダーからの相対パスを指定します。
`.lua` は省略できます。たとえば `Script_PlayerMove` は
`resources/Scripts/Script_PlayerMove.lua` を読み込みます。

## ライフサイクル

```lua
function Init()             -- 読み込み直後に1回（OnCreateも使用可能）
end

function Update(deltaTime)  -- 有効な間、毎フレーム
end

function OnDestroy()        -- コンポーネント削除・シーン変更・再読込時
end
```

ファイルを保存すると更新時刻の変更を検出して自動的に再読み込みします。
各ScriptComponentは独立したLua stateを持つため、スクリプトのグローバル変数は
別エンティティと共有されません。

## 主なAPI

- `transform`: このエンティティのTransform。`translate`、`rotate`、`scale`を持ちます。
- `force`: このエンティティのPhysicsComponent。存在しない場合は`nil`です。
- `GetDeltaTime()` / `GetThisEntityId()` / `DebugLog(...)`
- `GetEntity(name)` / `GetTransform(id)` / `GetPhysics(id)` / `GetMaterial(id)`
- `SetTranslate(id, value)` / `SetRotate(id, value)` / `SetScale(id, value)`
- `SimpleCreateEntity(name)` / `destroy()` / `LoadScene(path)`
- `QFE.Input.GetKeyPress(action)`、`GetKeyTrigger`、`GetKeyRelease`
- `QFE.Input.GetKeyMoveDir()`、マウス・ゲームパッド入力
- `QFE.Math.Lerp`、`Slerp`、`SimpleEaseIn`、`EaseIn`、`EaseOut`、`EaseInOut`、`Rand`
- `Vector2` / `Vector3` / `Vector4` / `Transform`

Luaの標準ライブラリは `base`、`package`、`math`、`string`、`table`、
`coroutine`、`utf8` を開いています。エンジン埋め込み用途のため、`io`、`os`、
`debug` は公開していません。
