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
	
	--pchheader "hzpch.h"
	--pchsource "src/hzpch.cpp"
	
	files {
		"src/**.h",
		"src/**.cpp",
		"src/**.hpp",
		"src/**.inl",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm.hpp",
		"vendor/glm/glm.inl",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",
		
		"vendor/tinyobj/tiny_obj_loader.h",
		"vendor/filewatch/FileWatch.hpp",
        "vendor/wren/src/vm/*.c",
		"vendor/wren/src/vm/*.h",
	}
	
	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"IMGUI_DEFINE_MATH_OPERATORS",
		"GLFW_INCLUDE_NONE"
	}
	
	includedirs {
		"src",
		"src/Hazel",
		"vendor/filewatch",
		"vendor/stb_image/**.h",
		"vendor/nlohmann_json/include",
		-- "vendor/sol.hpp",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.PhysX}",
		-- "%{IncludeDir.Jolt}",
		-- "%{IncludeDir.bullet}",
		"%{IncludeDir.TinyOBJ}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.gli}",
		"%{IncludeDir.Assimp}",
		"%{IncludeDir.Coral}",
		"%{IncludeDir.Wren}",
		"%{IncludeDir.Lua}",
		"%{IncludeDir.Soloud}",
		"%{IncludeDir.FreeType}",
		"%{IncludeDir.msdfgen}",
		"%{IncludeDir.msdf_atlas_gen}",
	}
	
	links {
		"GLFW",
		"%{wks.location}/Faint/vendor/assimp/lib/assimp-vc143-mtd.lib",
		"glad",
		"ImGui",
		"yaml-cpp",
		"PhysX",
		-- "JoltPhysics",
		"Coral.Native",
		"soloud",
		"%{wks.location}/Faint/vendor/freetype/freetype.lib",
		"%{wks.location}/Faint/vendor/lua/lib/lua-5.4.4.lib",
		"msdfgen",
		"msdf-atlas-gen",
		"opengl32.lib",
		
		-- "%{wks.location}/Faint/vendor/bullet/lib/%{cfg.buildcfg}/Bullet3Collision.lib",
		-- "%{wks.location}/Faint/vendor/bullet/lib/%{cfg.buildcfg}/Bullet3Common.lib",
		-- "%{wks.location}/Faint/vendor/bullet/lib/%{cfg.buildcfg}/Bullet3Dynamics.lib",
		-- "%{wks.location}/Faint/vendor/bullet/lib/%{cfg.buildcfg}/Bullet3Geometry.lib",
		-- "%{wks.location}/Faint/vendor/bullet/lib/%{cfg.buildcfg}/BulletCollision.lib",
		-- "%{wks.location}/Faint/vendor/bullet/lib/%{cfg.buildcfg}/BulletDynamics.lib",
		-- "%{wks.location}/Faint/vendor/bullet/lib/%{cfg.buildcfg}/BulletSoftBody.lib",
		-- "%{wks.location}/Faint/vendor/bullet/lib/%{cfg.buildcfg}/LinearMath.lib",
	}
	
	filter "files:vendor/ImGuizmo/**.cpp"
	flags { "NoPCH" }
	
	filter "system:windows"
		systemversion "latest"
		defines { "FT_PLATFORM_WINDOWS" }
		
	filter "configurations:Debug"
		defines "FT_DEBUG"
		buildoptions "/MTd"
		symbols "on"
		
		buildoptions {"/Zi"}
		
	filter "configurations:Release"
		defines "FT_RELEASE"
		buildoptions "/MTd"
		symbols "on"
		
	filter "configurations:Dist"
		defines "FT_DIST"
		buildoptions "/MD"
		symbols "on"