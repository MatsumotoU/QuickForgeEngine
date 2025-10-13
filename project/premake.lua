-- common project settings function
function setup_common_project_settings()
	kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetdir "bin/%{cfg.buildcfg}"
    files { "src/**.h", "src/**.cpp" }
    includedirs { "src", "vendor/include" }
    libdirs { "vendor/lib" }
    links { "SomeLibrary" }
    filter "configurations:Debug"
        defines { "DEBUG" }
        symbols "On"
    filter "configurations:Development"
        defines { "DEVELOPMENT" }
        optimize "Release"
        symbols "On"
    filter "configurations:Release"
        defines { "NDEBUG" }
        optimize "On"
    filter {} -- ÉtÉBÉãÉ^âèú
end

-- Basic Premake5 script for a C++ project
workspace "MyProject"
	configurations { "Debug", "Development","Release" }
	architecture "x64"

project "MyProject"
	setup_common_project_settings()

project "DirectXTex"
    setup_common_project_settings()