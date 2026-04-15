using Syncfusion.SfSkinManager;
using System;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using T3Foundation;
using T3Foundation.Models;
using T3Foundation.Services;

namespace T3
{
  /// <summary>
  /// View model for test list items.
  /// </summary>
  public class TestListItem : INotifyPropertyChanged
  {
    public string TestName { get; set; } = "";
    public string API { get; set; } = "";
    public bool Passed { get; set; }
    public double MeanError { get; set; }
    public string MeanErrorText => MeanError > 0 ? $"(err: {MeanError:F4})" : "";
    public T3VisualTestResult? Result { get; set; }

    public Brush StatusColor => Result == null ? Brushes.Gray :
      (Result.Passed ? Brushes.LimeGreen :
      (Result.RenderSucceeded ? Brushes.OrangeRed : Brushes.Red));

    public event PropertyChangedEventHandler? PropertyChanged;
    public void Refresh() => PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(""));
  }

  /// <summary>
  /// Interaction logic for MainWindow.xaml — Mannequin Visual Test Runner GUI
  /// </summary>
  public partial class MainWindow : Window
  {
    private readonly T3TestRunnerService _testRunner = new();
    private readonly ObservableCollection<TestListItem> _testItems = new();
    private readonly ObservableCollection<string> _logMessages = new();
    private T3VisualTestSummary? _lastSummary;
    private CancellationTokenSource? _cts;

    public MainWindow()
    {
      // Theme initialization
      string style = "basetheme";
      SkinHelper? styleInstance = null;
      var skinHelpterStr = "Syncfusion.Themes." + style + ".WPF." + style + "SkinHelper, Syncfusion.Themes." + style + ".WPF";
      Type? skinHelpterType = Type.GetType(skinHelpterStr);
      if (skinHelpterType != null)
        styleInstance = Activator.CreateInstance(skinHelpterType) as SkinHelper;
      if (styleInstance != null)
        SfSkinManager.RegisterTheme("basetheme", styleInstance);
      SfSkinManager.SetTheme(this, new Theme("basetheme;MaterialDark"));

      InitializeComponent();

      // Bind collections
      LB_TestList.ItemsSource = _testItems;
      LB_Log.ItemsSource = _logMessages;

      // Wire up framework logging to the UI log panel
      T3Core.OnLogMessage += (msg, level) =>
      {
        Dispatcher.BeginInvoke(() =>
        {
          _logMessages.Add(msg);
          LB_Log.ScrollIntoView(msg);
        });
      };

      // Wire up test runner events
      _testRunner.OnTestOutput += (output) =>
      {
        Dispatcher.BeginInvoke(() =>
        {
          _logMessages.Add(output);
        });
      };

      _testRunner.OnTestRunComplete += (summary) =>
      {
        Dispatcher.BeginInvoke(() => UpdateSummary(summary));
      };

      // Initialize framework
      T3Core.Initialize();
      T3Core.Log("Mannequin Visual Graphics Test Runner ready.", T3LogLevel.Info);

      // Default configuration
      _testRunner.Configure(
        testRunnerExe: "RendererTest.exe",
        referenceDir: "Data/UnitTests/RendererTest/ReferenceImages",
        outputDir: "TestOutput"
      );
    }

    // --- Toolbar Event Handlers ---

    private void OnFileClick(object sender, RoutedEventArgs e)
    {
      // File menu — load results, export, etc.
      var dialog = new Microsoft.Win32.OpenFileDialog
      {
        Filter = "JSON Results|*.json|All Files|*.*",
        Title = "Load Test Results"
      };

      if (dialog.ShowDialog() == true)
      {
        var summary = T3VisualTestSummary.LoadFromJson(dialog.FileName);
        if (summary != null)
        {
          UpdateSummary(summary);
          T3Core.Log($"Loaded results from {dialog.FileName}", T3LogLevel.Info);
        }
      }
    }

    private async void OnRunTestsClick(object sender, RoutedEventArgs e)
    {
      await RunTests(filter: null);
    }

    private async void OnRunSelectedClick(object sender, RoutedEventArgs e)
    {
      var selected = LB_TestList.SelectedItems;
      if (selected.Count == 0)
      {
        T3Core.Log("No tests selected.", T3LogLevel.Warning);
        return;
      }

      var names = selected.Cast<TestListItem>().Select(t => t.TestName);
      await RunTests(filter: string.Join(",", names));
    }

    private void OnStopTestsClick(object sender, RoutedEventArgs e)
    {
      _cts?.Cancel();
      _testRunner.AbortTests();
      SetRunningState(false);
    }

    private async void OnUpdateBaselinesClick(object sender, RoutedEventArgs e)
    {
      var result = MessageBox.Show(
        "This will update reference images from the latest test output.\nAre you sure?",
        "Update Baselines", MessageBoxButton.YesNo, MessageBoxImage.Question);

      if (result == MessageBoxResult.Yes)
      {
        string api = GetSelectedAPI();
        _cts = new CancellationTokenSource();
        SetRunningState(true);
        await _testRunner.RunTestsAsync(new[] { api }, updateBaselines: true, cancellationToken: _cts.Token);
        SetRunningState(false);
        T3Core.Log("Baselines updated.", T3LogLevel.Info);
      }
    }

    private void OnSettingsClick(object sender, RoutedEventArgs e)
    {
      T3Core.Log("Settings panel — configure test runner paths, thresholds, and comparison settings.", T3LogLevel.Info);
    }

    private void OnAPISelectionChanged(object sender, SelectionChangedEventArgs e)
    {
      string api = GetSelectedAPI();
      if (TB_StatusBarAPI != null)
        TB_StatusBarAPI.Text = $"API: {api}";
    }

    // --- Test List ---

    private void OnTestFilterChanged(object sender, TextChangedEventArgs e)
    {
      string filter = TB_TestFilter.Text.ToLowerInvariant();
      if (_lastSummary == null)
        return;

      _testItems.Clear();
      foreach (var r in _lastSummary.Results)
      {
        if (string.IsNullOrEmpty(filter) || r.TestName.ToLowerInvariant().Contains(filter))
        {
          _testItems.Add(new TestListItem
          {
            TestName = r.TestName,
            API = r.API,
            Passed = r.Passed,
            MeanError = r.MeanError,
            Result = r
          });
        }
      }
    }

    private void OnTestSelectionChanged(object sender, SelectionChangedEventArgs e)
    {
      if (LB_TestList.SelectedItem is TestListItem item && item.Result != null)
      {
        ShowTestDetails(item.Result);
      }
    }

    // --- View Mode ---

    private void OnViewModeChanged(object sender, RoutedEventArgs e)
    {
      // Toggle between side-by-side, overlay, and diff-only view modes
    }

    private void OnClearLogClick(object sender, RoutedEventArgs e)
    {
      _logMessages.Clear();
    }

    // --- Core Logic ---

    private async Task RunTests(string? filter)
    {
      string api = GetSelectedAPI();
      var apis = api == "All APIs"
        ? new[] { "DX12", "Vulkan", "DX11" }
        : new[] { api };

      _cts = new CancellationTokenSource();
      SetRunningState(true);
      TB_StatusBar.Text = "Running tests...";

      var summary = await _testRunner.RunTestsAsync(apis, filter, cancellationToken: _cts.Token);

      SetRunningState(false);

      if (summary != null)
      {
        UpdateSummary(summary);
        TB_StatusBar.Text = summary.FailedCount == 0
          ? $"All {summary.PassedCount} tests passed!"
          : $"{summary.FailedCount} test(s) failed.";
      }
      else
      {
        TB_StatusBar.Text = "Test run completed with errors.";
      }
    }

    private void UpdateSummary(T3VisualTestSummary summary)
    {
      _lastSummary = summary;

      // Update summary panel
      TB_TotalTests.Text = summary.TotalTests.ToString();
      TB_PassedTests.Text = summary.PassedCount.ToString();
      TB_FailedTests.Text = summary.FailedCount.ToString();
      TB_SkippedTests.Text = summary.SkippedCount.ToString();
      TB_NewBaselines.Text = summary.NewBaselines.ToString();
      TB_TotalTime.Text = $"{summary.TotalTimeMs:F0} ms";
      PB_TestProgress.Value = 100;

      // Populate test list
      _testItems.Clear();
      foreach (var r in summary.Results)
      {
        _testItems.Add(new TestListItem
        {
          TestName = r.TestName,
          API = r.API,
          Passed = r.Passed,
          MeanError = r.MeanError,
          Result = r
        });
      }
    }

    private void ShowTestDetails(T3VisualTestResult result)
    {
      // Update properties panel
      TB_TestName.Text = result.TestName;
      TB_Status.Text = result.StatusText;
      TB_Status.Foreground = result.Passed ? Brushes.LimeGreen : Brushes.OrangeRed;
      TB_API.Text = result.API;
      TB_RenderTime.Text = $"{result.RenderTimeMs:F1} ms";
      TB_MeanError.Text = $"{result.MeanError:F6}";
      TB_MaxError.Text = $"{result.MaxError:F6}";
      TB_MedianError.Text = $"{result.MedianError:F6}";
      TB_P95.Text = $"{result.P95Error:F6}";
      TB_PixelsFailed.Text = $"{result.PixelsFailed} / {result.TotalPixels}";
      TB_FailurePercent.Text = $"{result.FailurePercentage:F2}%";
      TB_ErrorMsg.Text = result.ErrorMessage ?? "";

      // Load comparison images
      var (testPath, refPath, diffPath) = _testRunner.GetComparisonImages(result.API, result.TestName);
      IMG_TestOutput.Source = T3ImageService.LoadImage(testPath ?? "");
      IMG_Reference.Source = T3ImageService.LoadImage(refPath ?? "");
      IMG_Diff.Source = T3ImageService.LoadImage(diffPath ?? "");
    }

    private void SetRunningState(bool running)
    {
      B_RunTests.IsEnabled = !running;
      B_RunSelected.IsEnabled = !running;
      B_StopTests.IsEnabled = running;
      B_UpdateBaselines.IsEnabled = !running;
    }

    private string GetSelectedAPI()
    {
      if (CB_GraphicsAPI.SelectedItem is ComboBoxItem item)
        return item.Content?.ToString() ?? "DX12";
      return "DX12";
    }
  }
}
