using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace QuickForgeEngine
{
    // QuickForgeEngineのC#側の内部プロパティを記述するクラス
    public static class InternalProperty
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetTransforms(uint[] ids, Transform[] trs, ref uint count);
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetTransforms(uint[] ids, Transform[] trs, uint count);
    }
}