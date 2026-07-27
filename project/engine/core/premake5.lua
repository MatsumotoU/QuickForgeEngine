project "EngineCore"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}

        -- 警告レベル4
        warnings "Extra"

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/externals/",
        }