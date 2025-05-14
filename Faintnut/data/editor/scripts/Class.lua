
local function class(name, base)
	local cls = {}
	cls.__index = cls
	cls.__name = name
	
	function cls:new(...)
		local instance = setmetatable({}, self)
		if instance.constructor then
			instance:constructor(...)
		end
		return instance
	end
	
	if base then
		setmetatable(cls, {__index = base})
		cls.__base = base
	end
	
	return cls
end

return class