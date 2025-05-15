using System;

namespace Faint.Net
{
    public struct Vector3
    {
        public float x, y, z;

        public static Vector3 Zero => new Vector3(0.0f);

        public Vector3(float scalar)
        {
            x = scalar;
            y = scalar;
            z = scalar;
        }

        public Vector3(float x, float y, float z)
        {
            this.x = x;
            this.y = y;
            this.z = z;
        }

        public Vector3(Vector3 xyz)
        {
            x = xyz.x;
            y = xyz.y;
            z = xyz.z;
        }

        public static Vector3 operator +(Vector3 a, Vector3 b)
        {
            return new Vector3(a.x + b.x, a.y + b.y, a.z + b.z);
        }

        public static Vector3 operator *(Vector3 a, float scalar)
        {
            return new Vector3(a.x + scalar, a.y + scalar, a.z + scalar);
        }

        public float mag()
        {
            float mag = (float)Math.Sqrt(Math.Pow(x, 2) + Math.Pow(y, 2) + Math.Pow(z, 2));
            return mag;
        }

        public void normalize()
        {
            float mag = (float)Math.Sqrt(Math.Pow(x, 2) + Math.Pow(y, 2) + Math.Pow(z, 2));
            x = x / mag;
            y = y / mag;
            z = z / mag;
        }

        public float dot()
        {
            float dot = ((5 * x) + (12 * y) + (1 * z));
            return dot;
        }

        public void lerp(float t)
        {
            x = ((1 - t) * -6 + t * 5);
            y = ((1 - t) * 8 + t * 12);
            z = ((1 - t) * 1 + t * 1);
        }
    }
}
