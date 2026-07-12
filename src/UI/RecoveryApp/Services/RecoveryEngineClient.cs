using RecoveryApp.Interop;
using System.Text;

namespace RecoveryApp.Services;

public sealed class RecoveryEngineClient : IDisposable
{
    private nint _handle;

    public RecoveryEngineClient()
    {
        _handle = NativeMethods.Recovery_Create();
        if (_handle == nint.Zero)
        {
            throw new InvalidOperationException("Unable to initialize RecoveryEngine");
        }
    }

    public void StartScan(string source, string destination, string signatureDbPath, string sessionDbPath, int mode)
    {
        var options = new NativeMethods.RecoveryScanOptions
        {
            Source = source,
            Destination = destination,
            SignatureDb = signatureDbPath,
            SessionDb = sessionDbPath,
            Mode = mode
        };

        var result = NativeMethods.Recovery_StartScan(_handle, ref options);
        if (result != 0)
        {
            throw new InvalidOperationException(GetLastError());
        }
    }

    public void Pause() => EnsureOk(NativeMethods.Recovery_Pause(_handle));
    public void Resume() => EnsureOk(NativeMethods.Recovery_Resume(_handle));
    public void Cancel() => EnsureOk(NativeMethods.Recovery_Cancel(_handle));
    public void SaveSession(string sessionId) => EnsureOk(NativeMethods.Recovery_SaveSession(_handle, sessionId));
    public void LoadSession(string sessionId) => EnsureOk(NativeMethods.Recovery_LoadSession(_handle, sessionId));

    private void EnsureOk(int rc)
    {
        if (rc != 0)
        {
            throw new InvalidOperationException(GetLastError());
        }
    }

    public string GetLastError()
    {
        var sb = new StringBuilder(1024);
        _ = NativeMethods.Recovery_GetLastError(_handle, sb, 1024);
        return sb.ToString();
    }

    public void Dispose()
    {
        if (_handle != nint.Zero)
        {
            NativeMethods.Recovery_Destroy(_handle);
            _handle = nint.Zero;
        }
    }
}
