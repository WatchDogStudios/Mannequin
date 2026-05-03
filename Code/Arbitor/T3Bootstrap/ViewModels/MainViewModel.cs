/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using CommunityToolkit.Mvvm.Input;
using T3.Models;
using T3.Services;
using T3Foundation;
using T3Foundation.Commands;
using T3Foundation.Mvvm;
using T3Foundation.Services.Dialog;
using T3Foundation.Services.Settings;

namespace T3.ViewModels
{
  /// <summary>
  /// Comparison view modes for the image viewport.
  /// </summary>
  public enum ComparisonViewMode
  {
    SideBySide,
    Overlay,
    DiffOnly
  }

  /// <summary>
  /// Primary ViewModel for the Mannequin main window.
  /// Replaces all code-behind logic from MainWindow.xaml.cs.
  /// </summary>
  public class MainViewModel : T3ViewModelBase
  {
    private readonly T3TestRunnerService _testRunner;
    private readonly IT3DialogService _dialogs;
    private readonly IT3SettingsService _settings;
    private readonly T3CommandManager _commandManager;

    private T3VisualTestSummary? _lastSummary;
    private CancellationTokenSource? _cts;
    private readonly DebounceAction _filterDebounce;
    private IReadOnlyList<T3VisualTestResult> _discoveredTests = Array.Empty<T3VisualTestResult>();

    // --- Observable Collections ---
    public ObservableCollection<TestListItemViewModel> TestItems { get; } = new();
    public ObservableCollection<T3TestApplication> AvailableApplications { get; } = new();
    public ObservableCollection<string> LogMessages { get; } = new();

    // --- Selected Test ---
    private TestListItemViewModel? _selectedTest;
    public TestListItemViewModel? SelectedTest
    {
      get => _selectedTest;
      set
      {
        if (!SetProperty(ref _selectedTest, value))
          return;

        if (value?.Result != null)
          SelectedTestDetail.LoadFromResult(value.Result, _testRunner);
        else if (value != null)
          SelectedTestDetail.LoadFromDiscoveredTest(value);
        else
          SelectedTestDetail.Clear();
      }
    }

    // --- Test Detail ---
    public TestDetailViewModel SelectedTestDetail { get; } = new();

    // --- Test Filter ---
    private string _testFilter = "";
    public string TestFilter
    {
      get => _testFilter;
      set
      {
        if (SetProperty(ref _testFilter, value))
          _filterDebounce.Invoke();
      }
    }

    // --- API Selection ---
    private string _selectedAPI = "DX12";
    public string SelectedAPI
    {
      get => _selectedAPI;
      set
      {
        if (SetProperty(ref _selectedAPI, value))
          OnPropertyChanged(nameof(StatusBarAPI));
      }
    }

    public string[] AvailableAPIs { get; } = { "DX12", "Vulkan", "DX11", "All APIs" };

    // --- Application Selection ---
    private T3TestApplication? _selectedApplication;
    public T3TestApplication? SelectedApplication
    {
      get => _selectedApplication;
      set
      {
        if (!SetProperty(ref _selectedApplication, value))
          return;

        if (value != null)
        {
          _testRunner.SetTestRunnerPath(value.ExecutablePath);
          _settings.Set("TestRunnerPath", value.ExecutablePath);
          _settings.SaveAsync().FireAndForget();
        }

        OnPropertyChanged(nameof(StatusBarAPI));
        RefreshTestsAsync().FireAndForget();
      }
    }

    // --- Status Bar ---
    private string _statusBarText = "Ready";
    public string StatusBarText
    {
      get => _statusBarText;
      set => SetProperty(ref _statusBarText, value);
    }

    public string StatusBarAPI => $"App: {SelectedApplication?.DisplayName ?? "None"} | API: {SelectedAPI}";

    // --- Running State ---
    private bool _isTestRunning;
    public bool IsTestRunning
    {
      get => _isTestRunning;
      set => SetProperty(ref _isTestRunning, value);
    }

    // --- View Mode ---
    private ComparisonViewMode _viewMode = ComparisonViewMode.SideBySide;
    public ComparisonViewMode ViewMode
    {
      get => _viewMode;
      set => SetProperty(ref _viewMode, value);
    }

    private double _overlayOpacity = 0.5;
    public double OverlayOpacity
    {
      get => _overlayOpacity;
      set => SetProperty(ref _overlayOpacity, value);
    }

