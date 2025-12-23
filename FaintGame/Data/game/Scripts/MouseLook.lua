-- Lua Script
local MouseLook = {
    pitch = 0.0,
    yaw = 0.0,
    sensitivity = 4.0
}

local objectGrabable = nil

function MouseLook:Start()
    self.camera = self.owner:GetCamera()
    Input.DisableCursor()

    playerCameraTransform = self.owner:GetTransform()
    objectGrabPointTransform = self.owner:GetChildByName("ObjectGrabPoint"):GetTransform()
end

function MouseLook:Update(dt)
    local transform = self.owner:GetTransform()

    self.pitch = self.pitch + -Input.GetMouseYOffset() * self.sensitivity * dt
    self.yaw = self.yaw + -Input.GetMouseXOffset() * self.sensitivity * dt
    self.owner:Parent():GetTransform():SetLocalRotation(Vector3.new(0, self.yaw, 0))
    transform:SetLocalRotation(Vector3.new(self.pitch, 0, 0))
	
    if Input.KeyPressed(Key.E) then
        if objectGrabable == nil then
            local hit = Physics.Raycast(self.owner:GetTransform():GetWorldPosition(), self.owner:GetTransform():ToForwardVector() * Vector3.new(-1), 2, 1, true)
            if hit.name == "cup" then
                local entity = hit.actor.owner
                objectGrabable = entity:GetBehaviour("ObjectGrabable")
                objectGrabable:Drag(objectGrabPointTransform)
            end
        else
            objectGrabable:Drop()
            objectGrabable = nil
        end
    end
end
return MouseLook