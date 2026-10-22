local _sourceDirectory = path.getabsolute(path.getdirectory(_SCRIPT))

project "ShootingGame"
    location (_sourceDirectory)
    language "C++"
    kind "SharedLib"
    files {
        path.join(_sourceDirectory, "**.h"),
        path.join(_sourceDirectory, "**.cpp"),
    }
    includedirs {
        path.join(QFE_PROJECT_ROOT, "."),
        path.join(QFE_PROJECT_ROOT, "engine"),
        path.join(QFE_PROJECT_ROOT, "engine/core"),
        path.join(QFE_PROJECT_ROOT, "externals"),
        path.join(QFE_PROJECT_ROOT, "externals/assimp"),
        path.join(QFE_PROJECT_ROOT, "externals/assimp/upstream/include"),
        path.join(QFE_PROJECT_ROOT, "externals/imgui"),
    }
    links {
        "EngineAssetsFactory",
        "EngineCore",
        "Assimp",
        "ApplicationFramework",
        "GraphicFramework",
        "GraphicEngine",
        "DirectXTex",
        "GuiFramework",
        "GUIEngine",
        "ImGui",
        "InputFramework",
        "Input",
        "SceneFramework",
        "Scene",
        "WindowFramework",
        "WindowingSystem",
        "PhysicsFramework",
        "PhysicsEngine",
        "AudioFramework",
        "Audio",
        "Network",
        "AnimationFramework",
        "Animation",
    }
