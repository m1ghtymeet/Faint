project "FaintNet"
    language "C#"
    dotnetframework "net8.0"
    kind "SharedLib"
	clr "Unsafe"
	
    -- Don't specify architecture here. (see https://github.com/premake/premake-core/issues/1758)

    propertytags {
        { "AppendTargetFrameworkToOutputPath", "false" },
        { "Nullable", "enable" },
    }

    files 
    {
        "src/**.cs"
    }
    
	links 
    {
        "Coral.Managed"
    }

    postbuildcommands {
        '{ECHO} Copying "%{wks.location}/FaintNet/bin/$(Configuration)/FaintNet.dll" to "%{wks.location}/Hazelnut"',
		'{COPYFILE} "%{wks.location}/FaintNet/bin/$(Configuration)/FaintNet.dll" "%{wks.location}/Hazelnut"'
    }
	