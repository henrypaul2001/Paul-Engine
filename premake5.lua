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
IncludeDir["glm"] = "PaulEngine/vendor/glm"
IncludeDir["stb_image"] = "PaulEngine/vendor/stb_image"
IncludeDir["entt"] = "PaulEngine/vendor/entt/include"
IncludeDir["yaml"] = "PaulEngine/vendor/yaml/yaml-cpp"
IncludeDir["imguizmo"] = "PaulEngine/vendor/imguizmo"

group "Dependencies"
	include "PaulEngine/vendor/GLFW"
	include "PaulEngine/vendor/glad"
	include "PaulEngine/vendor/imgui"
	include "PaulEngine/vendor/yaml"
group ""

project "PaulEngine"
	location "PaulEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pepch.h"
	pchsource "PaulEngine/src/pepch.cpp"

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"YAML_CPP_STATIC_DEFINE"
	}

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl",
		"%{prj.name}/vendor/imguizmo/ImGuizmo.h",
		"%{prj.name}/vendor/imguizmo/ImGuizmo.cpp"
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml}/include",
		"%{IncludeDir.imguizmo}"
	}

	links
	{
		"GLFW",
		"glad",
		"ImGui",
		"opengl32.lib",
		"yaml-cpp"
	}

	filter "files:PaulEngine/vendor/imguizmo/**.cpp"
	flags { "NoPCH"}


	filter "system:windows"
		systemversion "latest"

		buildoptions "/utf-8"

		defines
		{
			"PE_PLATFORM_WINDOWS",
			"PE_BUILD_DLL",
			"GLFW_INCLUDE_NONE",
		}

	filter "configurations:Debug"
		defines 
		{
			"PE_DEBUG",
			"PE_ENABLE_ASSERTS"
		}
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PE_DIST"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"PaulEngine/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml}"
	}

	links
	{
		"PaulEngine"
	}

	defines
	{
		"YAML_CPP_STATIC_DEFINE"
	}

	filter "system:windows"
		systemversion "latest"

		buildoptions "/utf-8"

		defines
		{
			"PE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "PE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PE_DIST"
		runtime "Release"
		optimize "on"


project "PE-Editor"
	location "PE-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"PaulEngine/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.entt}",
		"{IncludeDir.yaml}"
	}

	links
	{
		"PaulEngine"
	}

	defines
	{
		"YAML_CPP_STATIC_DEFINE"
	}

	filter "system:windows"
		systemversion "latest"

		buildoptions "/utf-8"

		defines
		{
			"PE_PLATFORM_WINDOWS"
		}

	filter "configurations:Debug"
		defines "PE_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "PE_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "PE_DIST"
		runtime "Release"
		optimize "on"
