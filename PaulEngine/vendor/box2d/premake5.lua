project "Box2D"
	kind "StaticLib"
	language "C"
	cdialect "C17"
	staticruntime "off"

	targetdir ("box2d/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("box2d/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"box2d/src/**.h",
		"box2d/src/**.c",
		"box2d/include/**.h"
	}

	includedirs
	{
		"box2d/include",
		"box2d/src"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		runtime "Release"
		optimize "on"