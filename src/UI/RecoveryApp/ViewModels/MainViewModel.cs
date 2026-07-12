using RecoveryApp.Models;
using RecoveryApp.Services;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.IO;
using System.Runtime.CompilerServices;
using System.Windows.Input;

namespace RecoveryApp.ViewModels;

public sealed class MainViewModel : INotifyPropertyChanged
{
    private readonly RecoveryEngineClient _client;
    private string _source = @"\\.\PhysicalDrive0";
    private string _destination = @"D:\Recovered";
    private string _sessionId = "default-session";

    public MainViewModel()
    {
        _client = new RecoveryEngineClient();
        StartCommand = new RelayCommand(_ => StartScan());
        PauseCommand = new RelayCommand(_ => _client.Pause());
        ResumeCommand = new RelayCommand(_ => _client.Resume());
        SaveSessionCommand = new RelayCommand(_ => _client.SaveSession(SessionId));
        LoadSessionCommand = new RelayCommand(_ => _client.LoadSession(SessionId));
    }

    public ObservableCollection<RecoverableFileModel> Files { get; } = new();

    public string Source
    {
        get => _source;
        set => SetField(ref _source, value);
    }

    public string Destination
    {
        get => _destination;
        set => SetField(ref _destination, value);
    }

    public string SessionId
    {
        get => _sessionId;
        set => SetField(ref _sessionId, value);
    }

    public ICommand StartCommand { get; }
    public ICommand PauseCommand { get; }
    public ICommand ResumeCommand { get; }
    public ICommand SaveSessionCommand { get; }
    public ICommand LoadSessionCommand { get; }

    public event PropertyChangedEventHandler? PropertyChanged;

    private void StartScan()
    {
        var signatureDb = Path.Combine(AppContext.BaseDirectory, "data", "signatures", "default_signatures.json");
        var sessionDb = Path.Combine(AppContext.BaseDirectory, "sessions.db");
        _client.StartScan(Source, Destination, signatureDb, sessionDb, mode: 1);
    }

    private void SetField<T>(ref T field, T value, [CallerMemberName] string? name = null)
    {
        if (EqualityComparer<T>.Default.Equals(field, value))
        {
            return;
        }

        field = value;
        PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
    }

    private sealed class RelayCommand : ICommand
    {
        private readonly Action<object?> _execute;

        public RelayCommand(Action<object?> execute) => _execute = execute;

        public event EventHandler? CanExecuteChanged;

        public bool CanExecute(object? parameter) => true;

        public void Execute(object? parameter) => _execute(parameter);
    }
}
