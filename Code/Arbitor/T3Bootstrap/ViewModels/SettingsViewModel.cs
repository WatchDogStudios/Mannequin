/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using System.Collections.Generic;
using System.Threading.Tasks;
using CommunityToolkit.Mvvm.Input;
using T3Foundation;
using T3Foundation.Mvvm;
using T3Foundation.Services.Dialog;
using T3Foundation.Services.Settings;
using T3Foundation.Services.Theme;

namespace T3.ViewModels
{
  /// <summary>
  /// ViewModel for the settings panel.
  /// </summary>
  public class SettingsViewModel : T3ViewModelBase
  {
    private readonly IT3SettingsService _settings;
    private readonly IT3DialogService _dialogs;
    private readonly T3ThemeService _themeService;

    private string _testRunnerPath = "RendererTest.exe";
    private string _referenceImageDir = "Data/UnitTests/RendererTest/ReferenceImages";
    private string _outputDir = "TestOutput";
    private double _failureThreshold = 0.01;
    private string _selectedTheme = "MaterialDark";

    public string TestRunnerPath
    {
      get => _testRunnerPath;
      set => SetProperty(ref _testRunnerPath, value);
    }

    public string ReferenceImageDir
    {
      get => _referenceImageDir;
      set => SetProperty(ref _referenceImageDir, value);
    }

    public string OutputDir
    {
      get => _outputDir;
      set => SetProperty(ref _outputDir, value);
    }

    public double FailureThreshold
    {
      get => _failureThreshold;
      set => SetProperty(ref _failureThreshold, value);
    }

    public string SelectedTheme
    {
      get => _selectedTheme;
      set
      {
        if (SetProperty(ref _selectedTheme, value))
        {
          _themeService.ApplyTheme(value, System.Windows.Application.Current.MainWindow);
        }
      }
    }

    public IReadOnlyList<string> AvailableThemes => _themeService.AvailableThemes;

    public IAsyncRelayCommand BrowseTestRunnerCommand { get; }
    public IAsyncRelayCommand BrowseReferenceDirCommand { get; }
    public IAsyncRelayCommand BrowseOutputDirCommand { get; }
    public IAsyncRelayCommand SaveSettingsCommand { get; }
    public IRelayCommand ResetDefaultsCommand { get; }

    public SettingsViewModel(IT3SettingsService settings, IT3DialogService dialogs, T3ThemeService themeService)
    {
      _settings = settings;
      _dialogs = dialogs;
      _themeService = themeService;
      Title = "Settings";

      BrowseTestRunnerCommand = new AsyncRelayCommand(BrowseTestRunnerAsync);
      BrowseReferenceDirCommand = new AsyncRelayCommand(BrowseReferenceDirAsync);
      BrowseOutputDirCommand = new AsyncRelayCommand(BrowseOutputDirAsync);
      SaveSettingsCommand = new AsyncRelayCommand(SaveAsync);
      ResetDefaultsCommand = new RelayCommand(ResetDefaults);
    }

    public override Task InitializeAsync()
    {
      // Load current values from settings
      TestRunnerPath = _settings.Get("TestRunnerPath", "RendererTest.exe");
      ReferenceImageDir = _settings.Get("ReferenceImageDir", "Data/UnitTests/RendererTest/ReferenceImages");
      OutputDir = _settings.Get("OutputDir", "TestOutput");
      FailureThreshold = _settings.Get("FailureThreshold", 0.01);
      SelectedTheme = _settings.Get("Theme", "MaterialDark");

      return base.InitializeAsync();
    }

    private async Task BrowseTestRunnerAsync()
    {
      var path = await _dialogs.ShowOpenFileDialogAsync("Executables|*.exe|All Files|*.*", "Select Test Runner");
      if (path != null)
        TestRunnerPath = path;
    }

    private async Task BrowseReferenceDirAsync()
    {
      var path = await _dialogs.ShowFolderDialogAsync("Select Reference Image Directory");
      if (path != null)
        ReferenceImageDir = path;
    }

    private async Task BrowseOutputDirAsync()
    {
      var path = await _dialogs.ShowFolderDialogAsync("Select Output Directory");
      if (path != null)
        OutputDir = path;
    }

    private async Task SaveAsync()
    {
      _settings.Set("TestRunnerPath", TestRunnerPath);
      _settings.Set("ReferenceImageDir", ReferenceImageDir);
      _settings.Set("OutputDir", OutputDir);
      _settings.Set("FailureThreshold", FailureThreshold);
      _settings.Set("Theme", SelectedTheme);
      await _settings.SaveAsync();

      _dialogs.ShowNotification("Settings saved.", T3NotificationType.Success);
      T3Core.Log("Settings saved.", T3LogLevel.Info);
    }

    private void ResetDefaults()
    {
      TestRunnerPath = "RendererTest.exe";
      ReferenceImageDir = "Data/UnitTests/RendererTest/ReferenceImages";
      OutputDir = "TestOutput";
      FailureThreshold = 0.01;
      SelectedTheme = "MaterialDark";
    }
  }
}
