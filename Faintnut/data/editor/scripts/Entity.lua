local class = require("Class")
EntityWrapper = class("Entity")

function EntityWrapper:constructor(nativeEntity)
	self.native = nativeEntity -- nativeEntity = Entity
end

function EntityWrapper:GetName()
	return self.native:GetName()
end

function EntityWrapper:HasComponent(name)
	if name == "Transform" then return self.native:HasTransform() end
	return false
end

function EntityWrapper:GetComponent(name)
	--if name == "Transform" then return HasComponent("Transform") end
	--return false
end

return EntityWrapper