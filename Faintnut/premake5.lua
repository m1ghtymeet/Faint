project "Faintnut"
	language "C++"
	
	dependson { "FaintNet" } 
	
	targetdir ("%{wks.location}/bin/".. outputdir .."/%{prj.name}")
	objdir ("%{wks.location}/bin-int/".. outputdir .."/%{prj.name}")
	
	files {
		"src/**.h",
		"src/**.cpp",
	}
	
	includedirs {
		-- "%{wks.location}/Faint/vendor/spdlog/include/",
		"%{wks.location}/Faint/src",
		"%{wks.location}/Faint/vendor",
		"%{wks.location}/Faint/vendor/filewatch/",
		"%{wks.location}/Faint/vendor/nlohmann_json/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.PhysX}",
		-- "%{IncludeDir.Jolt}",
		"%{IncludeDir.Coral}",
		"%{IncludeDir.Wren}",
		"%{IncludeDir.Lua}",
		"%{IncludeDir.Soloud}",
		"%{IncludeDir.FreeType}",
		-- "%{IncludeDir.msdfgen}",
		-- "%{IncludeDir.msdf_atlas_gen}",
	}
	
	links {
		"ImGui",
		"Faint",
		"PhysX",
		-- "JoltPhysics",
		"%{wks.location}/Faint/vendor/assimp/lib/assimp-vc143-mtd.lib",
		"%{wks.location}/Faint/vendor/freetype/freetype.lib",
		"%{wks.location}/Faint/vendor/lua/lib/lua-5.4.4.lib",
		-- "msdfgen",
		-- "msdf-atlas-gen",
		"Coral.Native",
	}
	
	filter "system:windows"
		cppdialect "C++20"
		systemversion "latest"
		staticruntime "On"
		defines { "FT_PLATFORM_WINDOWS", "IMGUI_DEFINE_MATH_OPERATORS" }
		externalincludedirs { "%{prj.name}/../Faint/vendor/Coral/Coral.Native/Include/" }
		postbuildcommands {
			--'{COPYFILE} "%{wks.location}/bin/Debug-windows-x86_64/FaintRuntime/FNGame.exe" "%{wks.location}/%{prj.name}/Builder/Development/"',
			"xcopy \"bin\\"..outputdir.."\\FaintRuntime\\FaintRuntime.exe\" \"%{builddir}%{cfg.buildcfg}\\development\" /y /i /c",
			'{COPYFILE} "%{wks.location}/Faint/vendor/Coral/Coral.Managed/bin/%{cfg.buildcfg}/Coral.Managed.dll" "%{wks.location}/%{prj.name}"',
			'{COPYFILE} "%{wks.location}/Faint/vendor/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}/%{prj.name}"',
		}
		
	filter "configurations:Debug"
		defines "FT_DEBUG"
		runtime "Debug"
		buildoptions "/MTd"
		symbols "on"
		kind "ConsoleApp"
		
	filter "configurations:Release"
		defines "FT_RELEASE"
		runtime "Release"
		buildoptions "/MTd"
		symbols "on"
		kind "WindowedApp"
		
	filter "configurations:Dist"
		defines "HZ_DIST"
		symbols "on"
		kind "ConsoleApp"