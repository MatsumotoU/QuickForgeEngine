project "FrameworkCore"
        kind "StaticLib"
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}
        links{
            "EngineCore",
            "Components",
            "EngineAssetsFactory",
            "GraphicFramework",
            "SceneFramework",
            "InputFramework",
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
        }