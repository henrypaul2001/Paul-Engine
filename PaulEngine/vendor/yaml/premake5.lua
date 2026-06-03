project "yaml-cpp"
	kind "StaticLib"
	language "C++"
	staticruntime "off"

	targetdir ("yaml-cpp/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("yaml-cpp/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"yaml-cpp/src/**.h",
		"yaml-cpp/src/**.cpp",
		
		"yaml-cpp/include/**.h"
	}

	includedirs
	{
		"yaml-cpp/include"
	}

	defines
	{
		"YAML_CPP_STATIC_DEFINE"
	}

	filter "system:windows"
		systemversion "latest"
		cppdialect "C++17"

	filter "system:linux"
		pic "On"
		systemversion "latest"
		cppdialect "C++17"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"
