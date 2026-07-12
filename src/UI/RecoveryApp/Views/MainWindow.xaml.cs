using RecoveryApp.ViewModels;
using System.Windows;

namespace RecoveryApp.Views;

public partial class MainWindow : Window
{
    public MainWindow()
    {
        InitializeComponent();
        DataContext = new MainViewModel();
    }
}
