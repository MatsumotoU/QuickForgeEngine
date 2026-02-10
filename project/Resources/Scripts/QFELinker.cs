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

	// 入力機能
    public static class Input
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern QuickForgeEngine.Vector3 GetKeyMoveDir();

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyTrigger(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyPress(string actionName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetKeyRelease(string actionName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetMousePress(int button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetMouseTrigger(int button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetMouseRelease(int button);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern QuickForgeEngine.Vector2 GetMouseScreenPos();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern QuickForgeEngine.Vector3 GetMouseMoveDir();
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern float GetMouseWheelDir();
    }

    // オーディオ管理
    public static class Audio
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint LoadSound(string soundName);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint PlaySound(uint soundHandle, bool isLoop, float volume);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void StopSound(uint playHandle);
    }

    // シーン管理
	public static class SceneManager
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void LoadScene(string sceneName);
    }
    
    // エンティティ生成
    public static class Entity 
    {
         [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern uint Create(string entityName);

         [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ChangeModel(uint entityId, string modelName);

         [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void ChangeMesh(uint entityId, string meshName);
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