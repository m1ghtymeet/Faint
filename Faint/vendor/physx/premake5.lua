project "PhysX"
	kind "StaticLib"
	language "C++"
	staticruntime "On"
	warnings 'Off'
	optimize 'Speed'
	
	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
	
	files {
		"include/**.h",
		"src/**.h",
		"src/**.cpp"
	}
	
	includedirs {
		"include"
	}
	
	links {
		"lib/%{cfg.buildcfg}/PhysX_64",
		"lib/%{cfg.buildcfg}/PhysXCharacterKinematic_static_64",
		"lib/%{cfg.buildcfg}/PhysXCommon_64",
		"lib/%{cfg.buildcfg}/PhysXCooking_64",
		"lib/%{cfg.buildcfg}/PhysXExtensions_static_64",
		"lib/%{cfg.buildcfg}/PhysXFoundation_64",
		"lib/%{cfg.buildcfg}/PhysXPvdSDK_static_64",
		"lib/%{cfg.buildcfg}/PhysXTask_static_64",
		"lib/%{cfg.buildcfg}/PVDRuntime_64",
	}
	
	filter "system:windows"
		systemversion "latest"
		
	filter "configurations:Debug"
		buildoptions "/MT"
		cppdialect "C++17"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		buildoptions "/MT"
		cppdialect "C++17"
		runtime "Release"
		optimize "on"