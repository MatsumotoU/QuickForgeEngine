using System;
using System.Runtime.CompilerServices;

namespace QuickForgeEngine
{
    // C++のログ機能をC#から呼び出すためのDebugクラス
    public static class Debug
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Log(string message);
    }

    // C#スクリプト用QuickForgeコンポーネントクラス
    public abstract class QuickForgeComponent
    {
        public virtual void Initialize() { }
        public virtual void Update() { }
    }
}
