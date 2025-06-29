using Coral.Managed.Interop;
using Faint.Net;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Drawing;
using System.Numerics;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace Faint.Net
{
    /// <summary>
    /// This is the core Nuake.Net API. 
    /// All internal call should happen in this file.
    /// </summary>
    public class Engine
    {
        internal static unsafe delegate*<NativeString, void> LoggerLogIcall;
        internal static unsafe delegate*<NativeString, void> LoadSceneIcall;

        public Engine() { }

        public static void LoadScene(string path)
        {
            unsafe { LoadSceneIcall(path); }
        }

        /// <summary>
        /// Prints a message to the console log
        /// </summary>
        /// <param name="message">message to be printed</param>
        public static void Log(string input)
        {
            unsafe { LoggerLogIcall(input); }
        }
    }

    public struct AABB
    {
        public Vector3 A;
        public Vector3 B;

        public AABB(Vector3 a, Vector3 b)
        {
            A = a;
            B = b;
        }

        public AABB(float ax, float ay, float az, float bx, float by, float bz)
        {
            A = new Vector3(ax, ay, az);
            B = new Vector3(bx, by, bz);
        }
    }

    [AttributeUsage(AttributeTargets.Field)]
    public sealed class ExposedAttribute : Attribute
    {
        private bool HasDefaultValue = false;
        private int DefaultValueInternalInt;
        private float DefaultValueInternalFloat;
        private bool DefaultValueInternalBool;
        private string DefaultValueInternalString;

        public ExposedAttribute(int DefaultValue)
        {
            DefaultValueInternalInt = DefaultValue;
            HasDefaultValue = true;
        }

        public ExposedAttribute(float DefaultValue)
        {
            this.DefaultValueInternalFloat = DefaultValue;
            HasDefaultValue = true;
        }

        public ExposedAttribute(string DefaultValue)
        {
            this.DefaultValueInternalString = DefaultValue;
            HasDefaultValue = true;
        }

        public ExposedAttribute(bool DefaultValue)
        {
            this.DefaultValueInternalBool = DefaultValue;
            HasDefaultValue = true;
        }

        public ExposedAttribute() { }
    }

    [AttributeUsage(AttributeTargets.Class)]
    public sealed class BrushScript : Attribute
    {
        public string Description = "";
        public bool IsTrigger = false;

        public BrushScript(string description = "", bool isTrigger = false)
        {
            Description = description;
            IsTrigger = isTrigger;
        }
    }

    [AttributeUsage(AttributeTargets.Class)]
    public sealed class PointScript : Attribute
    {
        public string Description = "";
        public string Base = "";
        public Vector3 AABBA;
        public Vector2 AABBB;

        public List<float> AABBData = new();
        public List<float> ColorData = new();

        public Color Color
        {
            get { return Color; }
            set
            {
                ColorData.Add(value.R);
                ColorData.Add(value.G);
                ColorData.Add(value.B);
            }
        }

        string Model;
        float ModelScale = 1.0f;
        string Sprite;

        public PointScript(string description = "")
        {
            Description = description;
        }
    }

    public class Debug
    {
        internal static unsafe delegate*<float, float, float,
                                         float, float, float,
                                         float, float, float,
                                         void> DrawLineIcall;

        public Debug() { }

        public static void DrawLine(Vector3 start, Vector3 end, Vector3 color)
        {
            unsafe
            {
                DrawLineIcall(start.x, start.y, start.z, end.x, end.y, end.z, color.x, color.y, color.z);
            }
        }
    }

}