project "Faint"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "On"
	
	local moduleSources = {}
	if _ACTION then
		local modulesDir = "Hazel/src/Modules"
		local outputFilePath = path.join(modulesDir, "Modules.cpp")
		
		-- Load and generate the modules file
		--local modules = loadModules(modulesDir)
		--modulesSources = generateModulesFile(modules, outputFilePath, "Hazel/src/Modules")
	end
	
	targetdir ("%{wks.location}/bin/".. outputdir .."/%{prj.name}")
	objdir ("%{wks.location}/bin-int/".. outputdir .."/%{prj.name}")
	
	files {
		"src/**.h",
		"src/**.c",
		"src/**.cpp",
		"src/**.hpp",
		"src/**.inl",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm.hpp",
		"vendor/glm/glm.inl",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
		"vendor/msdfgen/**.cpp",
		"vendor/msdfgen/core/**.cpp",
		"vendor/msdf-atlas-gen/**.cpp",
		
		"vendor/tinygltf/tinygltf.h"
	}
	
	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"IMGUI_DEFINE_MATH_OPERATORS",
		"GLFW_INCLUDE_NONE"
	}
	
	includedirs {
		"src",
		"vendor/zstd/include",
		"vendor/utf8",
		"vendor/stb_image/**.h",
		"vendor/msdf-atlas-gen",
		"vendor/nlohmann_json/include",
		"vendor/tiny_gtlf.h",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.PhysX}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.gli}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.FreeType}",
		"%{IncludeDir.Coral}",
		"%{IncludeDir.Lua}",
		"%{IncludeDir.Soloud}",
		"%{wks.location}/Faint/vendor/compressonator/include",
		"%{wks.location}/Faint/vendor/msdfgen"
	}
	
	links {
		"GLFW",
		"%{wks.location}/Faint/vendor/assimp/lib/assimp-vc143-mtd.lib",
		"glad",
		"ImGui",
		"yaml-cpp",
		"PhysX",
		"Coral.Native",
		"soloud",
		"%{wks.location}/Faint/vendor/lua/lib/lua-5.4.4.lib",
		"%{wks.location}/Faint/vendor/compressonator/lib/CMP_Framework_MTd.lib",
		"%{wks.location}/Faint/vendor/compressonator/lib/Compressonator_MTd.lib",
		"%{wks.location}/Faint/vendor/zstd/lib/libzstd.lib",
		"opengl32.lib"
	}
	
	filter "files:vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }
	
	filter "system:windows"
		systemversion "latest"
		defines "FT_PLATFORM_WINDOWS"
		
	filter "configurations:Debug"
		defines { "FT_DEBUG", "_DEBUG" }
		runtime "Debug"
		symbols "on"
		
	filter "configurations:Release"
		defines { "FT_RELEASE", "_DEBUG" }
		runtime "Release"
		symbols "on"
		
	filter "configurations:Dist"
		defines "FT_DIST"
		runtime "Release"
		symbols "on"