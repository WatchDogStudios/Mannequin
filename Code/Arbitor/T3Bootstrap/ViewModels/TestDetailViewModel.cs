/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows.Media;
using T3.Models;
using T3.Services;
using T3Foundation.Mvvm;

namespace T3.ViewModels
{
  /// <summary>
  /// ViewModel for the test details/properties panel.
  /// </summary>
  public class TestDetailViewModel : T3ObservableObject
  {
    private string _testName = "No test selected";
    private string _statusText = "-";
    private Brush _statusBrush = Brushes.Gray;
    private string _api = "-";
    private string _renderTime = "-";
    private string _meanError = "-";
    private string _maxError = "-";
    private string _medianError = "-";
    private string _p95Error = "-";
    private string _pixelsFailed = "-";
    private string _failurePercent = "-";
    private string _errorMessage = "";
    private ImageSource? _testImage;
    private ImageSource? _referenceImage;
    private ImageSource? _diffImage;
    private ImageSource? _errorOverlayImage;
    private string _apiDiagnosticsSummary = "No API diagnostics";
    private Brush _apiDiagnosticsBrush = Brushes.Gray;
    private string _resourceSummary = "No captured resources";
    private Brush _resourceSummaryBrush = Brushes.Gray;

    public string TestName { get => _testName; set => SetProperty(ref _testName, value); }
    public string StatusText { get => _statusText; set => SetProperty(ref _statusText, value); }
    public Brush StatusBrush { get => _statusBrush; set => SetProperty(ref _statusBrush, value); }
    public string API { get => _api; set => SetProperty(ref _api, value); }
    public string RenderTime { get => _renderTime; set => SetProperty(ref _renderTime, value); }
    public string MeanError { get => _meanError; set => SetProperty(ref _meanError, value); }
    public string MaxError { get => _maxError; set => SetProperty(ref _maxError, value); }
    public string MedianError { get => _medianError; set => SetProperty(ref _medianError, value); }
    public string P95Error { get => _p95Error; set => SetProperty(ref _p95Error, value); }
    public string PixelsFailed { get => _pixelsFailed; set => SetProperty(ref _pixelsFailed, value); }
    public string FailurePercent { get => _failurePercent; set => SetProperty(ref _failurePercent, value); }
    public string ErrorMessage { get => _errorMessage; set => SetProperty(ref _errorMessage, value); }
    public ImageSource? TestImage { get => _testImage; set => SetProperty(ref _testImage, value); }
    public ImageSource? ReferenceImage { get => _referenceImage; set => SetProperty(ref _referenceImage, value); }
    public ImageSource? DiffImage { get => _diffImage; set => SetProperty(ref _diffImage, value); }
    public ImageSource? ErrorOverlayImage { get => _errorOverlayImage; set => SetProperty(ref _errorOverlayImage, value); }
    public string ApiDiagnosticsSummary { get => _apiDiagnosticsSummary; set => SetProperty(ref _apiDiagnosticsSummary, value); }
    public Brush ApiDiagnosticsBrush { get => _apiDiagnosticsBrush; set => SetProperty(ref _apiDiagnosticsBrush, value); }
    public string ResourceSummary { get => _resourceSummary; set => SetProperty(ref _resourceSummary, value); }
    public Brush ResourceSummaryBrush { get => _resourceSummaryBrush; set => SetProperty(ref _resourceSummaryBrush, value); }
    public ObservableCollection<ApiCallCheckViewModel> ApiCallChecks { get; } = new();
    public ObservableCollection<ResourceSnapshotViewModel> ResourceSnapshots { get; } = new();

    /// <summary>
    /// Populate from a test result and load comparison images.
    /// </summary>
    public void LoadFromResult(T3VisualTestResult result, T3TestRunnerService runner)
    {
      TestName = result.TestName;
      StatusText = result.StatusText;
      StatusBrush = result.StatusText == "SKIPPED" ? Brushes.Gray : (result.Passed ? Brushes.LimeGreen : Brushes.OrangeRed);
      API = result.API;
      RenderTime = $"{result.RenderTimeMs:F1} ms";
      MeanError = $"{result.MeanError:F6}";
      MaxError = $"{result.MaxError:F6}";
      MedianError = $"{result.MedianError:F6}";
      P95Error = $"{result.P95Error:F6}";
      PixelsFailed = $"{result.PixelsFailed} / {result.TotalPixels}";
      FailurePercent = $"{result.FailurePercentage:F2}%";
      ErrorMessage = result.ErrorMessage ?? "";

      // Load comparison images
      var (testPath, refPath, diffPath) = runner.GetComparisonImages(result.API, result.TestName);
      TestImage = T3ImageService.LoadImage(testPath ?? "");
      ReferenceImage = T3ImageService.LoadImage(refPath ?? "");
      DiffImage = T3ImageService.LoadImage(diffPath ?? "");
      ErrorOverlayImage = T3ImageService.CreateErrorOverlayImage(testPath, refPath) ?? DiffImage ?? TestImage;

      SetApiCallChecks(T3ApiDiagnosticsService.BuildChecks(result));
      SetResourceSnapshots(result, runner);
    }

