project "GraphicFramework"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}
        links{
            "EngineCore",
            "DirectXTex",
            "GraphicEngine",
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