using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace QuickForgeEngine
{
    [StructLayout(LayoutKind.Sequential)]
    public struct Vector4
    {
        public float x, y, z, w;
        public float Length() => (float)Math.Sqrt(this.x * this.x + this.y * this.y + this.z * this.z + this.w * this.w);
        public Vector4 Normalize()
        {
            float len = Length();
            if (len > 0)
            {
                return new Vector4 { x = this.x / len, y = this.y / len, z = this.z / len, w = this.w / len };
            }
            return new Vector4 { x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f };
        }

        public static Vector4 Zero => new Vector4 { x = 0.0f, y = 0.0f, z = 0.0f, w = 0.0f };
        public static Vector4 operator +(Vector4 a, Vector4 b) => new Vector4 { x = a.x + b.x, y = a.y + b.y, z = a.z + b.z, w = a.w + b.w };
        public static Vector4 operator -(Vector4 a, Vector4 b) => new Vector4 { x = a.x - b.x, y = a.y - b.y, z = a.z - b.z, w = a.w - b.w };
        public static Vector4 operator *(Vector4 a, float b) => new Vector4 { x = a.x * b, y = a.y * b, z = a.z * b, w = a.w * b };
        public static Vector4 operator /(Vector4 a, float b) => new Vector4 { x = a.x / b, y = a.y / b, z = a.z / b, w = a.w / b };
    };

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector3
    {
        public float x, y, z;

        public float Length() => (float)Math.Sqrt(this.x * this.x + this.y * this.y + this.z * this.z); 
        public Vector3 Normalize()
        {
            float len = Length();
            if (len > 0)
            {
                return new Vector3 { x = this.x / len, y = this.y / len, z = this.z / len };
            }
            return new Vector3 { x = 0.0f, y = 0.0f, z = 0.0f };
        }

        public static Vector3 Zero => new Vector3 { x = 0.0f, y = 0.0f, z = 0.0f };

        public static Vector3 operator +(Vector3 a, Vector3 b) => new Vector3 { x = a.x + b.x, y = a.y + b.y, z = a.z + b.z };
        public static Vector3 operator -(Vector3 a, Vector3 b) => new Vector3 { x = a.x - b.x, y = a.y - b.y, z = a.z - b.z };
        public static Vector3 operator *(Vector3 a, float b) => new Vector3 { x = a.x * b, y = a.y * b, z = a.z * b };
        public static Vector3 operator /(Vector3 a, float b) => new Vector3 { x = a.x / b, y = a.y / b, z = a.z / b };
        public static float Length(Vector3 v) => (float)Math.Sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
    }

    [StructLayout(LayoutKind.Sequential)]
    public struct Vector2
    {
        public float x, y;
        public float Length() => (float)Math.Sqrt(this.x * this.x + this.y * this.y);
        public Vector2 Normalize()
        {
            float len = Length();
            if (len > 0)
            {
                return new Vector2 { x = this.x / len, y = this.y / len };
            }
            return new Vector2 { x = 0, y = 0 };
        }

        public static Vector2 Zero => new Vector2 { x = 0.0f, y = 0.0f };

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