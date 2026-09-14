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

        -- GraphicFrameworkを利用するプロジェクトへ伝播させるリンク設定。
        -- アプリケーション側はuses { "GraphicFramework" }だけを指定する。
        usage "INTERFACE"
                links {
                        "GraphicFramework",
                        "GraphicEngine",
                        "EngineCore",
                        "DirectXTex",
                }
