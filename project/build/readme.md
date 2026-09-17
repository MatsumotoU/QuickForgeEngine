# ProjectGenerator build

このディレクトリはQuickForgeEngine ProjectGeneratorが生成しました。

- エディタのルートディレクトリ名: `project`
- その一つ上のディレクトリ名: `QuickForgeEngine`

このエディタのルートディレクトリは、`QuickForgeEngine/project` に配置してください。

Premakeを実行するときは、この `build` ディレクトリをカレントディレクトリにしてください。

例:

```text
cd project/build
premake5 --file=premake.lua vs2022
```
