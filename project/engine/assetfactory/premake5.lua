project "EngineAssetsFactory"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"./**.h","./**.cpp"}

        -- 警告レベル4
        warnings "Extra"
        
        -- リンク
        links{
            "EngineCore",
            "ExternalFolders",
        }

        -- 追加のインクルード
        includedirs {
            "%{wks.location}/engine/core/",
        }

        -- 外部ファイルのインクルード
        externalincludedirs {
            "%{wks.location}/externals/",
            "%{wks.location}/externals/assimp/",
            "%{wks.location}/externals/assimp/include/",
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