/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using System;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using T3.Models;
using T3Foundation;

namespace T3.Services
{
  /// <summary>
  /// Export test results to HTML or CSV formats.
  /// </summary>
  public class T3ExportService
  {
    /// <summary>
    /// Export test results as a self-contained HTML report.
    /// </summary>
    public async Task ExportToHtmlAsync(T3VisualTestSummary summary, string outputPath)
    {
      var sb = new StringBuilder();
      sb.AppendLine("<!DOCTYPE html>");
      sb.AppendLine("<html><head>");
      sb.AppendLine("<meta charset='utf-8'/>");
      sb.AppendLine("<title>Mannequin Test Report</title>");
      sb.AppendLine("<style>");
      sb.AppendLine("body { font-family: 'Segoe UI', sans-serif; background: #1e1e1e; color: #d4d4d4; margin: 20px; }");
      sb.AppendLine("h1 { color: #569cd6; border-bottom: 2px solid #333; padding-bottom: 8px; }");
      sb.AppendLine("h2 { color: #4ec9b0; margin-top: 24px; }");
      sb.AppendLine(".summary { display: flex; gap: 24px; flex-wrap: wrap; margin: 16px 0; }");
      sb.AppendLine(".stat { background: #252526; padding: 12px 20px; border-radius: 6px; border: 1px solid #333; }");
      sb.AppendLine(".stat .value { font-size: 24px; font-weight: bold; }");
      sb.AppendLine(".stat .label { font-size: 12px; color: #888; margin-top: 4px; }");
      sb.AppendLine(".pass { color: #4caf50; }");
      sb.AppendLine(".fail { color: #f44336; }");
      sb.AppendLine(".warn { color: #d7a833; }");
      sb.AppendLine(".skip { color: #888; }");
      sb.AppendLine("table { border-collapse: collapse; width: 100%; margin-top: 16px; }");
      sb.AppendLine("th, td { padding: 8px 12px; text-align: left; border-bottom: 1px solid #333; }");
      sb.AppendLine("th { background: #252526; color: #569cd6; font-weight: 600; }");
      sb.AppendLine("tr:hover { background: #2a2d2e; }");
      sb.AppendLine(".status-pass { background: #1b3a1b; }");
      sb.AppendLine(".status-fail { background: #3a1b1b; }");
      sb.AppendLine("</style>");
      sb.AppendLine("</head><body>");

      sb.AppendLine("<h1>Mannequin Visual Test Report</h1>");
      sb.AppendLine($"<p>Generated: {DateTime.Now:yyyy-MM-dd HH:mm:ss}</p>");

      // Summary stats
      sb.AppendLine("<div class='summary'>");
      sb.AppendLine($"<div class='stat'><div class='value'>{summary.TotalTests}</div><div class='label'>Total Tests</div></div>");
      sb.AppendLine($"<div class='stat'><div class='value pass'>{summary.PassedCount}</div><div class='label'>Passed</div></div>");
      sb.AppendLine($"<div class='stat'><div class='value fail'>{summary.FailedCount}</div><div class='label'>Failed</div></div>");
      sb.AppendLine($"<div class='stat'><div class='value skip'>{summary.SkippedCount}</div><div class='label'>Skipped</div></div>");
      sb.AppendLine($"<div class='stat'><div class='value'>{summary.NewBaselines}</div><div class='label'>New Baselines</div></div>");
      sb.AppendLine($"<div class='stat'><div class='value'>{summary.TotalTimeMs:F0} ms</div><div class='label'>Total Time</div></div>");
      sb.AppendLine("</div>");

      // Results table
      sb.AppendLine("<h2>Test Results</h2>");
      sb.AppendLine("<table>");
      sb.AppendLine("<tr><th>Test Name</th><th>API</th><th>Status</th><th>Render Time</th><th>Mean Error</th><th>Max Error</th><th>Pixels Failed</th><th>Failure %</th><th>API Checks</th></tr>");

      foreach (var r in summary.Results.OrderBy(r => r.Passed).ThenBy(r => r.TestName))
      {
        string rowClass = r.Passed ? "status-pass" : "status-fail";
        string statusText = r.Passed ? "<span class='pass'>PASSED</span>" : "<span class='fail'>FAILED</span>";
        var diagnostics = T3ApiDiagnosticsService.BuildChecks(r);
        int fatalChecks = diagnostics.Count(check => string.Equals(check.StatusText, "FATAL", StringComparison.OrdinalIgnoreCase));
        int failedChecks = diagnostics.Count(check => string.Equals(check.StatusText, "FAIL", StringComparison.OrdinalIgnoreCase));
        int warningChecks = diagnostics.Count(check => string.Equals(check.StatusText, "WARN", StringComparison.OrdinalIgnoreCase));
        string checksText = fatalChecks + failedChecks > 0
          ? $"<span class='fail'>{fatalChecks + failedChecks} fatal</span>, <span class='warn'>{warningChecks} warn</span>"
          : warningChecks > 0
            ? $"<span class='warn'>{warningChecks} warn</span>"
            : "<span class='pass'>clean</span>";

        sb.AppendLine($"<tr class='{rowClass}'>");
        sb.AppendLine($"<td>{Escape(r.TestName)}</td>");
        sb.AppendLine($"<td>{Escape(r.API)}</td>");
        sb.AppendLine($"<td>{statusText}</td>");
        sb.AppendLine($"<td>{r.RenderTimeMs:F1} ms</td>");
        sb.AppendLine($"<td>{r.MeanError:F6}</td>");
        sb.AppendLine($"<td>{r.MaxError:F6}</td>");
        sb.AppendLine($"<td>{r.PixelsFailed} / {r.TotalPixels}</td>");
        sb.AppendLine($"<td>{r.FailurePercentage:F2}%</td>");
        sb.AppendLine($"<td>{checksText}</td>");
        sb.AppendLine("</tr>");
      }

      sb.AppendLine("</table>");

      sb.AppendLine("<h2>API Call Diagnostics</h2>");
      sb.AppendLine("<table>");
      sb.AppendLine("<tr><th>Test</th><th>API</th><th>Call</th><th>Status</th><th>Message</th><th>Recommendation</th></tr>");

      foreach (var r in summary.Results.OrderBy(r => r.TestName).ThenBy(r => r.API))
      {
        foreach (var check in T3ApiDiagnosticsService.BuildChecks(r))
        {
          string statusClass = check.StatusText == "PASS" ? "pass" : (check.StatusText == "WARN" ? "warn" : "fail");
          sb.AppendLine("<tr>");
          sb.AppendLine($"<td>{Escape(r.TestName)}</td>");
          sb.AppendLine($"<td>{Escape(r.API)}</td>");
          sb.AppendLine($"<td>{Escape(check.Category)} / {Escape(check.CallName)}</td>");
          sb.AppendLine($"<td><span class='{statusClass}'>{Escape(check.StatusText)}</span></td>");
          sb.AppendLine($"<td>{Escape(check.Message)}</td>");
          sb.AppendLine($"<td>{Escape(check.Recommendation)}</td>");
          sb.AppendLine("</tr>");
        }
      }

      sb.AppendLine("</table>");

      sb.AppendLine("<h2>Resource Inspector</h2>");
      sb.AppendLine("<table>");
      sb.AppendLine("<tr><th>Test</th><th>API</th><th>Resource</th><th>Type</th><th>Slot</th><th>Format</th><th>Dimensions</th><th>State</th><th>Checks</th><th>Values</th></tr>");

      foreach (var r in summary.Results.OrderBy(r => r.TestName).ThenBy(r => r.API))
      {
        foreach (var resource in r.ResourceSnapshots)
        {
          int fatal = resource.StateChecks.Count(check => string.Equals(check.StatusText, "FATAL", StringComparison.OrdinalIgnoreCase));
          int warn = resource.StateChecks.Count(check => string.Equals(check.StatusText, "WARN", StringComparison.OrdinalIgnoreCase));
          string checks = fatal > 0
            ? $"<span class='fail'>{fatal} fatal</span>, <span class='warn'>{warn} warn</span>"
            : warn > 0
              ? $"<span class='warn'>{warn} warn</span>"
              : "<span class='pass'>clean</span>";
          string dimensions = resource.Width > 0 || resource.Height > 0 || resource.Depth > 0
            ? $"{resource.Width} x {resource.Height} x {Math.Max(1, resource.Depth)}"
            : "";

          sb.AppendLine("<tr>");
          sb.AppendLine($"<td>{Escape(r.TestName)}</td>");
          sb.AppendLine($"<td>{Escape(r.API)}</td>");
          sb.AppendLine($"<td>{Escape(resource.Name)}</td>");
          sb.AppendLine($"<td>{Escape(resource.Type)}</td>");
          sb.AppendLine($"<td>{Escape(resource.Slot)}</td>");
          sb.AppendLine($"<td>{Escape(resource.Format)}</td>");
          sb.AppendLine($"<td>{Escape(dimensions)}</td>");
          sb.AppendLine($"<td>{Escape(resource.State)}</td>");
          sb.AppendLine($"<td>{checks}</td>");
          sb.AppendLine($"<td>{Escape(string.Join("; ", resource.Values))}</td>");
          sb.AppendLine("</tr>");
        }
      }

      sb.AppendLine("</table>");
      sb.AppendLine("</body></html>");

      await Task.Run(() => File.WriteAllText(outputPath, sb.ToString()));
      T3Core.Log($"HTML report exported to {outputPath}", T3LogLevel.Info);
    }

