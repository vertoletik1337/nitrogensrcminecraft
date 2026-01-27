using System;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

class NitrogenInjector {
    [DllImport("kernel32.dll")]
    public static extern IntPtr OpenProcess(int dwDesiredAccess, bool bInheritHandle, int dwProcessId);
    [DllImport("kernel32.dll")]
    public static extern IntPtr GetModuleHandle(string lpModuleName);
    [DllImport("kernel32.dll")]
    public static extern IntPtr GetProcAddress(IntPtr hModule, string procName);
    [DllImport("kernel32.dll")]
    public static extern IntPtr VirtualAllocEx(IntPtr hProcess, IntPtr lpAddress, uint dwSize, uint flAllocationType, uint flProtect);
    [DllImport("kernel32.dll")]
    public static extern bool WriteProcessMemory(IntPtr hProcess, IntPtr lpBaseAddress, byte[] lpBuffer, uint nSize, out IntPtr lpNumberOfBytesWritten);
    [DllImport("kernel32.dll")]
    public static extern IntPtr CreateRemoteThread(IntPtr hProcess, IntPtr lpThreadAttributes, uint dwStackSize, IntPtr lpStartAddress, IntPtr lpParameter, uint dwCreationFlags, IntPtr lpThreadId);

    static void Main() {
        string dllPath = AppDomain.CurrentDomain.BaseDirectory + "Nitrogen.dll";
        Process[] processes = Process.GetProcessesByName("javaw"); // Майн обычно тут

        if (processes.Length > 0) {
            Console.WriteLine("Процесс найден. Инжекчу Nitrogen.dll...");
            IntPtr hProc = OpenProcess(0x1F0FFF, false, processes[0].Id);
            IntPtr addr = VirtualAllocEx(hProc, IntPtr.Zero, (uint)dllPath.Length + 1, 0x1000, 0x40);
            IntPtr outPtr;
            WriteProcessMemory(hProc, addr, Encoding.Default.GetBytes(dllPath), (uint)dllPath.Length + 1, out outPtr);
            CreateRemoteThread(hProc, IntPtr.Zero, 0, GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA"), addr, 0, IntPtr.Zero);
            
            Console.WriteLine("Готово! Тефтели полетели.");
            System.Threading.Thread.Sleep(2000);
        }
    }
}
