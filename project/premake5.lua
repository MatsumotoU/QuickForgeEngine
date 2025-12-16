-- Solution
workspace "QuickForgeEngine"
    
    architecture "x64"
    configurations {"Debug","Development","Release"}
    startproject "Editor"

    cppdialect "C++20"

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
            "/utf-8"
        }
    filter ""
    
group "QuickForge" -- MyMainProject
    project "Editor"
        location "editor"
        kind "ConsoleApp"
        language "C++"
        files {"editor/**.h","editor/**.cpp"}
        links{
            "Engine",
            "ExternalFolders"
        }
    
        -- 追加のインクルード
        includedirs{
            "$(SolutionDir)/",
            "$(SolutionDir)/engine/include/",
            "$(SolutionDir)/externals/",
            "$(SolutionDir)/externals/nlohmann/",
            "$(SolutionDir)/externals/lua/",
            "$(SolutionDir)/externals/sol2/",
        }

    project "Engine"
        location "engine"
        kind "StaticLib" 
        language "C++"
        files {"engine/**.h","engine/**.cpp"}
        links{
            "DirectXTex",
            "ExternalFolders"
        }

        -- 追加のインクルード
        includedirs{
            "$(SolutionDir)/",
            "$(SolutionDir)/externals/",
            "$(SolutionDir)/externals/assimp/",
            "$(SolutionDir)/externals/assimp/include/",
            "$(SolutionDir)/externals/DirectXTex/",
            "$(SolutionDir)/externals/imgui/",
            "$(SolutionDir)/externals/lua/",
            "$(SolutionDir)/externals/Mono/",
            "$(SolutionDir)/externals/Mono/include",
            "$(SolutionDir)/externals/Mono/include/mono-2.0/",
            "$(SolutionDir)/externals/nlohmann/",
            "$(SolutionDir)/externals/sol2",
        }
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
    
    configurations { "Debug","Development","Release" }
    platforms { "x64" }

    filter "system:windows"
        systemversion "latest"
        buildoptions {
            "/permissive-",
            "/openmp"
        }
    filter ""
    
    includedirs {
        ".", 
        "Shaders/Compiled"
    }

    files {
        "BC.h", "BC.cpp", "BC4BC5.cpp", 
        "BC6HBC7.cpp", "BCDirectCompute.h", "d3dx12.h",
        "DDS.h", "filters.h", "scoped.h", "DirectXTex.h", 
        "DirectXTexP.h", "DirectXTex.inl", "BCDirectCompute.cpp", 
        "DirectXTexCompress.cpp", "DirectXTexCompressGPU.cpp", 
        "DirectXTexConvert.cpp", "DirectXTexD3D11.cpp", 
        "DirectXTexD3D12.cpp", "DirectXTexDDS.cpp", "DirectXTexFlipRotate.cpp", 
        "DirectXTexHDR.cpp", "DirectXTexImage.cpp", "DirectXTexMipMaps.cpp", 
        "DirectXTexMisc.cpp", "DirectXTexNormalMaps.cpp", 
        "DirectXTexPMAlpha.cpp", "DirectXTexResize.cpp", 
        "DirectXTexTGA.cpp", "DirectXTexUtil.cpp", 
        "DirectXTexWIC.cpp"
    }

    filter "platforms:x64"
        targetdir "../generated/outputs/%{cfg.platform}/%{cfg.buildcfg}"
        objdir "../generated/obj/DirectXTex/%{cfg.platform}/%{cfg.buildcfg}"
    filter ""
    
    filter "configurations:Debug"
        defines { "_DEBUG" }
        optimize "Off"
        runtime "Debug"
        symbols "On"
    filter "configurations:Release or configurations:Profile or configurations:Development"
        defines { "NDEBUG" }
        optimize "On"
        runtime "Release"
        symbols "Off"
    filter ""

group ""