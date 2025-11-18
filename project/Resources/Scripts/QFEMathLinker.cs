using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace QuickForgeEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector4
    {
        public float x, y, z, w;
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float x, y, z;
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2
    {
        public float x, y;
    }
}