project "Moonnut"
	language "C++"
	
	dependson { "MoonNet" } 
	
	targetdir ("%{wks.location}/bin/".. outputdir .."/%{prj.name}")
	objdir ("%{wks.location}/bin-int/".. outputdir .."/%{prj.name}")
	
	files {
		"src/**.h",
		"src/**.cpp",
	}
	
	includedirs {
		"%{wks.location}/Faint/src",
		"%{wks.location}/Faint/vendor",
		"%{wks.location}/Faint/vendor/zstd/include",
		"%{wks.location}/Faint/vendor/nlohmann_json/include",
		"%{wks.location}/Faint/vendor/msdf-atlas-gen",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.Coral}",
		"%{IncludeDir.Lua}",
		"%{IncludeDir.Soloud}",
		"%{IncludeDir.FreeType}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.msdf_atlas_gen}",
	}
	
	links {
		"ImGui",
		"Faint",
		"PhysX",
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
		--externalincludedirs { "%{prj.name}/../Faint/vendor/Coral/Coral.Native/Include/" }
		postbuildcommands {
			--'{COPYFILE} "%{wks.location}/bin/Debug-windows-x86_64/FaintRuntime/FNGame.exe" "%{wks.location}/%{prj.name}/Builder/Development/"',
			--"xcopy \"bin\\Debug-windows-x86_64\\FaintGame\\FaintGame.exe\" \"bin\\Debug-windows-x86_64\\Faintnut\builder\\development\\\" /y /i /c",
			--"xcopy \"bin\\Debug-windows-x86_64\\FaintGame\\FaintGame.exe\" \"Faintnut\\builder\\development\\\" /y /i /c",
			--'{COPYFILE} "%{wks.location}/Faint/vendor/Coral/Coral.Managed/bin/%{cfg.buildcfg}/Coral.Managed.dll" "%{wks.location}/%{prj.name}"',
			--'{COPYFILE} "%{wks.location}/Faint/vendor/Coral/Coral.Managed/Coral.Managed.runtimeconfig.json" "%{wks.location}/%{prj.name}"',
		}
		--debugenvs { "PATH=%PATH%;" }
		
	filter "configurations:Debug"
		defines { "FT_DEBUG", "_DEBUG" }
		runtime "Debug"
		symbols "on"
		kind "ConsoleApp"
		
	filter "configurations:Release"
		defines { "FT_RELEASE", "_DEBUG" }
		runtime "Release"
		symbols "on"
		kind "WindowedApp"
		
	filter "configurations:Dist"
		defines "HZ_DIST"
		symbols "on"
		kind "ConsoleApp"