    // --- Summary Stats ---
    private string _totalTests = "0";
    private string _passedCount = "0";
    private string _failedCount = "0";
    private string _skippedCount = "0";
    private string _newBaselines = "0";
    private string _totalTime = "-";
    private double _testProgress;

    public string TotalTests { get => _totalTests; set => SetProperty(ref _totalTests, value); }
    public string PassedCount { get => _passedCount; set => SetProperty(ref _passedCount, value); }
    public string FailedCount { get => _failedCount; set => SetProperty(ref _failedCount, value); }
    public string SkippedCount { get => _skippedCount; set => SetProperty(ref _skippedCount, value); }
    public string NewBaselines { get => _newBaselines; set => SetProperty(ref _newBaselines, value); }
    public string TotalTime { get => _totalTime; set => SetProperty(ref _totalTime, value); }
    public double TestProgress { get => _testProgress; set => SetProperty(ref _testProgress, value); }

    // --- Commands ---
    public IAsyncRelayCommand RunAllTestsCommand { get; }
    public IAsyncRelayCommand RunSelectedTestsCommand { get; }
    public IRelayCommand StopTestsCommand { get; }
    public IAsyncRelayCommand UpdateBaselinesCommand { get; }
    public IAsyncRelayCommand LoadResultsFileCommand { get; }
    public IAsyncRelayCommand BrowseApplicationCommand { get; }
    public IAsyncRelayCommand RefreshTestsCommand { get; }
    public IRelayCommand ClearLogCommand { get; }
    public IAsyncRelayCommand ExportResultsCommand { get; }

    public MainViewModel(
      T3TestRunnerService testRunner,
      IT3DialogService dialogs,
      IT3SettingsService settings,
      T3CommandManager commandManager)
    {
      _testRunner = testRunner;
      _dialogs = dialogs;
      _settings = settings;
      _commandManager = commandManager;
      Title = "Mannequin — Visual Graphics Test Runner";

      // Debounced filter (250ms)
      _filterDebounce = new DebounceAction(() =>
        Application.Current.Dispatcher.BeginInvoke(ApplyFilter), 250);

      // Commands
      RunAllTestsCommand = new AsyncRelayCommand(RunAllTestsAsync, () => !IsTestRunning);
      RunSelectedTestsCommand = new AsyncRelayCommand(RunSelectedTestsAsync, () => !IsTestRunning);
      StopTestsCommand = new RelayCommand(StopTests, () => IsTestRunning);
      UpdateBaselinesCommand = new AsyncRelayCommand(UpdateBaselinesAsync, () => !IsTestRunning);
      LoadResultsFileCommand = new AsyncRelayCommand(LoadResultsFileAsync);
      BrowseApplicationCommand = new AsyncRelayCommand(BrowseApplicationAsync, () => !IsTestRunning);
      RefreshTestsCommand = new AsyncRelayCommand(RefreshTestsAsync, () => !IsTestRunning);
      ClearLogCommand = new RelayCommand(() => LogMessages.Clear());
      ExportResultsCommand = new AsyncRelayCommand(ExportResultsAsync, () => _lastSummary != null);

      // Wire up framework logging to LogMessages
      T3Core.OnLogMessage += (msg, level) =>
      {
        Application.Current?.Dispatcher.BeginInvoke(() =>
        {
          LogMessages.Add(msg);
        });
      };

      // Wire up test runner events
      _testRunner.OnTestOutput += (output) =>
      {
        Application.Current?.Dispatcher.BeginInvoke(() =>
        {
          LogMessages.Add(output);
        });
      };

      _testRunner.OnTestRunComplete += (summary) =>
      {
        Application.Current?.Dispatcher.BeginInvoke(() => UpdateSummary(summary));
      };
    }

    public override Task InitializeAsync()
    {
      // Load configuration from settings
      _testRunner.LoadFromSettings();
      LoadApplications();
      T3Core.Log("Mannequin Visual Graphics Test Runner ready.", T3LogLevel.Info);
      return base.InitializeAsync();
    }

    /// <summary>
    /// Register keyboard shortcuts with the command manager.
    /// </summary>
    public void RegisterShortcuts()
    {
      _commandManager.Register("app.runAll", RunAllTestsCommand,
        new System.Windows.Input.KeyGesture(System.Windows.Input.Key.F5), "Run All Tests");
      _commandManager.Register("app.stop", StopTestsCommand,
        new System.Windows.Input.KeyGesture(System.Windows.Input.Key.F5, System.Windows.Input.ModifierKeys.Shift), "Stop Tests");
      _commandManager.Register("app.export", ExportResultsCommand,
        new System.Windows.Input.KeyGesture(System.Windows.Input.Key.E, System.Windows.Input.ModifierKeys.Control), "Export Results");
    }

