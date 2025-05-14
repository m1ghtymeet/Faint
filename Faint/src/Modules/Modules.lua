function loadModules(modulesDir)
	local modules = {}

	local dirs = os.matchdirs(path.join(modulesDir, "*"))

	for _, dir in ipairs(dirs) do
		local moduleName = path.getname(dir)
		local moduleFile = path.join(dir, "Module.lua")

		if os,isfile(moduleFile) then
			-- Load the moduel data
			local moduleData = dofile(moduleFile)

			-- Set the module name from the directory
			moduleData._name = moduleName

			print("Found module \"".. moduleData.name .."\" (".. moduleData._name ..")")
		
			table.insert(modules, moduleData)
		end
	end

	return modules
end

function generateModulesFile(modules, outputFilePath, sourceDir)
	local outputFile = io.open(outputFilePath, "w")

	
end