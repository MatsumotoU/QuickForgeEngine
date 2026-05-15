-- Solution
workspace "QuickForgeEngine"
    architecture "x64"
    configurations {"Debug","Development","Release"}
    startproject "Editor"

    flags { "MultiProcessorCompile" }

    cppdialect "C++20"
    staticruntime "on"

    -- projectDefaultSetting
    objdir ("../generated/obj/%{prj.name}/%{cfg.buildcfg}/%{cfg.platform}")
    targetdir ("../generated/outputs/%{cfg.buildcfg}/%{cfg.platform}")

    -- Windows SDK とコンパイラ設定
    filter "system:windows"
        systemversion "latest"
        -- ビルドオプション
        buildoptions {
            "/permissive-",
            "/Zc:__cplusplus",
            "/utf-8",
            "/openmp"
        }
        linkoptions { "/ignore:4099" }
    filter ""
    
    filter "configurations:Debug"
        defines { "_DEBUG", "QFE_OPTIMIZE_OFF","QFE_MODE_DEBUG"}
        optimize "Off"
        symbols "On"
        runtime "Debug"
    filter "configurations:Development"
        defines { "NDEBUG", "QFE_OPTIMIZE_OFF","QFE_MODE_DEVELOPMENT" } 
        optimize "Off"  -- プロジェクト全体設定は無効（VS上で最適化「無効」に見える）
        symbols "On"     -- デバッグ情報を出す
        runtime "Release"
    filter ""

    -- 個別のファイルを対象に最適化を有効化（Development構成のみ）
    filter { "configurations:Development", "files:**.cpp" }
        optimize "On"
    filter ""
    filter "configurations:Release"
        defines { "NDEBUG", "QFE_OPTIMIZE_ON","QFE_MODE_RELEASE" }
        optimize "On"
        symbols "Off"
        runtime "Release"
    filter ""
    
group "QuickForge" -- MyMainProject
    project "Editor" -- Editor
        location "editor"
        kind "WindowedApp"
        language "C++"
        debugdir "%{wks.location}"
        files {
            "editor/**.h",
            "editor/**.cpp"
        }
        links{
            "Engine",
            "ExternalFolders"
        }

        -- 警告レベル4
        warnings "Extra"
        flags { "FatalWarnings" } --すべての警告をエラーとします
    
        -- 外部ファイルのインクルード
        externalincludedirs {
            "./externals/",
            "./externals/sol2",
            "./externals/assimp/",
            "./externals/assimp/include/",
            "./externals/DirectXTex/",
            "./externals/imgui/",
            "./externals/Mono/",
            "./externals/Mono/include",
            "./externals/Mono/include/mono-2.0/",
            "./externals/nlohmann/",
        }
        -- 追加のインクルード
        includedirs{
            "./",
            "./engine/include/",
        }

        -- リソースのコピー（Release構成のみ）
        filter "configurations:Release"
            postbuildcommands {
                '{COPYDIR} "../Resources" "%{cfg.targetdir}/Resources"',
            }
        filter ""

        -- MonoとWindows SDKのDLLをコピー
        postbuildcommands {
            -- フォルダのコピー
            '{COPYDIR} "../engine/resources" "%{cfg.targetdir}/engine/resources"',
            '{COPYDIR} "../externals/Mono/lib" "%{cfg.targetdir}/mono/lib"',
            '{COPYDIR} "../externals/Mono/etc" "%{cfg.targetdir}/mono/etc"',
    
            -- ファイルのコピー
            '{COPY} "../externals/Mono/bin/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
            '{COPY} "$(WindowsSdkDir)bin/$(TargetPlatformVersion)/x64/dxcompiler.dll" "%{cfg.targetdir}"',
            '{COPY} "$(WindowsSdkDir)bin/$(TargetPlatformVersion)/x64/dxil.dll" "%{cfg.targetdir}"'
        }       

    project "Engine" -- Engine
        location "engine"
        kind "StaticLib" 
        language "C++"
        debugdir "%{wks.location}"
        files {"engine/**.h","engine/**.cpp"}
        links{
            "DirectXTex",
            "ImGui",
            "mono-2.0-sgen",
            "ExternalFolders"
        }
        libdirs {
            "externals/Mono/lib"
        }

        -- 警告レベル4
        warnings "Extra"

        -- 外部ファイルのインクルード
        externalincludedirs {
            "./externals/",
            "./externals/sol2",
            "./externals/assimp/",
            "./externals/assimp/include/",
            "./externals/DirectXTex/",
            "./externals/imgui/",
            "./externals/Mono/",
            "./externals/Mono/include",
            "./externals/Mono/include/mono-2.0/",
            "./externals/nlohmann/",
        }

        -- 追加のインクルード
        includedirs{
            "./",
            "./engine/include/",
        }

        prebuildcommands {
            'call "%{wks.location}/Resources/Scripts/setup_vscode_settings.bat" /silent',
            'call "%{wks.location}/GenerateBuildInfo.bat"',
            'call "%{wks.location}/externals/Mono/setup_mono.bat"'
        }

        filter "configurations:Debug"
            links { "assimp-vc143-mtd" }
            libdirs {
                "externals/assimp/lib/Debug"
            }
        filter "configurations:Release or configurations:Development"
            links { "assimp-vc143-mt" }
            libdirs {
                "externals/assimp/lib/Release"
            }
        filter ""

    project "launcher" -- Launcher
        location "launcher"
        kind "StaticLib" 
        language "C++"
        
        debugdir "%{wks.location}"
group ""

-- MySubProject
project "Resource"
    location "Resources"
    kind "None"

group "External"
project "ExternalFolders"
    location "externals"
    kind "None"
    files {
        "externals/**"
    }

-- ExternalsProject
project "DirectXTex"
    location "externals/DirectXTex"
    kind "StaticLib" 
    language "C++"

    includedirs {
        "externals/DirectXTex", 
        "externals/DirectXTex/Shaders/Compiled"
    }

    files {
        "externals/DirectXTex/**.cpp",
        "externals/DirectXTex/**.h",
        "externals/DirectXTex/**.inl"
    }

    filter "system:windows"
        prebuildcommands {
            "cd Shaders && CompileShaders.cmd"
        }

project "ImGui"
    location "externals/imgui"
    kind "StaticLib" 
    language "C++"
    includedirs {
        "externals/imgui", 
    }

    files {
        "externals/imgui/**.h",
        "externals/imgui/**.cpp",
    }

group "Docs"
    project "DevelopmentRules"
        kind "None" -- コンパイルしない設定
        location "./" -- slnと同じ場所に置く
   
        files { 
            "DEVELOPMENT_RULE.md", 
        }

    vpaths {
        ["/*"] = { "**.md" } -- 全てのmdファイルをプロジェクト直下に表示
    }
group ""