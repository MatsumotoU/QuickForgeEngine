project "Sandbox" -- Sandbox
        kind "WindowedApp"
        language "C++"
        debugdir "%{wks.location}"
        files {
            "./**.h",
            "./**.cpp"
        }

        -- 警告レベル4
        warnings "Extra"

        -- 外部ファイルのインクルード
        externalincludedirs {
            "../externals/",
            "../engine/",
            "../engine/core/"
        }