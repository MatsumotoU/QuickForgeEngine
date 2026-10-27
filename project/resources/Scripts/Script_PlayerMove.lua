-- scene.json で参照されている最小の移動サンプルです。
-- ScriptComponent の scriptFunctionName に拡張子なしで指定できます。

local moveSpeed = 1.0

function Update(deltaTime)
    if transform == nil then
        return
    end

    transform.translate.x = transform.translate.x + moveSpeed * deltaTime
end