    /// <summary>
    /// Populate details for a discovered test that has not produced results yet.
    /// </summary>
    public void LoadFromDiscoveredTest(TestListItemViewModel test)
    {
      TestName = test.TestName;
      StatusText = "Not run";
      StatusBrush = Brushes.Gray;
      API = string.IsNullOrWhiteSpace(test.API) ? "-" : test.API;
      RenderTime = "-";
      MeanError = "-";
      MaxError = "-";
      MedianError = "-";
      P95Error = "-";
      PixelsFailed = "-";
      FailurePercent = "-";
      ErrorMessage = "";
      TestImage = null;
      ReferenceImage = null;
      DiffImage = null;
      ErrorOverlayImage = null;
      SetApiCallChecks(new[]
      {
        new T3ApiCallCheck
        {
          Category = "Instrumentation",
          CallName = "RunTest",
          Passed = false,
          Severity = "Warning",
          Message = "This test has not run yet.",
          Recommendation = "Run the test to collect API call checks and recommendations for the selected backend."
        }
      });
      ResourceSnapshots.Clear();
      ResourceSummary = "Run test to inspect RHI state";
      ResourceSummaryBrush = Brushes.Goldenrod;
    }

    /// <summary>
    /// Reset to default empty state.
    /// </summary>
    public void Clear()
    {
      TestName = "No test selected";
      StatusText = "-";
      StatusBrush = Brushes.Gray;
      API = "-";
      RenderTime = "-";
      MeanError = "-";
      MaxError = "-";
      MedianError = "-";
      P95Error = "-";
      PixelsFailed = "-";
      FailurePercent = "-";
      ErrorMessage = "";
      TestImage = null;
      ReferenceImage = null;
      DiffImage = null;
      ErrorOverlayImage = null;
      ApiCallChecks.Clear();
      ApiDiagnosticsSummary = "No API diagnostics";
      ApiDiagnosticsBrush = Brushes.Gray;
      ResourceSnapshots.Clear();
      ResourceSummary = "No captured resources";
      ResourceSummaryBrush = Brushes.Gray;
    }

    private void SetApiCallChecks(IEnumerable<T3ApiCallCheck> checks)
    {
      ApiCallChecks.Clear();

      foreach (var check in checks)
        ApiCallChecks.Add(new ApiCallCheckViewModel(check));

      int total = ApiCallChecks.Count;
      int fatal = ApiCallChecks.Count(check => string.Equals(check.StatusText, "FATAL", StringComparison.OrdinalIgnoreCase));
      int failed = ApiCallChecks.Count(check => string.Equals(check.StatusText, "FAIL", StringComparison.OrdinalIgnoreCase));
      int warnings = ApiCallChecks.Count(check => string.Equals(check.StatusText, "WARN", StringComparison.OrdinalIgnoreCase));
      int passed = Math.Max(0, total - fatal - failed - warnings);

      ApiDiagnosticsSummary = total == 0
        ? "No API diagnostics"
        : $"{passed} pass, {warnings} warn, {fatal + failed} fatal";

      ApiDiagnosticsBrush = fatal + failed > 0 ? Brushes.OrangeRed :
        (warnings > 0 ? Brushes.Goldenrod : Brushes.LimeGreen);
    }

    private void SetResourceSnapshots(T3VisualTestResult result, T3TestRunnerService runner)
    {
      ResourceSnapshots.Clear();

      foreach (var snapshot in result.ResourceSnapshots)
        ResourceSnapshots.Add(new ResourceSnapshotViewModel(snapshot, runner, result.API, result.TestName));

      int fatal = ResourceSnapshots.Count(snapshot => string.Equals(snapshot.StatusText, "FATAL", StringComparison.OrdinalIgnoreCase));
      int warnings = ResourceSnapshots.Count(snapshot => string.Equals(snapshot.StatusText, "WARN", StringComparison.OrdinalIgnoreCase));

      ResourceSummary = ResourceSnapshots.Count == 0
        ? "No captured resources"
        : $"{ResourceSnapshots.Count} captured, {warnings} warn, {fatal} fatal";

      ResourceSummaryBrush = fatal > 0 ? Brushes.OrangeRed :
        (warnings > 0 ? Brushes.Goldenrod : (ResourceSnapshots.Count > 0 ? Brushes.LimeGreen : Brushes.Gray));
    }
  }
}
