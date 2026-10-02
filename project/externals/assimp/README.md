# Assimp build

The `Assimp` node in `engine-project-1.4.2.json` is a normal Premake `StaticLib` project. It builds `Assimp.lib` into `generated/outputs/<configuration>/` with the rest of QuickForgeEngine. The `Assimp` dependency edge from `EngineAssetsFactory` controls build order and linking.

`upstream/` contains the source files from the official Assimp v5.3.0 archive (`https://github.com/assimp/assimp/archive/refs/tags/v5.3.0.zip`, SHA-256 `CCCBD20522B577613096B0B157F62C222F844BC177356B8301CD74EEE3FECADB`). The archive's `test/` and `samples/` directories are omitted. See `upstream/LICENSE` and the licenses within `upstream/contrib/`.

The `sourcePatterns` field in `engine-project-1.4.2.json` lists the Windows x64 source files selected by Assimp's v5.3.0 CMake configuration with static linkage, bundled zlib, and tests, tools, samples, and installation disabled. The generated `config.h`, `revision.h`, and `zconf.h` are stored with the source so a normal Premake build does not need CMake or a download step. When updating Assimp, regenerate the source list and these headers from the same upstream configuration, then rebuild all three QuickForgeEngine configurations.

The ProjectGenerator saves the `sourcePatterns` field and writes the selected files directly to `premake.lua`. An empty field uses the default C and C++ file patterns, including `.c`, `.cc`, `.cpp`, and `.cxx`.

The old `build-assimp.bat`, copied headers, and prebuilt libraries are retained for the older `engine-project-1.4.json` and `engine-project-1.4.1.json` snapshots. The 1.4.2 configuration does not use them.
