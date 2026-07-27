project "Scene"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}

        -- 警告レベル4
        warnings "Extra"

        links{
            "EngineCore",
            "Components",
        }

        -- 追加のインクルード
        includedirs {
            "%{wks.location}/engine/core/",
            "%{wks.location}/engine/scene/",
            "%{wks.location}/engine/components/",
        }

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/externals/",
        }