
project "msdfgen"
	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'

	targetdir ("msdfgen/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("msdfgen/bin-obj/" .. outputdir .. "/%{prj.name}")

	includedirs {
		"msdf-atlas-gen/include",
		"freetype/include"
	}

	files {
		'msdfgen/*.h',
		'msdfgen/*.cpp',
		'msdfgen/*.hpp',
		'msdfgen/Core/*.h',
		'msdfgen/Core/*.hpp',
		'msdfgen/Core/*.cpp',
		'msdfgen/lib/*.cpp',
		'msdfgen/ext/*.h',
		'msdfgen/ext/*.cpp'
	}

	filter "system:windows"
		
		flags {
			"MultiProcessorCompile"
		}
		
	filter "configurations:Debug"
		cppdialect "C++17"
		buildoptions "/MTd"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		cppdialect "C++17"
		buildoptions "/MTd"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		cppdialect "C++17"
		runtime "Release"
		optimize "on"

project "msdf-atlas-gen"
	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'
	cppdialect "C++17"

	targetdir ("msdf-atlas-gen/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("msdf-atlas-gen/bin-obj/" .. outputdir .. "/%{prj.name}")

	includedirs {
		"msdf-atlas-gen/msdf-atlas-gen",
		"msdfgen",
		--"msdfgen/include"
	}

	files {
		"msdf-atlas-gen/msdf-atlas-gen/**.h",
		"msdf-atlas-gen/msdf-atlas-gen/**.hpp",
		"msdf-atlas-gen/msdf-atlas-gen/**.cpp",
	}

	links {
		'msdfgen'
	}
		
	filter "configurations:Debug"
		buildoptions "/MTd"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		buildoptions "/MTd"
		runtime "Release"
		optimize "on"
