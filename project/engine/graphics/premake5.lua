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

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/engine/core/",
            "%{wks.location}/externals/assimp/",
            "%{wks.location}/externals/assimp/include/",
            "%{wks.location}/externals/DirectXTex/",
            "%{wks.location}/externals/",
        }

        -- ライブラリのリンク
        filter "configurations:Debug"
            links { "assimp-vc143-mtd" }
            libdirs {
                "%{wks.location}/externals/assimp/lib/Debug"
            }
        filter "configurations:Release or configurations:Development"
            links { "assimp-vc143-mt" }
            libdirs {
                "%{wks.location}/externals/assimp/lib/Release"
            }
        filter ""