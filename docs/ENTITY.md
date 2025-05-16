# Entity System (Faint Engine)

Faint uses an ECS (Entity Component System) model based on [EnTT](https://github.com/skypjack/entt).

---

# Creating Entities

```cpp
Entity player = scene->CreateEntity("Player");

/* ^^^ Adding Component ^^^ */
player.AddComponent<MeshRendererComponent>();
player.AddComponent<RigidBodyComponent>();
player.AddComponent<BoxColliderComponent>();

/* ^^^ Getting Components ^^^ */
auto& transform = player.GetComponent<TransformComponent>();
transform.SetLocalPosition(Vec3(0.0f, 1.0f, 0.0f));

/* ^^^ Checking Components ^^^ */
if (player.HasComponent<RigidBodyComponent>()) {
	// Apply physics logic
}

/* ^^^ Destroying Entities ^^^ */
scene->DestroyEntity(player);
```

---