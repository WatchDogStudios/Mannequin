/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Threading;
using System.Threading.Tasks;
using T3Foundation.Models;

namespace T3Foundation.Services
{
  /// <summary>
  /// Service for running visual tests by invoking the C++ test runner executable
  /// and parsing its JSON output. This bridges the C# GUI with the native testing pipeline.
  /// </summary>
  public class T3TestRunnerService
  {
    private string _testRunnerPath = "";
    private string _referenceImageDir = "";
    private string _outputDir = "";
    private Process _runningProcess;

    /// <summary>
    /// Event raised when a test run completes.
    /// </summary>
    public event Action<T3VisualTestSummary> OnTestRunComplete;

    /// <summary>
    /// Event raised with real-time stdout/stderr output from the test runner.
    /// </summary>
    public event Action<string> OnTestOutput;

    /// <summary>
    /// Event raised when test run progress updates (0.0 to 1.0).
    /// </summary>
    public event Action<double, string> OnProgressUpdate;

    /// <summary>
    /// Whether a test run is currently active.
    /// </summary>
    public bool IsRunning => _runningProcess != null && !_runningProcess.HasExited;

    /// <summary>
    /// Configure the test runner paths.
    /// </summary>
    public void Configure(string testRunnerExe, string referenceDir, string outputDir)
    {
      _testRunnerPath = testRunnerExe;
      _referenceImageDir = referenceDir;
      _outputDir = outputDir;
    }

    /// <summary>
    /// Run visual tests for specified APIs asynchronously.
    /// </summary>
    public async Task<T3VisualTestSummary> RunTestsAsync(
      IEnumerable<string> apis,
      string filter = null,
      bool updateBaselines = false,
      CancellationToken cancellationToken = default)
    {
      if (string.IsNullOrEmpty(_testRunnerPath) || !File.Exists(_testRunnerPath))
      {
        T3Core.Log($"Test runner not found at: {_testRunnerPath}", T3LogLevel.Error);
        return null;
      }

      // Build command line arguments
      var args = new List<string>();

      foreach (var api in apis)
        args.Add($"--api {api}");

      if (!string.IsNullOrEmpty(_referenceImageDir))
        args.Add($"--reference-dir \"{_referenceImageDir}\"");

      if (!string.IsNullOrEmpty(_outputDir))
        args.Add($"--output-dir \"{_outputDir}\"");

      if (!string.IsNullOrEmpty(filter))
        args.Add($"--filter \"{filter}\"");

      if (updateBaselines)
        args.Add("--update-baselines");

      args.Add("--json-output");

      string jsonOutputPath = Path.Combine(_outputDir, "results.json");
      args.Add($"--json-path \"{jsonOutputPath}\"");

      T3Core.Log($"Starting test runner: {_testRunnerPath} {string.Join(" ", args)}", T3LogLevel.Info);

      var startInfo = new ProcessStartInfo
      {
        FileName = _testRunnerPath,
        Arguments = string.Join(" ", args),
        UseShellExecute = false,
        RedirectStandardOutput = true,
        RedirectStandardError = true,
        CreateNoWindow = true
      };

      try
      {
        _runningProcess = Process.Start(startInfo);
        if (_runningProcess == null)
        {
          T3Core.Log("Failed to start test runner process.", T3LogLevel.Error);
          return null;
        }

        // Stream output
        _runningProcess.OutputDataReceived += (s, e) =>
        {
          if (e.Data != null)
          {
            OnTestOutput?.Invoke(e.Data);
            T3Core.Log(e.Data, T3LogLevel.Debug);
          }
        };
        _runningProcess.ErrorDataReceived += (s, e) =>
        {
          if (e.Data != null)
          {
            OnTestOutput?.Invoke($"[ERR] {e.Data}");
            T3Core.Log(e.Data, T3LogLevel.Warning);
          }
        };

        _runningProcess.BeginOutputReadLine();
        _runningProcess.BeginErrorReadLine();

        await Task.Run(() => _runningProcess.WaitForExit(), cancellationToken);

        T3Core.Log($"Test runner exited with code {_runningProcess.ExitCode}", T3LogLevel.Info);

        // Parse results
        if (File.Exists(jsonOutputPath))
        {
          var summary = T3VisualTestSummary.LoadFromJson(jsonOutputPath);
          if (summary != null)
            OnTestRunComplete?.Invoke(summary);
          return summary;
        }

        T3Core.Log("No JSON results file found.", T3LogLevel.Warning);
        return null;
      }
      catch (OperationCanceledException)
      {
        AbortTests();
        T3Core.Log("Test run cancelled by user.", T3LogLevel.Warning);
        return null;
      }
      catch (Exception ex)
      {
        T3Core.Log($"Test runner error: {ex.Message}", T3LogLevel.Error);
        return null;
      }
      finally
      {
        _runningProcess = null;
      }
    }

    /// <summary>
    /// Abort a currently running test.
    /// </summary>
    public void AbortTests()
    {
      if (_runningProcess != null && !_runningProcess.HasExited)
      {
        try
        {
          _runningProcess.Kill();
          T3Core.Log("Test run aborted.", T3LogLevel.Warning);
        }
        catch (Exception ex)
        {
          T3Core.Log($"Error aborting tests: {ex.Message}", T3LogLevel.Error);
        }
      }
    }

    /// <summary>
    /// Get paths to comparison images for a specific test.
    /// </summary>
    public (string TestImage, string ReferenceImage, string DiffImage) GetComparisonImages(
      string apiName, string testName)
    {
      string basePath = Path.Combine(_outputDir, apiName, testName);
      string testImg = Path.Combine(basePath, $"{testName}_test.png");
      string refImg = Path.Combine(basePath, $"{testName}_reference.png");
      string diffImg = Path.Combine(basePath, $"{testName}_diff.png");

      return (
        File.Exists(testImg) ? testImg : null,
        File.Exists(refImg) ? refImg : null,
        File.Exists(diffImg) ? diffImg : null
      );
    }
  }
}