    // --- Command Implementations ---

    private async Task RunAllTestsAsync()
    {
      await RunTestsAsync(filters: null);
    }

    private async Task RunSelectedTestsAsync()
    {
      var selectedTestNames = TestItems
        .Where(test => test.IsIncluded)
        .Select(test => test.TestName)
        .Distinct(StringComparer.OrdinalIgnoreCase)
        .ToArray();

      if (selectedTestNames.Length == 0 && SelectedTest != null)
        selectedTestNames = new[] { SelectedTest.TestName };

      if (selectedTestNames.Length == 0)
      {
        T3Core.Log("No tests checked or selected.", T3LogLevel.Warning);
        return;
      }

      await RunTestsAsync(selectedTestNames);
    }

    private async Task RunTestsAsync(IEnumerable<string>? filters, bool updateBaselines = false)
    {
      if (SelectedApplication == null)
      {
        T3Core.Log("No test application selected.", T3LogLevel.Warning);
        return;
      }

      var apis = SelectedAPI == "All APIs"
        ? new[] { "DX12", "Vulkan", "DX11" }
        : new[] { SelectedAPI };

      var filtersToRun = filters?
        .Where(filter => !string.IsNullOrWhiteSpace(filter))
        .Distinct(StringComparer.OrdinalIgnoreCase)
        .ToArray() ?? Array.Empty<string>();

      _cts = new CancellationTokenSource();
      IsTestRunning = true;
      NotifyCommandsCanExecuteChanged();
      StatusBarText = filtersToRun.Length == 0 ? "Running all tests..." : $"Running {filtersToRun.Length} selected test(s)...";

      try
      {
        T3VisualTestSummary? summary;
        if (filtersToRun.Length == 0)
        {
          var summaries = new List<T3VisualTestSummary>();
          foreach (string api in apis)
          {
            StatusBarText = apis.Length == 1 ? "Running all tests..." : $"Running all tests on {api}...";
            var partial = await _testRunner.RunTestsAsync(new[] { api }, updateBaselines: updateBaselines, cancellationToken: _cts.Token);
            if (partial != null)
              summaries.Add(partial);

            if (_cts.IsCancellationRequested)
              break;
          }

          summary = summaries.Count == 1 ? summaries[0] : MergeSummaries(summaries);
        }
        else
        {
          var summaries = new List<T3VisualTestSummary>();
          foreach (string api in apis)
          {
            foreach (string filter in filtersToRun)
            {
              StatusBarText = apis.Length == 1 ? $"Running {filter}..." : $"Running {filter} on {api}...";
              var partial = await _testRunner.RunTestsAsync(new[] { api }, filter, updateBaselines, _cts.Token);
              if (partial != null)
                summaries.Add(FilterSummaryToSelection(partial, filter));

              if (_cts.IsCancellationRequested)
                break;
            }

            if (_cts.IsCancellationRequested)
              break;
          }

          summary = MergeSummaries(summaries);
        }

        if (summary != null)
        {
          UpdateSummary(summary);
          StatusBarText = GetRunStatusText(summary);

          _dialogs.ShowNotification(StatusBarText,
            summary.FailedCount == 0 && summary.SkippedCount == 0 ? T3NotificationType.Success : T3NotificationType.Warning);
        }
        else
        {
          StatusBarText = "Test run completed with errors.";
        }
      }
      finally
      {
        IsTestRunning = false;
        NotifyCommandsCanExecuteChanged();
      }
    }

    private void StopTests()
    {
      _cts?.Cancel();
      _testRunner.AbortTests();
      IsTestRunning = false;
      NotifyCommandsCanExecuteChanged();
    }

    private async Task UpdateBaselinesAsync()
    {
      var result = await _dialogs.ShowMessageAsync(
        "Update Baselines",
        "This will update reference images from the latest test output.\nAre you sure?",
        T3DialogButton.YesNo);

      if (result != T3DialogResult.Yes)
        return;

      await RunTestsAsync(filters: null, updateBaselines: true);

      T3Core.Log("Baselines updated.", T3LogLevel.Info);
      _dialogs.ShowNotification("Baselines updated.", T3NotificationType.Success);
    }

