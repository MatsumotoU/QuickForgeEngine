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
            "WindowFramework",
            "EngineAssetsFactory",
            "SceneFramework",
            "InputFramework",
            "ApplicationFramework",
            "PhysicsFramework",
            "Camera",
            "ImGui",
            "Components",
            "Input",
            "PhysicsEngine",
        }

        -- グラフィック関連の内部ライブラリはGraphicFramework側から伝播する。
        uses {
            "GraphicFramework",
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
