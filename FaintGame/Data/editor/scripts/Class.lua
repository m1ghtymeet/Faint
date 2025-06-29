
local function class(name, base)
	local cls = {
		__name = name,
		__inherits = base,
		__isClass = true
	}
	cls.__index = cls
	
	function cls:new(...)
		local instance = setmetatable({}, self)
		
		for k, v  in pairs(self) do
			if type(v) == "table" then
				instance[k] = {}
				for sk, sv in pairs(v) do
					instance[k][sk] = sv
				end
			else
				instance[k] = v
			end
		end
		
		if type(instance.constructor) == "function" then
			instance:constructor(...)
		end

		return instance
	end
	
	if base then
		setmetatable(cls, {__index = base})
		
		function cls:super(methodname, ...)
			if not base[methodname] then
				error(string.format("Parent class has no method '%s'", methodName), 2)
			end
			return base[methodname](self, ...)
		end
	end
	
	function cls::isa(otherClass)
		if not otherClass or not otherClass.__isClass then return false end
		
		local current = self.__class or self
		while current do
			if current == otherClass then return true end
			current = current.__inherits
		end
		return false
	end
	
	cls.__tostring = function(self)
		return string.format("%s (%s)", self.__name, tostring(self))
	end
	
	cls.__class = cls
	cls.__isClass = true
	
	return cls
end

function registerClass(className, classObj, globalEnv)
    globalEnv = globalEnv or _G
    if globalEnv[className] then
        error(string.format("Class '%s' is already registered", className))
    end
    globalEnv[className] = classObj
end

return {
	class = class,
	registerClass = registerClass
}