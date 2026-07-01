-- include: サブフォルダの premake を workspace スコープ内で読み込む
local _root = path.getdirectory(_SCRIPT)

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
        }

       if not os.getenv("SONAR_SCAN") then
            buildoptions { "/openmp" }
        end

        linkoptions { "/ignore:4099" }
    filter ""
    
    filter "configurations:Debug"
        defines { "_DEBUG", "QFE_OPTIMIZE_OFF","QFE_MODE_DEBUG","USE_IMGUI"}
        optimize "Off"
        symbols "On"
        runtime "Debug"
    filter "configurations:Development"
        defines { "NDEBUG", "QFE_OPTIMIZE_OFF","QFE_MODE_DEVELOPMENT","USE_IMGUI"} 
        optimize "Off"  -- プロジェクト全体設定は無効（VS上で最適化「無効」に見える）
        symbols "On"     -- デバッグ情報を出す
        runtime "Release"
    filter "configurations:Release"
        defines { "NDEBUG", "QFE_OPTIMIZE_ON","QFE_MODE_RELEASE","NO_IMGUI" }
        optimize "On"
        symbols "Off"
        runtime "Release"
    filter ""

    -- 個別のファイルを対象に最適化を有効化（Development構成のみ）
    filter { "configurations:Development", "files:**.cpp" }
        optimize "On"
    filter ""

group "00_System" -- すべての土台のプロジェクト
    dofile(path.join(_root, "engine/resources/premake5.lua"))
    -- 異存なしのコアプロジェクトの読み込み
    dofile(path.join(_root, "engine/core/premake5.lua"))
group ""

group "01_SubSystems" -- 独立した機能を提供するプロジェクト達
    -- ウィンドウシステムプロジェクトの読み込み
    dofile(path.join(_root, "engine/window/premake5.lua"))
    -- グラフィックエンジンプロジェクトの読み込み
    dofile(path.join(_root, "engine/graphics/premake5.lua"))
    -- GUIプロジェクトの読み込み
    dofile(path.join(_root, "engine/gui/premake5.lua"))
    
group ""

group "02_Middleware" -- アセットの読み込みや、サブシステムで使うデータを提供するプロジェクト達
    -- アセットファクトリープロジェクトの読み込み
    dofile(path.join(_root, "engine/assetfactory/premake5.lua"))
    -- カメラプロジェクトの読み込み
    dofile(path.join(_root, "engine/camera/premake5.lua"))
group ""

group "03_Applications" -- アプリケーションプロジェクト達
    -- エディタープロジェクトの読み込み
    dofile(path.join(_root, "editor/premake5.lua"))
    -- サンドボックスプロジェクトの読み込み
    dofile(path.join(_root, "sandbox/premake5.lua"))
group ""

group "External"
project "ExternalFolders"
    location "externals"
    kind "None"
    files {
        "externals/**"
    }

-- ImGui
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

-- DirectXTex
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
