using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace QuickForgeEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector4
    {
        public float x, y, z, w;

        public static Vector4 operator +(Vector4 a, Vector4 b) => new Vector4 { x = a.x + b.x, y = a.y + b.y, z = a.z + b.z, w = a.w + b.w };
        public static Vector4 operator -(Vector4 a, Vector4 b) => new Vector4 { x = a.x - b.x, y = a.y - b.y, z = a.z - b.z, w = a.w - b.w };
        public static Vector4 operator *(Vector4 a, float b) => new Vector4 { x = a.x * b, y = a.y * b, z = a.z * b, w = a.w * b };
        public static Vector4 operator /(Vector4 a, float b) => new Vector4 { x = a.x / b, y = a.y / b, z = a.z / b, w = a.w / b };
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float x, y, z;

        public static Vector3 operator +(Vector3 a, Vector3 b) => new Vector3 { x = a.x + b.x, y = a.y + b.y, z = a.z + b.z };
        public static Vector3 operator -(Vector3 a, Vector3 b) => new Vector3 { x = a.x - b.x, y = a.y - b.y, z = a.z - b.z };
        public static Vector3 operator *(Vector3 a, float b) => new Vector3 { x = a.x * b, y = a.y * b, z = a.z * b };
        public static Vector3 operator /(Vector3 a, float b) => new Vector3 { x = a.x / b, y = a.y / b, z = a.z / b };
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2
    {
        public float x, y;

        public static Vector2 operator +(Vector2 a, Vector2 b) => new Vector2 { x = a.x + b.x, y = a.y + b.y };
        public static Vector2 operator -(Vector2 a, Vector2 b) => new Vector2 { x = a.x - b.x, y = a.y - b.y };
        public static Vector2 operator *(Vector2 a, float b) => new Vector2 { x = a.x * b, y = a.y * b };
        public static Vector2 operator /(Vector2 a, float b) => new Vector2 { x = a.x / b, y = a.y / b };
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Transform
    {
        public Vector3 scale, rotate, translate;
    }
    
}