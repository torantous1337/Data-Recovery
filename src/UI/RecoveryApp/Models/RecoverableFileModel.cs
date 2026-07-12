namespace RecoveryApp.Models;

public sealed class RecoverableFileModel
{
    public ulong Id { get; set; }
    public string Name { get; set; } = string.Empty;
    public string Type { get; set; } = string.Empty;
    public ulong Size { get; set; }
    public string Status { get; set; } = "Unknown";
    public string PreviewText { get; set; } = string.Empty;
}
