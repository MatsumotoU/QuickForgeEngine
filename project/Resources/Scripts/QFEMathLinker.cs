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

    // C++����API���Ăяo�����߂̓����N���X
    internal static class TransformInternal
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetTranslate(uint entityId, out QuickForgeEngine.Vector3 translate);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetTranslate(uint entityId, ref QuickForgeEngine.Vector3 translate);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetRotate(uint entityId, out QuickForgeEngine.Vector3 rotate);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetRotate(uint entityId, ref QuickForgeEngine.Vector3 rotate);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void GetScale(uint entityId, out QuickForgeEngine.Vector3 scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal static extern void SetScale(uint entityId, ref QuickForgeEngine.Vector3 scale);
    }

    // �X�N���v�g����A�N�Z�X���邽�߂�Transform�N���X
    public class Transform
    {
        private readonly uint entityId;

        internal Transform(uint entityId)
        {
            this.entityId = entityId;
        }

        public QuickForgeEngine.Vector3 Translate
        {
            get { TransformInternal.GetTranslate(entityId, out QuickForgeEngine.Vector3 result); return result; }
            set { TransformInternal.SetTranslate(entityId, ref value); }
        }

        public QuickForgeEngine.Vector3 Rotate
        {
            get { TransformInternal.GetRotate(entityId, out QuickForgeEngine.Vector3 result); return result; }
            set { TransformInternal.SetRotate(entityId, ref value); }
        }

        public QuickForgeEngine.Vector3 Scale
        {
            get { TransformInternal.GetScale(entityId, out QuickForgeEngine.Vector3 result); return result; }
            set { TransformInternal.SetScale(entityId, ref value); }
        }
    }
}