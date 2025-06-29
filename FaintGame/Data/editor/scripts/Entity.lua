local class = require("Class")
local Entity = class.class("Entity")

local COMPONENTS = {
	TRANSFORM = "Transform",
	CAMERA = "Camera",
	NAME = "Name"
}

function Entity:constructor(nativeEntity)
	if not nativeEntity then
		error("Cannot create Entity wrapper without native entity")
	end
	
	self.native = nativeEntity -- nativeEntity = C++ Entity object
	self._children = {}
	self._componentsChache = {}
end

function Entity:GetName()
	local nameComp = self:GetComponent(COMPONENTS.NAME)
	return nameComp and nameComp.name or "(Unnamed)"
end

function Entity:HasComponent(name)
	if name == "Transform" then return self.native:HasTransform() end
	if name == "Name" then return self.native:HasName() end
	return false
end

function Entity:GetComponent(name)
	if not self:HasComponent(name) then
		print("[Warning] Entity does not have component: " .. name)
		return nil
	end
	
	if name == "Transform" then return self.native:GetTransform() end
	if name == "Camera" then return self.native:GetCamera() end
	--if name == "RigidBody" then return self.native:GetRigidBody() end
	--if name == "Audio" then return self.native:GetAudio() end
	if name == "Name" then return self.native:GetNameComponent() end
	
	return nil
end

function Entity:AddChild(childEntity)
	if childEntity and childEntity.native then
		self.native:AddChild(childEntity.native)
	end
end

Entity.COMPONENTS = COMPONENTS

return Entity