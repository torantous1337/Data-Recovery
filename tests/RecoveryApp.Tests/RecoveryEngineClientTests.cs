using RecoveryApp.Services;
using Xunit;

namespace RecoveryApp.Tests;

public class RecoveryEngineClientTests
{
    [Fact]
    public void CreatingClientWithoutNativeDllThrowsControlledErrorOnNonWindows()
    {
        if (OperatingSystem.IsWindows())
        {
            return;
        }

        Assert.ThrowsAny<Exception>(() => _ = new RecoveryEngineClient());
    }
}
