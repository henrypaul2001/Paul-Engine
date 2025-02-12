workspace "PaulEngine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	startproject "Sandbox"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

IncludeDir = {}
IncludeDir["GLFW"] = "PaulEngine/vendor/GLFW/GLFW/include"
IncludeDir["glad"] = "PaulEngine/vendor/glad/include"
IncludeDir["ImGui"] = "PaulEngine/vendor/imgui/imgui"

include "PaulEngine/vendor/GLFW"
include "PaulEngine/vendor/glad"
include "PaulEngine/vendor/imgui"

project "PaulEngine"
	location "PaulEngine"
	kind "SharedLib"
	language "C++"
	staticruntime "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pepch.h"
	pchsource "PaulEngine/src/pepch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.ImGui}"
	}

	links
	{
		"GLFW",
		"glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		buildoptions "/utf-8"

		defines
		{
			"PE_PLATFORM_WINDOWS",
			"PE_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

		postbuildcommands
		{
			("{COPY} %{cfg.buildtarget.relpath} ../bin/" .. outputdir .. "/Sandbox")
		}

	filter "configurations:Debug"
		defines 
		{
			"PE_DEBUG",
			"PE_ENABLE_ASSERTS"
		}
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "PE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "PE_DIST"
		runtime "Release"
		optimize "On"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	staticruntime "Off"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}

	includedirs
	{
		"PaulEngine/vendor/spdlog/include",
		"PaulEngine/src"
	}

	links
	{
		"PaulEngine"
	}

	filter "system:windows"
		cppdialect "C++17"
		systemversion "latest"

		buildoptions "/utf-8"

		defines
		{
			"PE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "PE_DEBUG"
		runtime "Debug"
		symbols "On"

	filter "configurations:Release"
		defines "PE_RELEASE"
		runtime "Release"
		optimize "On"

	filter "configurations:Dist"
		defines "PE_DIST"
		runtime "Release"
		optimize "On"
