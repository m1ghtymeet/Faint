
project 'Soloud'
	location "soloud"
	kind "StaticLib"
	staticruntime "on"
	warnings 'Off'
	optimize 'Speed'

	targetdir ("soloud/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("soloud/bin-obj/" .. outputdir .. "/%{prj.name}")

	includedirs {
		"soloud/include"
	}

	files {
		"soloud/src/**.h",
		"soloud/src/core/**.cpp",
		"soloud/src/audiosource/**.c*",
	}

	filter "system:windows"
		
		defines {
			"WITH_WASAPI"
		}
		
		files {
			"soloud/src/backend/wasapi/**.c*"
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
