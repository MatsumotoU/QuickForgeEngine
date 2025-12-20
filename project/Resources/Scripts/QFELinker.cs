using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace QuickForgeEngine
{
    // C++のログ機能をC#から呼び出すためのDebugクラス
    public static class Debug
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);
    }

    public static class Input
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyTrigger(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyPress(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyRelease(string actionName);
    }

    public static class Time
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetDeltaTime();
    }

    public static class Entity
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Create(string name);
    }

    // C#スクリプト用QuickForgeコンポーネントクラス
    public abstract class QuickForgeComponent
    {
        public uint EntityID { get; internal set; }
        public virtual void Initialize() { }
        public virtual void Update() { }

        private Transform _transform;
        public Transform transform
        {
            get
            {
                if (_transform == null)
                {
                    _transform = new Transform(EntityID);
                }
                return _transform;
            }
        }
    }

    public abstract class QuickForgeParticleSystem
    {
        public uint EntityID { get; internal set; }
        public virtual void Initialize() { }
        public virtual void Update() { }
        private Transform _transform;
        public Transform transform
        {
            get
            {
                if (_transform == null)
                {
                    _transform = new Transform(EntityID);
                }
                return _transform;
            }
        }
    }
}