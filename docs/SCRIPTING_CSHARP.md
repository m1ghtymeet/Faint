

---

## C# Scripting API: 'SCRIPTING_CSHARP.md'

# Scripting (C#) - Faint Engine

Faint supports scripting via Coral, a C# integration layer.

---

## Basic Script Structure

```csharp
using Faint;

public class PlayerController : Entity
{
	public override void OnStart()
	{
		Console.WriteLine("Player Initialized");
	}
	
	public override void OnUpdate(float dt)
	{
		if (Input.KeyDown(Key.W))
			Transform.Position += Vector3.Forward * dt;
	}
}
```

## Input Example
```csharp
if (Input.LeftMouseButton())
	Console.WriteLine("Left click!");
```

## Exposed Events
- OnStart() - Called once when the script is Initialized
- OnUpdate(float dt) - Called every frame
- OnFixedUpdate(float dt) - Called every 60 frame