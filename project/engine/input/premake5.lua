project "Input"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}
        links{
            "EngineCore",
        }

        -- 警告レベル4
        warnings "Extra"

        -- 追加のインクルード
        includedirs {
            "%{wks.location}/engine/core/",
        }