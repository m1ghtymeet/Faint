local Character = {
    moveSpeed = 10.0,
    jumpPower = 8.0,
	airVelocity = 0.3,
	
	velocityY = 0.0,
	wasGrounded = false
}
function Character:Start()
    self.controller = self.owner:GetCharacterController()
	
	self.lastPosition = self.owner:GetTransform():GetLocalPosition()
    self.wasGrounded = self.controller:IsGrounded()
end

function Character:Update(dt)
	local isGrounded = self.controller:IsGrounded()
	self.wasGrounded = isGrounded
	
    local move = Vector3.new(0, 0, 0)
	if Input.KeyDown(Key.W) then move.z = move.z - 1 * dt end
    if Input.KeyDown(Key.S) then move.z = move.z + 1 * dt end
    if Input.KeyDown(Key.A) then move.x = move.x - 1 * dt end
    if Input.KeyDown(Key.D) then move.x = move.x + 1 * dt end
	
	local forward = self.owner:GetChildByName("Camera"):GetCamera():GetForward()
	local right = self.owner:GetChildByName("Camera"):GetCamera():GetRight()
	
	forward.y = 0
	right.y = 0
	forward = forward:normalized()
	right = right:normalized()
	
	local wishDir = (forward * move.z + right * move.x)
	if wishDir:length() > 0 then
		wishDir = wishDir:normalized()
	end
	local moveVelocity = wishDir * self.moveSpeed
	if not IsGrounded then
		moveVelocity  = moveVelocity * self.airVelocity
	end
	
	if self.controller:IsGrounded() then
		self.velocityY = -0.1
	else
		self.velocityY = self.velocityY - 18 * dt
	end
	
	local displacement = Vector3.new(
		moveVelocity.x * dt,
		self.velocityY,
		moveVelocity.z * dt
	)

    self.controller:Move(displacement)
	-- self.owner:GetTransform():SetLocalPosition(self.controller:GetFootPosition())
end
return Character