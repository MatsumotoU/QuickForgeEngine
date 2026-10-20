local _sourceDirectory = path.getabsolute(path.getdirectory(_SCRIPT))

project "ShootingGameEditor"
    location (_sourceDirectory)
    language "C++"
    kind "StaticLib"
    files {
        path.join(_sourceDirectory, "**.h"),
        path.join(_sourceDirectory, "**.cpp"),
    }
    includedirs {
        path.join(QFE_PROJECT_ROOT, "."),
        path.join(QFE_PROJECT_ROOT, "engine"),
        path.join(QFE_PROJECT_ROOT, "engine/core"),
        path.join(QFE_PROJECT_ROOT, "editor"),
        path.join(QFE_PROJECT_ROOT, "externals"),
        path.join(QFE_PROJECT_ROOT, "externals/imgui"),
    }
