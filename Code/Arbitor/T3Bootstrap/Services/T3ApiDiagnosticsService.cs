/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Linq;
using T3.Models;

namespace T3.Services
{
  /// <summary>
  /// Builds API-call diagnostics and recommendations from native test output.
  /// </summary>
  public static class T3ApiDiagnosticsService
  {
    public static IReadOnlyList<T3ApiCallCheck> BuildChecks(T3VisualTestResult result)
    {
      var checks = new List<T3ApiCallCheck>();

      if (result.ApiCallChecks.Count > 0)
      {
        checks.AddRange(result.ApiCallChecks
          .Where(check => !string.IsNullOrWhiteSpace(check.CallName))
          .Select(Normalize));
      }
      else
      {
        checks.Add(new T3ApiCallCheck
        {
          Category = "Instrumentation",
          CallName = "apiCallChecks",
          Passed = false,
          Severity = "Warning",
          Message = "This result did not include per-call API diagnostics.",
          Recommendation = "Instrument the native test with apiCallChecks so Mannequin can validate resource creation, frame flow, render passes, draw calls, and readback."
        });
      }

      checks.Add(BuildRenderContractCheck(result));
      checks.AddRange(BuildValidationChecks(result));
      checks.AddRange(BuildResourceStateChecks(result));

      if (result.RenderSucceeded && !result.ReferenceExists)
      {
        checks.Add(new T3ApiCallCheck
        {
          Category = "Baseline",
          CallName = "ReferenceImage",
          Passed = false,
          Severity = "Warning",
          Message = "The run created a new baseline because no reference image existed.",
          Recommendation = "Visually review the rendered output and commit the baseline only after confirming it is correct for this API."
        });
      }

      if (result.RenderSucceeded && result.FailurePercentage > 0.0)
      {
        checks.Add(new T3ApiCallCheck
        {
          Category = "Comparison",
          CallName = "PixelErrorBudget",
          Passed = result.Passed,
          Severity = result.Passed ? "Info" : "Error",
          Message = $"{result.FailurePercentage:F2}% of pixels exceeded the comparison threshold.",
          Recommendation = "Inspect the red error-pixel overlay, then verify shader constants, render states, viewport/scissor, resource formats, and color-space conversions."
        });
      }

      if (result.RenderTimeMs > 33.3)
      {
        checks.Add(new T3ApiCallCheck
        {
          Category = "Performance",
          CallName = "RenderTime",
          Passed = false,
          Severity = "Warning",
          Message = $"Render took {result.RenderTimeMs:F1} ms.",
          Recommendation = "Profile command submission, resource readback, synchronization fences, and backend fallback paths before using this as a performance baseline."
        });
      }

      return checks;
    }

    private static IEnumerable<T3ApiCallCheck> BuildValidationChecks(T3VisualTestResult result)
    {
      foreach (var validation in result.ValidationMessages)
      {
        string severity = NormalizeValidationSeverity(validation.Severity);
        yield return new T3ApiCallCheck
        {
          Category = "Validation",
          CallName = string.IsNullOrWhiteSpace(validation.Source) ? result.API : validation.Source,
          Passed = string.Equals(severity, "Info", StringComparison.OrdinalIgnoreCase),
          Severity = severity,
          Message = validation.Message ?? "",
          Recommendation = validation.Recommendation ?? ""
        };
      }
    }

    private static IEnumerable<T3ApiCallCheck> BuildResourceStateChecks(T3VisualTestResult result)
    {
      foreach (var snapshot in result.ResourceSnapshots)
      {
        foreach (var check in snapshot.StateChecks)
        {
          yield return new T3ApiCallCheck
          {
            Category = $"State/{snapshot.Name}",
            CallName = string.IsNullOrWhiteSpace(check.Name) ? snapshot.Type : check.Name,
            Passed = check.Passed,
            Severity = string.IsNullOrWhiteSpace(check.Severity)
              ? (check.Passed ? "Info" : "Error")
              : NormalizeValidationSeverity(check.Severity),
            Message = string.IsNullOrWhiteSpace(check.Message)
              ? $"Actual: {check.Actual}; expected: {check.Expected}"
              : check.Message,
            Recommendation = check.Recommendation ?? ""
          };
        }
      }
    }

    private static T3ApiCallCheck BuildRenderContractCheck(T3VisualTestResult result)
    {
      if (result.RenderSucceeded)
      {
        return new T3ApiCallCheck
        {
          Category = "Render Contract",
          CallName = "RenderCallback",
          Passed = true,
          Severity = "Info",
          Message = "The render callback produced an image for comparison.",
          Recommendation = "Keep this check green while expanding the test to cover more backend calls."
        };
      }

      return new T3ApiCallCheck
      {
        Category = "Render Contract",
        CallName = "RenderCallback",
        Passed = false,
        Severity = "Error",
        Message = string.IsNullOrWhiteSpace(result.ErrorMessage)
          ? "The render callback failed or produced no image."
          : result.ErrorMessage!,
        Recommendation = "Start with BeginFrame, resource creation, render pass setup, EndFrame, and readback diagnostics for the selected API."
      };
    }

    private static T3ApiCallCheck Normalize(T3ApiCallCheck check)
    {
      return new T3ApiCallCheck
      {
        Category = string.IsNullOrWhiteSpace(check.Category) ? "API" : check.Category,
        CallName = check.CallName,
        Passed = check.Passed,
        Severity = string.IsNullOrWhiteSpace(check.Severity)
          ? (check.Passed ? "Info" : "Error")
          : NormalizeValidationSeverity(check.Severity),
        Message = check.Message ?? "",
        Recommendation = check.Recommendation ?? ""
      };
    }

    private static string NormalizeValidationSeverity(string? severity)
    {
      if (string.Equals(severity, "Warning", StringComparison.OrdinalIgnoreCase))
        return "Warning";

      if (string.Equals(severity, "Error", StringComparison.OrdinalIgnoreCase) ||
          string.Equals(severity, "Fatal", StringComparison.OrdinalIgnoreCase))
        return "Fatal";

      return string.IsNullOrWhiteSpace(severity) ? "Info" : severity!;
    }
  }
}
