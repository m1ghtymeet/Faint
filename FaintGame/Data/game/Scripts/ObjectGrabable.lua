local ObjectGrabbable = {
    objectGrabPointTransform = nil
}

function ObjectGrabbable:Start()
    self.objectRigidbody = self.owner:GetRigidBody()
end

function ObjectGrabbable:Drag(transform)
    self.objectGrabPointTransform = transform
    self.objectRigidbody:SetGravityScale(0.0)
end

function ObjectGrabbable:Drop()
    self.objectGrabPointTransform = nil
	self.objectRigidbody:SetGravityScale(1.0)
end

function ObjectGrabbable:Update(dt)
    if self.objectGrabPointTransform ~= nil then
        --local newPosition = Math:Lerp(self.owner:GetTransform():GetLocalPosition(), self.objectGrabPointTransform:GetWorldPosition(), dt * 10.0)
        self.objectRigidbody:MovePosition(self.objectGrabPointTransform:GetWorldPosition())
    end
end

return ObjectGrabbable