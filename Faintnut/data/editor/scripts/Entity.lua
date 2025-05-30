local class = require("Class")
local Entity = class("Entity")

function Entity:constructor(nativeEntity)
	self.native = nativeEntity -- nativeEntity = Entity
end

function Entity:GetName()
	return self.native:GetName()
end

function Entity:HasComponent(name)
	if name == "Transform" then return self.native:HasTransform() end
	return false
end

function Entity:GetComponent(name)
	--if name == "Transform" then return HasComponent("Transform") end
	--return false
end

return Entity