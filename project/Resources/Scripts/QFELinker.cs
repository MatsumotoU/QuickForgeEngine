using System;

namespace QuickForgeEngine
{
    public static class QFELinker
    {
        public static void Log(string message)
        {
            Console.WriteLine($"[QFELinker] {message}");
            // In a real engine, this would interface with the native C++ logging system.
        }

        public static int Add(int a, int b)
        {
            return a + b;
        }
    }
}