    private async Task LoadResultsFileAsync()
    {
      var path = await _dialogs.ShowOpenFileDialogAsync("JSON Results|*.json|All Files|*.*", "Load Test Results");
      if (path == null)
        return;

      var summary = T3VisualTestSummary.LoadFromJson(path);
      if (summary != null)
      {
        _testRunner.SetResultsDirectory(System.IO.Path.GetDirectoryName(path));
        UpdateSummary(summary);
        T3Core.Log($"Loaded results from {path}", T3LogLevel.Info);
      }
    }

    private async Task BrowseApplicationAsync()
    {
      var path = await _dialogs.ShowOpenFileDialogAsync("Applications|*.exe|All Files|*.*", "Select Test Application");
      if (path == null)
        return;

      var existing = AvailableApplications.FirstOrDefault(app =>
        string.Equals(app.ExecutablePath, path, StringComparison.OrdinalIgnoreCase));

      if (existing == null)
      {
        existing = new T3TestApplication
        {
          DisplayName = System.IO.Path.GetFileNameWithoutExtension(path),
          ExecutablePath = path
        };
        AvailableApplications.Insert(0, existing);
      }

      SelectedApplication = existing;
    }

    private async Task RefreshTestsAsync()
    {
      if (IsTestRunning)
        return;

      if (SelectedApplication == null)
      {
        _discoveredTests = Array.Empty<T3VisualTestResult>();
        RebuildTestItems(_discoveredTests, includeResults: false);
        StatusBarText = "No test application selected.";
        return;
      }

      StatusBarText = $"Loading tests for {SelectedApplication.DisplayName}...";
      var discovered = await Task.Run(() => _testRunner.DiscoverTests());
      _discoveredTests = discovered.ToArray();
      _lastSummary = null;

      TotalTests = _discoveredTests.Count.ToString();
      PassedCount = "0";
      FailedCount = "0";
      SkippedCount = "0";
      NewBaselines = "0";
      TotalTime = "-";
      TestProgress = 0;

      RebuildTestItems(_discoveredTests, includeResults: false);
      ExportResultsCommand.NotifyCanExecuteChanged();

      StatusBarText = _discoveredTests.Count == 0
        ? "No tests discovered. Build the test app or browse to one with results."
        : $"Loaded {_discoveredTests.Count} test group(s).";
    }

    private async Task ExportResultsAsync()
    {
      if (_lastSummary == null) return;

      var path = await _dialogs.ShowSaveFileDialogAsync(
        "HTML Report|*.html|CSV|*.csv|All Files|*.*", "Export Test Results");
      if (path == null) return;

      var exportService = new T3ExportService();
      if (path.EndsWith(".csv", StringComparison.OrdinalIgnoreCase))
        await exportService.ExportToCsvAsync(_lastSummary, path);
      else
        await exportService.ExportToHtmlAsync(_lastSummary, path);

      T3Core.Log($"Results exported to {path}", T3LogLevel.Info);
      _dialogs.ShowNotification($"Results exported to {path}", T3NotificationType.Success);
    }

    // --- Internal Logic ---

    private void LoadApplications()
    {
      AvailableApplications.Clear();
      foreach (var app in _testRunner.DiscoverApplications())
        AvailableApplications.Add(app);

      SelectedApplication = AvailableApplications.FirstOrDefault(app =>
          string.Equals(app.ExecutablePath, _testRunner.TestRunnerPath, StringComparison.OrdinalIgnoreCase))
        ?? AvailableApplications.FirstOrDefault(app => app.Exists)
        ?? AvailableApplications.FirstOrDefault();

      if (SelectedApplication == null)
        StatusBarText = "No test applications found. Build RendererTest or browse to an executable.";
    }

    private void UpdateSummary(T3VisualTestSummary summary)
    {
      if (summary.Results.Count == 0 && _discoveredTests.Count > 0 && summary.TotalTests > 0)
      {
        summary.Results = _discoveredTests
          .Select(test => new T3VisualTestResult
          {
            TestName = test.TestName,
            API = SelectedAPI == "All APIs" ? "" : SelectedAPI,
            Passed = false,
            RenderSucceeded = false,
            ReferenceExists = false,
            ErrorMessage = "Skipped"
          })
          .ToList();
      }

      _lastSummary = summary;

      TotalTests = summary.TotalTests.ToString();
      PassedCount = summary.PassedCount.ToString();
      FailedCount = summary.FailedCount.ToString();
      SkippedCount = summary.SkippedCount.ToString();
      NewBaselines = summary.NewBaselines.ToString();
      TotalTime = $"{summary.TotalTimeMs:F0} ms";
      TestProgress = 100;

      RebuildTestItems(summary.Results, includeResults: true);

      ExportResultsCommand.NotifyCanExecuteChanged();
    }

