project "Runtime" -- Runtime
        kind "WindowedApp"
        language "C++"
        debugdir "%{wks.location}"
        files {
            "./**.h",
            "./**.cpp"
        }

        links{
            "EngineCore",
            "GuiFramework",
            "GraphicFramework",
            "WindowFramework",
            "EngineAssetsFactory",
            "SceneFramework",
            "ScriptFramework",
            "InputFramework",
            "ApplicationFramework",
            "Camera",
            "ImGui",
            "Components",
            "Input",
        }

        -- 警告レベル4
        warnings "Extra"

        -- 追加のインクルード
        includedirs {
            "%{wks.location}/engine/",
            "%{wks.location}/engine/core/",
        }

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/externals/",
            "%{wks.location}/externals/assimp/",
            "%{wks.location}/externals/assimp/include/",
            "%{wks.location}/externals/imgui/"
        }
