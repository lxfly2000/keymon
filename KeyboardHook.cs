using System;
using System.Diagnostics;
using System.Runtime.InteropServices;

namespace keymonp
{
    public class KeyboardHook
    {
        IntPtr hHook;
        WindowsAPI.HookProc KeyboardHookFunction;
        public delegate void KeyPressProc(KeyEnum key, bool pressed);
        public KeyPressProc KeyPressFunction;
        
        public void SetHook()
        {
            KeyboardHookFunction = new WindowsAPI.HookProc(KeyboardHookProcess);
            hHook = WindowsAPI.SetWindowsHookEx(WindowsAPI.WH_KEYBOARD_LL, KeyboardHookFunction,
                WindowsAPI.GetModuleHandle(Process.GetCurrentProcess().MainModule.ModuleName), 0);
        }

        public void UnHook()
        {
            WindowsAPI.UnhookWindowsHookEx(hHook);
        }

        private Int32 KeyboardHookProcess(int nCode, UIntPtr wParam, Int32 lParam)
        {
            int key = (int)wParam;
            if (nCode == WindowsAPI.HC_ACTION)
                switch (key)
                {
                    case WindowsAPI.WM_KEYDOWN:
                    case WindowsAPI.WM_KEYUP:
                        WindowsAPI.KBDLLHOOKSTRUCT d = (WindowsAPI.KBDLLHOOKSTRUCT)Marshal.PtrToStructure((IntPtr)lParam, typeof(WindowsAPI.KBDLLHOOKSTRUCT));
                        switch (d.vkCode)
                        {
                            case WindowsAPI.VK_LSHIFT: KeyPressFunction(KeyEnum.kShift, key == WindowsAPI.WM_KEYDOWN); break;
                            case 'Z': KeyPressFunction(KeyEnum.kZ, key == WindowsAPI.WM_KEYDOWN); break;
                            case 'X': KeyPressFunction(KeyEnum.kX, key == WindowsAPI.WM_KEYDOWN); break;
                            case 'C': KeyPressFunction(KeyEnum.kC, key == WindowsAPI.WM_KEYDOWN); break;
                            case WindowsAPI.VK_LEFT: KeyPressFunction(KeyEnum.kLeft, key == WindowsAPI.WM_KEYDOWN); break;
                            case WindowsAPI.VK_UP: KeyPressFunction(KeyEnum.kUp, key == WindowsAPI.WM_KEYDOWN); break;
                            case WindowsAPI.VK_DOWN: KeyPressFunction(KeyEnum.kDown, key == WindowsAPI.WM_KEYDOWN); break;
                            case WindowsAPI.VK_RIGHT: KeyPressFunction(KeyEnum.kRight, key == WindowsAPI.WM_KEYDOWN); break;
                        }
                        break;
                }
            return WindowsAPI.CallNextHookEx(hHook, nCode, wParam, lParam);
        }

        public enum KeyEnum { kShift, kZ, kX, kC, kLeft, kUp, kDown, kRight };
    }
}