    private void ApplyFilter()
    {
      if (_lastSummary != null)
        RebuildTestItems(_lastSummary.Results, includeResults: true);
      else
        RebuildTestItems(_discoveredTests, includeResults: false);
    }

    private void RebuildTestItems(IEnumerable<T3VisualTestResult> tests, bool includeResults)
    {
      var checkedNames = TestItems
        .Where(test => test.IsIncluded)
        .Select(test => test.TestName)
        .ToHashSet(StringComparer.OrdinalIgnoreCase);
      string? selectedName = SelectedTest?.TestName;
      string filter = TestFilter.ToLowerInvariant();

      TestItems.Clear();
      SelectedTest = null;

      foreach (var r in tests)
      {
        if (string.IsNullOrEmpty(filter) || r.TestName.ToLowerInvariant().Contains(filter))
        {
          var item = includeResults
            ? TestListItemViewModel.FromResult(r)
            : TestListItemViewModel.FromDiscoveredTest(r);

          item.IsIncluded = checkedNames.Contains(item.TestName);
          TestItems.Add(item);
        }
      }

      if (!string.IsNullOrWhiteSpace(selectedName))
        SelectedTest = TestItems.FirstOrDefault(test => string.Equals(test.TestName, selectedName, StringComparison.OrdinalIgnoreCase));
    }

    private static T3VisualTestSummary FilterSummaryToSelection(T3VisualTestSummary summary, string filter)
    {
      var filteredResults = summary.Results
        .Where(result =>
          result.RenderSucceeded ||
          string.Equals(result.TestName, filter, StringComparison.OrdinalIgnoreCase) ||
          result.TestName.IndexOf(filter, StringComparison.OrdinalIgnoreCase) >= 0)
        .ToList();

      if (filteredResults.Count == 0)
        filteredResults = summary.Results.ToList();

      var filteredSummary = new T3VisualTestSummary
      {
        Results = filteredResults
      };
      RecalculateSummary(filteredSummary);
      return filteredSummary;
    }

    private static T3VisualTestSummary? MergeSummaries(IReadOnlyList<T3VisualTestSummary> summaries)
    {
      if (summaries.Count == 0)
        return null;

      var merged = new T3VisualTestSummary();
      foreach (var summary in summaries)
      {
        foreach (var result in summary.Results)
          merged.Results.Add(result);
      }

      RecalculateSummary(merged);
      return merged;
    }

    private static void RecalculateSummary(T3VisualTestSummary summary)
    {
      summary.TotalTests = (uint)summary.Results.Count;
      summary.PassedCount = 0;
      summary.FailedCount = 0;
      summary.SkippedCount = 0;
      summary.TotalTimeMs = 0;

      foreach (var result in summary.Results)
      {
        bool skipped = !result.RenderSucceeded &&
          string.Equals(result.ErrorMessage, "Skipped", StringComparison.OrdinalIgnoreCase);

        if (skipped)
          summary.SkippedCount++;
        else if (result.Passed)
          summary.PassedCount++;
        else
          summary.FailedCount++;

        summary.TotalTimeMs += result.RenderTimeMs;
      }
    }

    private static string GetRunStatusText(T3VisualTestSummary summary)
    {
      if (summary.FailedCount > 0)
        return $"{summary.FailedCount} test(s) failed.";

      if (summary.SkippedCount > 0)
        return $"{summary.SkippedCount} test(s) skipped.";

      return $"All {summary.PassedCount} tests passed.";
    }

    private void NotifyCommandsCanExecuteChanged()
    {
      RunAllTestsCommand.NotifyCanExecuteChanged();
      RunSelectedTestsCommand.NotifyCanExecuteChanged();
      StopTestsCommand.NotifyCanExecuteChanged();
      UpdateBaselinesCommand.NotifyCanExecuteChanged();
      BrowseApplicationCommand.NotifyCanExecuteChanged();
      RefreshTestsCommand.NotifyCanExecuteChanged();
    }

    protected override void Dispose(bool disposing)
    {
      if (disposing)
      {
        _filterDebounce.Dispose();
        _cts?.Dispose();
      }
      base.Dispose(disposing);
    }
  }
}
