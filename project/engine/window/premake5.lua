project "WindowingSystem"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}
        links{
            "EngineCore",
        }

        -- Debug or Developmentビルド時にImGuiをリンクする
        filter { "configurations:Debug or Development" }
            links {
                "ImGui"
            }
        filter {}

        -- 警告レベル4
        warnings "Extra"

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/engine/core/",
            "%{wks.location}/externals/",
        }