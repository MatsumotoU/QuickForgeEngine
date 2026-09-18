# ProjectGenerator build

このディレクトリはQuickForgeEngine ProjectGeneratorが生成しました。

このエディタのルートディレクトリは、`QuickForgeEngine/project` に配置してください。

別の場所へ移動する場合は、`premake5.lua` の `_sourceRootOverride` だけを変更してください。各プロジェクトのPremakeは変更不要です。

Premakeを実行するときは、この `build` ディレクトリをカレントディレクトリにしてください。

例:

```text
cd project/build
premake5 vs2022
```
