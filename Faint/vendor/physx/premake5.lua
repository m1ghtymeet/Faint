project "PhysX"
	kind "StaticLib"
	language "C++"
	staticruntime "On"
	warnings 'Off'
	
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
	
	filter "system:windows"
		systemversion "latest"
		
	filter "configurations:Debug"
		cppdialect "C++17"
		runtime "Debug"
		symbols "on"
		
		links {
		"lib/Debug/PhysX_64",
		"lib/Debug/PhysXCharacterKinematic_static_64",
		"lib/Debug/PhysXCommon_64",
		"lib/Debug/PhysXCooking_64",
		"lib/Debug/PhysXExtensions_static_64",
		"lib/Debug/PhysXFoundation_64",
		"lib/Debug/PhysXPvdSDK_static_64",
		"lib/Debug/PhysXTask_static_64",
		"lib/Debug/PVDRuntime_64",
		}

	filter "configurations:Release"
		cppdialect "C++17"
		runtime "Release"
		optimize "on"
		
		links {
		"lib/Release/PhysX_64",
		"lib/Release/PhysXCharacterKinematic_static_64",
		"lib/Release/PhysXCommon_64",
		"lib/Release/PhysXCooking_64",
		"lib/Release/PhysXExtensions_static_64",
		"lib/Release/PhysXFoundation_64",
		"lib/Release/PhysXPvdSDK_static_64",
		"lib/Release/PhysXTask_static_64",
		"lib/Release/PVDRuntime_64",
		}