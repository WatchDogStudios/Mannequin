/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using System;
using System.Collections.Generic;
using Newtonsoft.Json;

namespace T3Foundation.Models
{
  /// <summary>
  /// Represents the result of a single visual test comparison.
  /// Maps to the JSON output from nsVisualTestRunner::ExportResultsJSON.
  /// </summary>
  public class T3VisualTestResult
  {
    [JsonProperty("testName")]
    public string TestName { get; set; } = "";

    [JsonProperty("api")]
    public string API { get; set; } = "";

    [JsonProperty("passed")]
    public bool Passed { get; set; }

    [JsonProperty("renderSucceeded")]
    public bool RenderSucceeded { get; set; }

    [JsonProperty("referenceExists")]
    public bool ReferenceExists { get; set; }

    [JsonProperty("renderTimeMs")]
    public double RenderTimeMs { get; set; }

    [JsonProperty("meanError")]
    public double MeanError { get; set; }

    [JsonProperty("maxError")]
    public double MaxError { get; set; }

    [JsonProperty("medianError")]
    public double MedianError { get; set; }

    [JsonProperty("p95Error")]
    public double P95Error { get; set; }

    [JsonProperty("pixelsFailed")]
    public uint PixelsFailed { get; set; }

    [JsonProperty("totalPixels")]
    public uint TotalPixels { get; set; }

    [JsonProperty("failurePercentage")]
    public double FailurePercentage { get; set; }

    [JsonProperty("error")]
    public string ErrorMessage { get; set; }

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
    [JsonProperty("totalTests")]
    public uint TotalTests { get; set; }

    [JsonProperty("passed")]
    public uint PassedCount { get; set; }

    [JsonProperty("failed")]
    public uint FailedCount { get; set; }

    [JsonProperty("skipped")]
    public uint SkippedCount { get; set; }

    [JsonProperty("newBaselines")]
    public uint NewBaselines { get; set; }

    [JsonProperty("totalTimeMs")]
    public double TotalTimeMs { get; set; }

    [JsonProperty("results")]
    public List<T3VisualTestResult> Results { get; set; } = new List<T3VisualTestResult>();

    /// <summary>
    /// Parse from JSON file exported by the C++ test runner.
    /// </summary>
    public static T3VisualTestSummary LoadFromJson(string path)
    {
      try
      {
        string json = System.IO.File.ReadAllText(path);
        return JsonConvert.DeserializeObject<T3VisualTestSummary>(json);
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