    /// <summary>
    /// Export test results as CSV.
    /// </summary>
    public async Task ExportToCsvAsync(T3VisualTestSummary summary, string outputPath)
    {
      var sb = new StringBuilder();
      sb.AppendLine("TestName,API,Passed,RenderTimeMs,MeanError,MaxError,MedianError,P95Error,PixelsFailed,TotalPixels,FailurePercentage,ApiDiagnostics,Recommendations,ResourceSnapshots,Error");

      foreach (var r in summary.Results)
      {
        var checks = T3ApiDiagnosticsService.BuildChecks(r);
        string diagnostics = string.Join(" | ", checks.Select(check => $"{check.StatusText}: {check.Category}/{check.CallName} - {check.Message}"));
        string recommendations = string.Join(" | ", checks
          .Where(check => !string.IsNullOrWhiteSpace(check.Recommendation))
          .Select(check => $"{check.Category}/{check.CallName}: {check.Recommendation}"));
        string resourceSnapshots = string.Join(" | ", r.ResourceSnapshots.Select(resource =>
          $"{resource.Name} ({resource.Type}, {resource.Slot}, {resource.Format}, {resource.Width}x{resource.Height}, {resource.State}): {string.Join("; ", resource.Values)}"));
        sb.AppendLine($"{CsvEscape(r.TestName)},{CsvEscape(r.API)},{r.Passed},{r.RenderTimeMs:F1},{r.MeanError:F6},{r.MaxError:F6},{r.MedianError:F6},{r.P95Error:F6},{r.PixelsFailed},{r.TotalPixels},{r.FailurePercentage:F2},{CsvEscape(diagnostics)},{CsvEscape(recommendations)},{CsvEscape(resourceSnapshots)},{CsvEscape(r.ErrorMessage ?? "")}");
      }

      await Task.Run(() => File.WriteAllText(outputPath, sb.ToString()));
      T3Core.Log($"CSV report exported to {outputPath}", T3LogLevel.Info);
    }

    private static string Escape(string s) =>
      s.Replace("&", "&amp;").Replace("<", "&lt;").Replace(">", "&gt;");

    private static string CsvEscape(string s) =>
      s.Contains(',') || s.Contains('"') || s.Contains('\n')
        ? $"\"{s.Replace("\"", "\"\"")}\""
        : s;
  }
}
