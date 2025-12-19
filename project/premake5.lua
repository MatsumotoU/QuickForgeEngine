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
            "./externals/Mono/",
            "./externals/Mono/include",
            "./externals/Mono/include/mono-2.0/",
            "./externals/nlohmann/",
        }

        prebuildcommands {
            'call "..\\GenerateBuildInfo.bat"'
        }

        postbuildcommands {
            -- Copy runtime dependencies
            'copy "$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxcompiler.dll" "%{cfg.targetdir}"',
            'copy "$(WindowsSdkDir)bin\\$(TargetPlatformVersion)\\x64\\dxil.dll" "%{cfg.targetdir}"',
            'copy "..\\externals\\Mono\\bin\\mono-2.0-sgen.dll" "%{cfg.targetdir}"',
            
            -- Copy runtime-required folders
            -- 'xcopy "..\\Resources" "%{cfg.targetdir}Resources\\" /S /I /Y',
            "mkdir \"%{cfg.targetdir}\\mono\"",
            "xcopy \"..\\externals\\Mono\\lib\" \"%{cfg.targetdir}\\mono\\lib\\\" /S /I /Y",
            "xcopy \"..\\externals\\Mono\\etc\" \"%{cfg.targetdir}\\mono\\etc\\\" /S /I /Y"
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
        links{
            "DirectXTex",
            "ImGui",
            "Lua",
            "mono-2.0-sgen",
            "ExternalFolders"
        }
        libdirs {
            "externals/Mono/lib"
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
            "./externals/Mono/",
            "./externals/Mono/include",
            "./externals/Mono/include/mono-2.0/",
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


