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
            "/openmp",
            "/wd5321"
        }
    filter ""

    filter "configurations:Debug"
        defines { "_DEBUG" }
        optimize "Off"
        symbols "On"
        runtime "Debug"
    filter "configurations:Release or configurations:Development"
        defines { "NDEBUG" }
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
            "Lua",
            "ExternalFolders"
        }

        -- 警告レベル4
        warnings "Extra"
        fatalwarnings "All" --すべての警告をエラーとします

        -- 追加のインクルード
        includedirs{
            "./",
            "./engine/include/",
            "./externals/",
            "./externals/assimp/",
            "./externals/assimp/include/",
            "./externals/DirectXTex/",
            "./externals/imgui/",
            "./externals/lua/",
            "./externals/sol2",
            "./externals/nlohmann/",
        }

        prebuildcommands {
            'call "%{wks.location}/GenerateBuildInfo.bat"'
        }

        postbuildcommands {
            'robocopy "$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64" "%{cfg.targetdir}" "dxcompiler.dll" "dxil.dll" /XO /R:0 /W:0 /NJH /NJS > nul',
            "exit /b 0"
        }

        filter "configurations:Debug"
            libdirs { "externals/lua/bin/Debug" }
        filter "configurations:Release or configurations:Development"
            libdirs { "externals/lua/bin/Release" }
        filter ""

    project "Engine" -- Engine
        location "engine"
        kind "StaticLib"
        language "C++"
        files {"engine/**.h","engine/**.cpp"}
        removefiles {
            "engine/**/Csharp*.h",
            "engine/**/Csharp*.cpp"
        }
        links{
            "DirectXTex",
            "ImGui",
            "Lua",
            "ExternalFolders"
        }
        -- 警告レベル4
        warnings "Extra"

        -- 追加のインクルード
        includedirs{
            "./",
            "./engine/include/",
            "./externals/",
            "./externals/assimp/",
            "./externals/assimp/include/",
            "./externals/DirectXTex/",
            "./externals/imgui/",
            "./externals/lua/",
            "./externals/sol2",
            "./externals/nlohmann/",
        }

        filter "configurations:Debug"
            links { "assimp-vc143-mtd" }
            libdirs {
                "externals/lua/bin/Debug",
                "externals/assimp/lib/Debug"
            }
        filter "configurations:Release or configurations:Development"
            links { "assimp-vc143-mt" }
            libdirs {
                "externals/lua/bin/Release",
                "externals/assimp/lib/Release"
            }
        filter ""
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
    removefiles { "externals/Mono/**" }

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
        "externals/DirectXTex/**.h",
        "externals/DirectXTex/**.cpp",
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

project "Lua"
    location "externals/lua"
    kind "StaticLib"
    language "C"
    files {
        "externals/lua/**.h",
        "externals/lua/**.c"
    }
    removefiles {
        "externals/lua/lua.c",
        "externals/lua/luac.c"
    }
    filter "system:windows"
        defines { "_CRT_SECURE_NO_WARNINGS" }
    filter ""
