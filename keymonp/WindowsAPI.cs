using System;
using System.Runtime.InteropServices;

namespace keymonp
{
    //http://jingyan.baidu.com/article/ceb9fb10ebd9f68cad2ba03c.html
    public class WindowsAPI
    {
        #region Definitions
        public const int WM_KEYDOWN = 0x100,
            WM_KEYUP = 0x101,
            WM_SYSKEYDOWN = 0x104,
            WM_SYSKEYUP = 0x105,
            WH_KEYBOARD_LL = 13,
            HC_ACTION = 0,
            VK_LSHIFT = 0xA0,
            VK_UP = 0x26,
            VK_DOWN = 0x28,
            VK_LEFT = 0x25,
            VK_RIGHT = 0x27;

        [StructLayout(LayoutKind.Sequential)]
        public struct KBDLLHOOKSTRUCT
        {
            public UInt32 vkCode;
            public UInt32 scanCode;
            public UInt32 flags;
            public UInt32 time;
            public UInt32 dwExtraInfo;
        }
        #endregion
        #region API Functions
        public delegate Int32 HookProc(int nCode, UIntPtr wParam, Int32 lParam);

        [DllImport("user32.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr SetWindowsHookEx(int idHook, HookProc lpFunction, IntPtr hInstance, UInt32 threadID);

        [DllImport("user32.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern int UnhookWindowsHookEx(IntPtr idHook);

        //下一个钩挂的函数 
        [DllImport("user32.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern Int32 CallNextHookEx(IntPtr idHook, int nCode, UIntPtr wParam, Int32 lParam);

        [DllImport("kernel32.dll", CharSet = CharSet.Auto, CallingConvention = CallingConvention.StdCall)]
        public static extern IntPtr GetModuleHandle(string lpModuleName);
        #endregion
    }
}
