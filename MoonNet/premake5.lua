project "MoonNet"
	language "C#"
	dotnetframework "net8.0"
	kind "SharedLib"
	clr "Unsafe"
	
	targetdir ("%{wks.location}/bin/".. outputdir .."/%{prj.name}")
	objdir ("%{wks.location}/bin-int/".. outputdir .."/%{prj.name}")
	
	--vsprops {
    --   AppendTargetFrameworkToOutputPath =  "false",
    --   Nullable = "enable",
    --   CopyLocalLockFileAssemblies = "true",
    --   EnableDynamicLoading = "true"
    --}
	
	files {
		"src/**.cs"
	}
	
	links {
		"Coral.Managed"
	}
	
	