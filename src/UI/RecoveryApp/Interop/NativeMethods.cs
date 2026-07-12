using System.Runtime.InteropServices;
using System.Text;

namespace RecoveryApp.Interop;

internal static class NativeMethods
{
    [StructLayout(LayoutKind.Sequential)]
    internal struct RecoveryScanOptions
    {
        public string Source;
        public string Destination;
        public string SignatureDb;
        public string SessionDb;
        public int Mode;
    }

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern nint Recovery_Create();

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern void Recovery_Destroy(nint handle);

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Recovery_StartScan(nint handle, ref RecoveryScanOptions options);

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Recovery_Pause(nint handle);

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Recovery_Resume(nint handle);

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Recovery_Cancel(nint handle);

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Recovery_SaveSession(nint handle, string sessionId);

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Recovery_LoadSession(nint handle, string sessionId);

    [DllImport("RecoveryEngine.dll", CallingConvention = CallingConvention.Cdecl)]
    internal static extern int Recovery_GetLastError(nint handle, StringBuilder buffer, uint bufferLen);
}
