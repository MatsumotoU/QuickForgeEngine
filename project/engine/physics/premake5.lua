project "PhysicsEngine"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}
        removefiles {"./tests/**"}
        links{
            "EngineCore",
        }

        -- 警告レベル4
        warnings "Extra"

        -- 追加のインクルード
        includedirs {
            "%{wks.location}/engine/",
            "%{wks.location}/engine/core/",
        }

project "PhysicsEngineTests"
        kind "ConsoleApp"
        language "C++"
        debugdir "%{wks.location}"
        files {"./tests/**.h", "./tests/**.cpp"}
        links {
            "PhysicsEngine",
            "EngineCore",
        }
        warnings "Extra"
        includedirs {
            "%{wks.location}/engine/",
            "%{wks.location}/engine/core/",
        }
