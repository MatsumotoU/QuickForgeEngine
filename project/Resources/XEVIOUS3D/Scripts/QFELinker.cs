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

    // 時間
    public static class Time
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetDeltaTime();
    }

    public static class Input
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetKeyMoveDir(out Vector2 outDir);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyTrigger(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyPress(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyRelease(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetMouseScreenPos(out Vector2 outPos);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetMouseMoveDir(out Vector2 outDir);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseWheelDir();
    }

    public static class SceneManager
    {
       [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LoadScene(string sceneName); 
    }

    public static class Audio
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint LoadSound(string soundName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint PlaySound(uint soundHandle,bool isLoop,float volume);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StopSound(uint playHandle);
    }

    public static class Entity
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Create(string entityName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ChangeModel(uint entityID,string modelName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ChangeMesh(uint entityID,string meshName);
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
}