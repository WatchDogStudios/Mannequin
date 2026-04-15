/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using System.Text.Json;
using System.Text.Json.Serialization;

namespace T3Foundation.Models
{
  /// <summary>
  /// Represents the result of a single visual test comparison.
  /// Maps to the JSON output from nsVisualTestRunner::ExportResultsJSON.
  /// </summary>
  public class T3VisualTestResult
  {
    [JsonPropertyName("testName")]
    public string TestName { get; set; } = "";

    [JsonPropertyName("api")]
    public string API { get; set; } = "";

    [JsonPropertyName("passed")]
    public bool Passed { get; set; }

    [JsonPropertyName("renderSucceeded")]
    public bool RenderSucceeded { get; set; }

    [JsonPropertyName("referenceExists")]
    public bool ReferenceExists { get; set; }

    [JsonPropertyName("renderTimeMs")]
    public double RenderTimeMs { get; set; }

    [JsonPropertyName("meanError")]
    public double MeanError { get; set; }

    [JsonPropertyName("maxError")]
    public double MaxError { get; set; }

    [JsonPropertyName("medianError")]
    public double MedianError { get; set; }

    [JsonPropertyName("p95Error")]
    public double P95Error { get; set; }

    [JsonPropertyName("pixelsFailed")]
    public uint PixelsFailed { get; set; }

    [JsonPropertyName("totalPixels")]
    public uint TotalPixels { get; set; }

    [JsonPropertyName("failurePercentage")]
    public double FailurePercentage { get; set; }

    [JsonPropertyName("error")]
    public string? ErrorMessage { get; set; }

    /// <summary>
    /// Status string for UI display.
    /// </summary>
    [JsonIgnore]
    public string StatusText => Passed ? "PASSED" : (RenderSucceeded ? "FAILED" : "ERROR");
  }

  /// <summary>
  /// Summary of a complete test suite run.
  /// </summary>
  public class T3VisualTestSummary
  {
    [JsonPropertyName("totalTests")]
    public uint TotalTests { get; set; }

    [JsonPropertyName("passed")]
    public uint PassedCount { get; set; }

    [JsonPropertyName("failed")]
    public uint FailedCount { get; set; }

    [JsonPropertyName("skipped")]
    public uint SkippedCount { get; set; }

    [JsonPropertyName("newBaselines")]
    public uint NewBaselines { get; set; }

    [JsonPropertyName("totalTimeMs")]
    public double TotalTimeMs { get; set; }

    [JsonPropertyName("results")]
    public List<T3VisualTestResult> Results { get; set; } = new();

    /// <summary>
    /// Parse from JSON file exported by the C++ test runner.
    /// </summary>
    public static T3VisualTestSummary? LoadFromJson(string path)
    {
      try
      {
        string json = System.IO.File.ReadAllText(path);
        return JsonSerializer.Deserialize<T3VisualTestSummary>(json);
      }
      catch (Exception ex)
      {
        T3Core.Log($"Failed to load test results: {ex.Message}", T3LogLevel.Error);
        return null;
      }
    }
  }

  /// <summary>
  /// Represents a graphics API backend available for testing.
  /// </summary>
  public class T3GraphicsBackend
  {
    public string Name { get; set; } = "";
    public string DisplayName { get; set; } = "";
    public bool IsAvailable { get; set; }
    public bool IsSelected { get; set; }
  }
}
