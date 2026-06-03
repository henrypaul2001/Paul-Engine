project "zlib"
	kind "StaticLib"
	language "C"
	staticruntime "off"

	targetdir ("zlib/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("zlib/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"zlib/crc32.h",
		"zlib/deflate.h",
		"zlib/gzguts.h",
		"zlib/inffast.h",
		"zlib/inffixed.h",
		"zlib/inflate.h",
		"zlib/inftrees.h",
		"zlib/trees.h",
		"zlib/zutil.h",

		"zlib/adler32.c",
		"zlib/compress.c",
		"zlib/crc32.c",
		"zlib/deflate.c",
		"zlib/gzclose.c",
		"zlib/gzlib.c",
		"zlib/gzread.c",
		"zlib/gzwrite.c",
		"zlib/inflate.c",
		"zlib/infback.c",
		"zlib/inftrees.c",
		"zlib/inffast.c",
		"zlib/trees.c",
		"zlib/uncompr.c",
		"zlib/zutil.c",

		"zlib/zconf.h",
		"zlib/zlib.h"
	}

	includedirs
	{
		"zlib"
	}

	defines
	{
		"_CRT_SECURE_NO_DEPRECATE",
		"_CRT_NONSTDC_NO_DEPRECATE"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"