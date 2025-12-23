using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Runtime.InteropServices;

namespace Moon
{
    [StructLayout(LayoutKind.Sequential)]
    public class Entity
    {
        public enum ComponentTypes
        {
            None = 0,
            Transform,
            Camera,
            MeshRenderer,
            PointLight,
            DirectionalLight
        }

        public int id { get; set; }
        
        public Entity(string name)
        {
            // TODO: Create entity
        }
        public Entity(string name, string tag)
        {
            // TODO: Create entity
        }

        public virtual void Start() { }
        public virtual void Update() { }
        public virtual void FixedUpdate() { }
        public virtual void Destroy() { }

        public T? AddComponent<T>() where T : IComponent
        {
            return (T?)Activator.CreateInstance(typeof(T), id);
        }
    }
}