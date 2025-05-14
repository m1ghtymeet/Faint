include "Dependencies.lua"

workspace "Faint"
	conformancemode "On"
	startproject "Faintnut"
	flags { "MultiProcessorCompile" }
	configurations { "Debug", "Release", "Dist" }
	filter { "language:C++" }
		architecture "x86_64"
	filter { "language:C" }
		architecture "x64"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
builddir = "%{wks.location}Faintnut/builder/"

-- Include directories relative to root folder (solution directory)

group "Dependencies"
	include "Faint/vendor/GLFW"
	include "Faint/vendor/glad"
	include "Faint/vendor/imgui"
	include "Faint/vendor/yaml-cpp"
	include "Faint/vendor/PhysX"
	-- include "Faint/vendor/jolt_premake.lua"
	include "Faint/vendor/coral_premake.lua"
	include "Faint/vendor/soloud_premake.lua"
	include "Faint/vendor/freetype_premake.lua"
	include "Faint/vendor/msdf-atlas-gen_premake.lua"
group ""

group "Core"
	include "Faint"
group ""

group "Tools"
	include "Faintnut"
group ""

include "FaintNet"
		
group "Tools"
project "FaintGame"
	location "FaintGame"
	language "C++"
	cppdialect "C++17"
	
	targetdir ("bin/".. outputdir .."/%{prj.name}")
	objdir ("bin-int/".. outputdir .."/%{prj.name}")
	
	files {
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}
	
	includedirs {
		"Faint/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.yaml_cpp}",
		"%{wks.location}/Faint/vendor/nlohmann_json/include",
		"%{IncludeDir.Jolt}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.Coral}",
		"%{IncludeDir.Wren}",
		"%{IncludeDir.Soloud}",
		"%{IncludeDir.FreeType}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.msdf_atlas_gen}",
	}
	
	links { "ImGui", "Faint", "Coral.Native",
	"%{wks.location}/Faint/vendor/freetype/freetype.lib", "msdfgen", "msdf-atlas-gen" }
	characterset ("MBCS")
	
	filter "system:windows"
		defines { "FT_PLATFORM_WINDOWS" }
		fastuptodate "Off"
		
	filter "configurations:Debug"
		defines "FT_DEBUG"
		symbols "on"
		kind "ConsoleApp"
		
	filter "configurations:Release"
		defines "FT_RELEASE"
		symbols "on"
		kind "WindowedApp"
		
	filter "configurations:Dist"
		defines "FT_DIST"
		symbols "on"
		kind "ConsoleApp"
group ""
	