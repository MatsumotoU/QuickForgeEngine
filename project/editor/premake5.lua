project "GameEditor" -- Editor
        kind "WindowedApp" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}
        links{
            "EngineCore",
            "GraphicEngine",
        }

        -- 警告レベル4
        warnings "Extra"