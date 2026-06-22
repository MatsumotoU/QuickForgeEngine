project "GraphicEngine"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}
        links{
            "EngineCore",
            "DirectXTex",
        }

        -- 警告レベル4
        warnings "Extra"

        -- 追加のインクルード
        includedirs {
            "%{wks.location}/engine/core/",
            "%{wks.location}/engine/graphics/"
        }

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/externals/DirectXTex/",
            "%{wks.location}/externals/",
        }