/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.IO;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using T3Foundation;

namespace T3.Models
{
  /// <summary>
  /// Diagnostic result for a graphics API call, call sequence, or renderer contract.
  /// </summary>
  public class T3ApiCallCheck
  {
    [JsonProperty("callName")]
    public string CallName { get; set; } = "";

    [JsonProperty("category")]
    public string Category { get; set; } = "";

    [JsonProperty("passed")]
    public bool Passed { get; set; }

    [JsonProperty("severity")]
    public string Severity { get; set; } = "Info";

    [JsonProperty("message")]
    public string Message { get; set; } = "";

    [JsonProperty("recommendation")]
    public string Recommendation { get; set; } = "";

    [JsonIgnore]
    public string StatusText => Passed ? "PASS" :
      (string.Equals(Severity, "Warning", StringComparison.OrdinalIgnoreCase) ? "WARN" :
      (string.Equals(Severity, "Error", StringComparison.OrdinalIgnoreCase) ||
       string.Equals(Severity, "Fatal", StringComparison.OrdinalIgnoreCase) ? "FATAL" : "FAIL"));
  }

  /// <summary>
  /// Validation layer message captured from a backend.
  /// </summary>
  public class T3ValidationMessage
  {
    [JsonProperty("source")]
    public string Source { get; set; } = "";

    [JsonProperty("severity")]
    public string Severity { get; set; } = "Info";

    [JsonProperty("message")]
    public string Message { get; set; } = "";

    [JsonProperty("recommendation")]
    public string Recommendation { get; set; } = "";

    [JsonIgnore]
    public string StatusText => string.Equals(Severity, "Warning", StringComparison.OrdinalIgnoreCase) ? "WARN" :
      (string.Equals(Severity, "Error", StringComparison.OrdinalIgnoreCase) ||
       string.Equals(Severity, "Fatal", StringComparison.OrdinalIgnoreCase) ? "FATAL" : "INFO");
  }

  /// <summary>
  /// Assertion against an internal renderer resource or state object.
  /// </summary>
  public class T3ResourceStateCheck
  {
    [JsonProperty("name")]
    public string Name { get; set; } = "";

    [JsonProperty("passed")]
    public bool Passed { get; set; }

    [JsonProperty("severity")]
    public string Severity { get; set; } = "Info";

    [JsonProperty("actual")]
    public string Actual { get; set; } = "";

    [JsonProperty("expected")]
    public string Expected { get; set; } = "";

    [JsonProperty("message")]
    public string Message { get; set; } = "";

    [JsonProperty("recommendation")]
    public string Recommendation { get; set; } = "";

    [JsonIgnore]
    public string StatusText => Passed ? "PASS" :
      (string.Equals(Severity, "Warning", StringComparison.OrdinalIgnoreCase) ? "WARN" :
      (string.Equals(Severity, "Error", StringComparison.OrdinalIgnoreCase) ||
       string.Equals(Severity, "Fatal", StringComparison.OrdinalIgnoreCase) ? "FATAL" : "FAIL"));
  }

  /// <summary>
  /// Captured RHI/shader resource snapshot that can be inspected alongside image results.
  /// </summary>
  public class T3ResourceSnapshot
  {
    [JsonProperty("name")]
    public string Name { get; set; } = "";

    [JsonProperty("type")]
    public string Type { get; set; } = "";

    [JsonProperty("slot")]
    public string Slot { get; set; } = "";

    [JsonProperty("format")]
    public string Format { get; set; } = "";

    [JsonProperty("state")]
    public string State { get; set; } = "";

    [JsonProperty("summary")]
    public string Summary { get; set; } = "";

    [JsonProperty("previewPath")]
    public string PreviewPath { get; set; } = "";

    [JsonProperty("width")]
    public uint Width { get; set; }

    [JsonProperty("height")]
    public uint Height { get; set; }

    [JsonProperty("depth")]
    public uint Depth { get; set; }

    [JsonProperty("mipLevels")]
    public uint MipLevels { get; set; }

    [JsonProperty("elementCount")]
    public uint ElementCount { get; set; }

    [JsonProperty("rowPitch")]
    public uint RowPitch { get; set; }

    [JsonProperty("byteSize")]
    public ulong ByteSize { get; set; }

    [JsonProperty("values")]
    public List<string> Values { get; set; } = new List<string>();

    [JsonProperty("stateChecks")]
    public List<T3ResourceStateCheck> StateChecks { get; set; } = new List<T3ResourceStateCheck>();
  }

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
    public string? ErrorMessage { get; set; }

    [JsonProperty("apiCallChecks")]
    public List<T3ApiCallCheck> ApiCallChecks { get; set; } = new List<T3ApiCallCheck>();

    [JsonProperty("validationMessages")]
    public List<T3ValidationMessage> ValidationMessages { get; set; } = new List<T3ValidationMessage>();

    [JsonProperty("resourceSnapshots")]
    public List<T3ResourceSnapshot> ResourceSnapshots { get; set; } = new List<T3ResourceSnapshot>();

    /// <summary>
    /// Status string for UI display.
    /// </summary>
    [JsonIgnore]
    public string StatusText => string.Equals(ErrorMessage, "Skipped", StringComparison.OrdinalIgnoreCase)
      ? "SKIPPED"
      : (Passed ? "PASSED" : (RenderSucceeded ? "FAILED" : "ERROR"));
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
    public static T3VisualTestSummary? LoadFromJson(string path)
    {
      try
      {
        string json = System.IO.File.ReadAllText(path);
        var visualSummary = JsonConvert.DeserializeObject<T3VisualTestSummary>(json);
        if (visualSummary?.Results.Count > 0)
          return visualSummary;

        return LoadFromTestFrameworkJson(json) ?? visualSummary;
      }
      catch (Exception ex)
      {
        T3Core.Log($"Failed to load test results: {ex.Message}", T3LogLevel.Error);
        return null;
      }
    }

    private static T3VisualTestSummary? LoadFromTestFrameworkJson(string json)
    {
      var root = JObject.Parse(json);
      if (root["tests"] is not JArray tests)
        return null;

      var summary = new T3VisualTestSummary();

      foreach (var testToken in tests)
      {
        string testName = testToken.Value<string>("m_sName") ?? "";
        if (string.IsNullOrWhiteSpace(testName))
          continue;

        bool executed = testToken.Value<bool?>("m_bExecuted") ?? false;
        bool passed = testToken.Value<bool?>("m_bSuccess") ?? false;
        double durationMs = (testToken.Value<double?>("m_fTestDuration") ?? 0.0) * 1000.0;

        summary.Results.Add(new T3VisualTestResult
        {
          TestName = testName,
          API = "",
          Passed = executed && passed,
          RenderSucceeded = executed,
          ReferenceExists = true,
          RenderTimeMs = durationMs,
          ErrorMessage = executed ? null : "Skipped"
        });

        summary.TotalTests++;

        if (!executed)
          summary.SkippedCount++;
        else if (passed)
          summary.PassedCount++;
        else
          summary.FailedCount++;

        summary.TotalTimeMs += durationMs;
      }

      return summary;
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

  /// <summary>
  /// Represents a native test application Mannequin can launch.
  /// </summary>
  public class T3TestApplication
  {
    public string DisplayName { get; set; } = "";
    public string ExecutablePath { get; set; } = "";

    [JsonIgnore]
    public bool Exists => File.Exists(ExecutablePath);

    public override string ToString() => DisplayName;
  }
}
