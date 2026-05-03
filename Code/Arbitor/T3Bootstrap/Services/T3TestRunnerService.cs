/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using T3.Models;
using T3Foundation;
using T3Foundation.Services.Settings;

namespace T3.Services
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
    private string? _lastResultsDirectory;
    private Process? _runningProcess;
    private readonly IT3SettingsService? _settings;

    /// <summary>
    /// Event raised when a test run completes.
    /// </summary>
    public event Action<T3VisualTestSummary>? OnTestRunComplete;

    /// <summary>
    /// Event raised with real-time stdout/stderr output from the test runner.
    /// </summary>
    public event Action<string>? OnTestOutput;

    /// <summary>
    /// Event raised when test run progress updates (0.0 to 1.0).
    /// </summary>
    public event Action<double, string>? OnProgressUpdate;

    /// <summary>
    /// Whether a test run is currently active.
    /// </summary>
    public bool IsRunning => _runningProcess != null && !_runningProcess.HasExited;

    public string TestRunnerPath => ResolvePath(_testRunnerPath);

    public string OutputDir => ResolvePath(_outputDir);

    public T3TestRunnerService() { }

    public T3TestRunnerService(IT3SettingsService settings)
    {
      _settings = settings;
      LoadFromSettings();
    }

    /// <summary>
    /// Load configuration from the settings service.
    /// </summary>
    public void LoadFromSettings()
    {
      if (_settings == null) return;
      _testRunnerPath = _settings.Get("TestRunnerPath", "RendererTest.exe");
      _referenceImageDir = _settings.Get("ReferenceImageDir", "Data/UnitTests/RendererTest/ReferenceImages");
      _outputDir = _settings.Get("OutputDir", "TestOutput");
    }

    /// <summary>
    /// Configure the test runner paths manually.
    /// </summary>
    public void Configure(string testRunnerExe, string referenceDir, string outputDir)
    {
      _testRunnerPath = testRunnerExe;
      _referenceImageDir = referenceDir;
      _outputDir = outputDir;
    }

    public void SetTestRunnerPath(string testRunnerExe)
    {
      _testRunnerPath = testRunnerExe;
    }

    public void SetResultsDirectory(string? resultsDirectory)
    {
      _lastResultsDirectory = string.IsNullOrWhiteSpace(resultsDirectory)
        ? null
        : ResolvePath(resultsDirectory!);
    }

    public IReadOnlyList<T3TestApplication> DiscoverApplications()
    {
      var apps = new List<T3TestApplication>();
      var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);

      void AddApplication(string executablePath, string? displayName = null, bool includeMissing = false)
      {
        if (string.IsNullOrWhiteSpace(executablePath))
          return;

        string fullPath = ResolvePath(executablePath);
        bool exists = File.Exists(fullPath);
        if (!exists && !includeMissing)
          return;

        if (!seen.Add(fullPath))
          return;

        string name = displayName ?? Path.GetFileNameWithoutExtension(fullPath);
        apps.Add(new T3TestApplication
        {
          DisplayName = exists ? name : $"{name} (not built)",
          ExecutablePath = fullPath
        });
      }

      AddApplication(_testRunnerPath);

      foreach (string directory in GetCandidateBinaryDirectories())
      {
        if (!Directory.Exists(directory))
          continue;

        foreach (string exePath in Directory.EnumerateFiles(directory, "*.exe", SearchOption.TopDirectoryOnly))
        {
          string name = Path.GetFileNameWithoutExtension(exePath);
          if (string.Equals(name, "T3Bootstrap", StringComparison.OrdinalIgnoreCase))
            continue;

          AddApplication(exePath, name);
        }
      }

      foreach (string sourceApp in DiscoverSourceTestApplications())
      {
        string candidatePath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, $"{sourceApp}.exe");
        AddApplication(candidatePath, sourceApp, includeMissing: true);
      }

      return apps
        .OrderByDescending(a => a.Exists)
        .ThenBy(a => a.DisplayName, StringComparer.OrdinalIgnoreCase)
        .ToArray();
    }

    public IReadOnlyList<T3VisualTestResult> DiscoverTests()
    {
      var discovered = DiscoverTestsFromSource();
      if (discovered.Count > 0)
        return discovered;

      string resultsPath = Path.Combine(OutputDir, "results.json");
      var summary = File.Exists(resultsPath) ? T3VisualTestSummary.LoadFromJson(resultsPath) : null;
      return summary?.Results != null
        ? (IReadOnlyList<T3VisualTestResult>)summary.Results
        : Array.Empty<T3VisualTestResult>();
    }

    /// <summary>
    /// Run visual tests for specified APIs asynchronously.
    /// </summary>
    public async Task<T3VisualTestSummary?> RunTestsAsync(
      IEnumerable<string> apis,
      string? filter = null,
      bool updateBaselines = false,
      CancellationToken cancellationToken = default)
    {
      string resolvedTestRunnerPath = TestRunnerPath;
      string resolvedOutputDir = OutputDir;

      if (string.IsNullOrEmpty(resolvedTestRunnerPath) || !File.Exists(resolvedTestRunnerPath))
      {
        T3Core.Log($"Test runner not found at: {resolvedTestRunnerPath}", T3LogLevel.Error);
        return null;
      }

      Directory.CreateDirectory(resolvedOutputDir);

      string jsonOutputPath = Path.Combine(resolvedOutputDir, "results.json");
      var runStartedUtc = DateTime.UtcNow.AddSeconds(-2);
      var apiArray = apis.Where(api => !string.IsNullOrWhiteSpace(api)).ToArray();
      var args = BuildRunnerArguments(apiArray, filter, updateBaselines, jsonOutputPath, resolvedOutputDir);

      T3Core.Log($"Starting test runner: {resolvedTestRunnerPath} {string.Join(" ", args)}", T3LogLevel.Info);
      OnProgressUpdate?.Invoke(0.0, "Starting test run");

      var startInfo = new ProcessStartInfo
      {
        FileName = resolvedTestRunnerPath,
        Arguments = string.Join(" ", args),
        UseShellExecute = false,
        RedirectStandardOutput = true,
        RedirectStandardError = true,
        CreateNoWindow = true,
        WorkingDirectory = Path.GetDirectoryName(resolvedTestRunnerPath) ?? AppDomain.CurrentDomain.BaseDirectory
      };

      try
      {
        _runningProcess = Process.Start(startInfo);
        if (_runningProcess == null)
        {
          T3Core.Log("Failed to start test runner process.", T3LogLevel.Error);
          return null;
        }

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
        OnProgressUpdate?.Invoke(1.0, "Test run complete");

        string? resultsPath = FindResultsJson(jsonOutputPath, resolvedTestRunnerPath, runStartedUtc);
        if (resultsPath != null)
        {
          _lastResultsDirectory = Path.GetDirectoryName(resultsPath);
          var summary = T3VisualTestSummary.LoadFromJson(resultsPath);
          if (summary != null)
            ApplyApiToSummary(summary, apiArray);

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

    private List<string> BuildRunnerArguments(
      IReadOnlyList<string> apis,
      string? filter,
      bool updateBaselines,
      string jsonOutputPath,
      string outputDir)
    {
      var args = new List<string>
      {
        "-run",
        "-close",
        "-noGui",
        "-all",
        "-json",
        QuoteArg(jsonOutputPath),
        "-outputDir",
        QuoteArg(outputDir)
      };

      if (!string.IsNullOrWhiteSpace(filter))
      {
        args.Add("-filter");
        args.Add(QuoteArg(filter!));
      }

      if (apis.Count == 1 && !string.Equals(apis[0], "All APIs", StringComparison.OrdinalIgnoreCase))
      {
        args.Add("-renderer");
        args.Add(QuoteArg(apis[0]));
      }

      if (updateBaselines)
      {
        T3Core.Log("Baseline updates are only available from the native test UI right now.", T3LogLevel.Warning);
      }

      if (apis.Count > 0)
      {
        T3Core.Log($"Selected API filter: {string.Join(", ", apis)}", T3LogLevel.Info);
      }

      return args;
    }

    private static void ApplyApiToSummary(T3VisualTestSummary summary, IReadOnlyList<string> apis)
    {
      if (apis.Count != 1)
        return;

      foreach (var result in summary.Results)
      {
        if (string.IsNullOrWhiteSpace(result.API))
          result.API = apis[0];
      }
    }

    private string? FindResultsJson(string expectedPath, string testRunnerPath, DateTime runStartedUtc)
    {
      string appName = Path.GetFileNameWithoutExtension(testRunnerPath);
      string workingDirectory = Path.GetDirectoryName(testRunnerPath) ?? AppDomain.CurrentDomain.BaseDirectory;
      var candidates = new[]
      {
        expectedPath,
        Path.Combine(OutputDir, "results.json"),
        Path.Combine(workingDirectory, "TestOutput", "Samples", appName, "results.json"),
        Path.Combine(workingDirectory, "TestOutput", appName, "results.json")
      };

      foreach (string candidate in candidates.Distinct(StringComparer.OrdinalIgnoreCase))
      {
        if (File.Exists(candidate) && File.GetLastWriteTimeUtc(candidate) >= runStartedUtc)
          return candidate;
      }

      return null;
    }

    private List<T3VisualTestResult> DiscoverTestsFromSource()
    {
      string? sourceDirectory = GetSelectedApplicationSourceDirectory();
      if (sourceDirectory == null)
        return new List<T3VisualTestResult>();

      var names = new SortedSet<string>(StringComparer.OrdinalIgnoreCase);
      var allowedExtensions = new HashSet<string>(StringComparer.OrdinalIgnoreCase)
      {
        ".cpp",
        ".h",
        ".hpp",
        ".inl"
      };

      foreach (string file in Directory.EnumerateFiles(sourceDirectory, "*.*", SearchOption.AllDirectories))
      {
        if (!allowedExtensions.Contains(Path.GetExtension(file)))
          continue;

        string content;
        try
        {
          content = File.ReadAllText(file);
        }
        catch (IOException)
        {
          continue;
        }

        foreach (Match match in GetTestNameRegex.Matches(content))
          names.Add(match.Groups[1].Value);

        foreach (Match match in SimpleTestGroupRegex.Matches(content))
          names.Add(match.Groups[1].Value);

        foreach (Match match in VisualTestNameRegex.Matches(content))
          names.Add(match.Groups[1].Value);
      }

      return names
        .Select(name => new T3VisualTestResult
        {
          TestName = name,
          API = "",
          Passed = false,
          RenderSucceeded = false,
          ReferenceExists = true,
          ErrorMessage = "Not run yet"
        })
        .ToList();
    }

    private string? GetSelectedApplicationSourceDirectory()
    {
      string? codeRoot = FindCodeRoot();
      if (codeRoot == null)
        return null;

      string appName = Path.GetFileNameWithoutExtension(_testRunnerPath);
      if (string.IsNullOrWhiteSpace(appName))
        appName = Path.GetFileNameWithoutExtension(TestRunnerPath);

      foreach (string sourceRoot in GetApplicationSourceRoots(codeRoot))
      {
        string candidate = Path.Combine(sourceRoot, appName);
        if (Directory.Exists(candidate))
          return candidate;
      }

      return null;
    }

    private IEnumerable<string> DiscoverSourceTestApplications()
    {
      string? codeRoot = FindCodeRoot();
      if (codeRoot == null)
        yield break;

      foreach (string sourceRoot in GetApplicationSourceRoots(codeRoot))
      {
        if (!Directory.Exists(sourceRoot))
          continue;

        foreach (string directory in Directory.EnumerateDirectories(sourceRoot))
        {
          string cmakeFile = Path.Combine(directory, "CMakeLists.txt");
          if (!File.Exists(cmakeFile))
            continue;

          string cmakeContent;
          try
          {
            cmakeContent = File.ReadAllText(cmakeFile);
          }
          catch (IOException)
          {
            continue;
          }

          if (cmakeContent.IndexOf("ns_create_target(APPLICATION", StringComparison.OrdinalIgnoreCase) >= 0)
            yield return Path.GetFileName(directory);
        }
      }
    }

    private static IEnumerable<string> GetApplicationSourceRoots(string codeRoot)
    {
      yield return Path.Combine(codeRoot, "UnitTests");
      yield return Path.Combine(codeRoot, "Samples");
    }

    private IEnumerable<string> GetCandidateBinaryDirectories()
    {
      var directories = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
      string baseDirectory = AppDomain.CurrentDomain.BaseDirectory;
      directories.Add(baseDirectory);

      var current = new DirectoryInfo(baseDirectory);
      while (current != null)
      {
        if (string.Equals(current.Name, "Bin", StringComparison.OrdinalIgnoreCase))
        {
          foreach (string child in Directory.EnumerateDirectories(current.FullName))
            directories.Add(child);
          break;
        }

        current = current.Parent;
      }

      string? codeRoot = FindCodeRoot();
      if (codeRoot != null)
      {
        string binRoot = Path.Combine(codeRoot, "Output", "Bin");
        if (Directory.Exists(binRoot))
        {
          foreach (string child in Directory.EnumerateDirectories(binRoot))
            directories.Add(child);
        }
      }

      return directories;
    }

    private static string? FindCodeRoot()
    {
      var current = new DirectoryInfo(AppDomain.CurrentDomain.BaseDirectory);
      while (current != null)
      {
        if (Directory.Exists(Path.Combine(current.FullName, "UnitTests")) &&
            Directory.Exists(Path.Combine(current.FullName, "Arbitor")))
        {
          return current.FullName;
        }

        string codeCandidate = Path.Combine(current.FullName, "Code");
        if (Directory.Exists(Path.Combine(codeCandidate, "UnitTests")) &&
            Directory.Exists(Path.Combine(codeCandidate, "Arbitor")))
        {
          return codeCandidate;
        }

        current = current.Parent;
      }

      return null;
    }

    private static string ResolvePath(string path)
    {
      if (string.IsNullOrWhiteSpace(path))
        return "";

      if (Path.IsPathRooted(path))
        return Path.GetFullPath(path);

      return Path.GetFullPath(Path.Combine(AppDomain.CurrentDomain.BaseDirectory, path));
    }

    private static string QuoteArg(string value)
    {
      return $"\"{value.Replace("\"", "\\\"")}\"";
    }

    private static readonly Regex GetTestNameRegex = new Regex(
      @"GetTestName\(\)\s+const\s+override\s*\{[^}]*return\s+""([^""]+)""",
      RegexOptions.Compiled | RegexOptions.Multiline);

    private static readonly Regex SimpleTestGroupRegex = new Regex(
      @"NS_CREATE_SIMPLE_TEST_GROUP\s*\(\s*([A-Za-z_][A-Za-z0-9_]*)\s*\)",
      RegexOptions.Compiled);

    private static readonly Regex VisualTestNameRegex = new Regex(
      @"\bm_sTestName\s*=\s*""([^""]+)""",
      RegexOptions.Compiled);

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
    public (string? TestImage, string? ReferenceImage, string? DiffImage) GetComparisonImages(
      string apiName, string testName)
    {
      var candidateRoots = new[]
      {
        OutputDir,
        _lastResultsDirectory ?? ""
      };

      foreach (string root in candidateRoots.Where(root => !string.IsNullOrWhiteSpace(root)).Distinct(StringComparer.OrdinalIgnoreCase))
      {
        string basePath = Path.Combine(root, apiName, testName);
        string testImg = Path.Combine(basePath, $"{testName}_test.png");
        string refImg = Path.Combine(basePath, $"{testName}_reference.png");
        string diffImg = Path.Combine(basePath, $"{testName}_diff.png");

        if (File.Exists(testImg) || File.Exists(refImg) || File.Exists(diffImg))
        {
          return (
            File.Exists(testImg) ? testImg : null,
            File.Exists(refImg) ? refImg : null,
            File.Exists(diffImg) ? diffImg : null
          );
        }
      }

      return (null, null, null);
    }

    public string? ResolveArtifactPath(string apiName, string testName, string? artifactPath)
    {
      if (string.IsNullOrWhiteSpace(artifactPath))
        return null;

      if (Path.IsPathRooted(artifactPath) && File.Exists(artifactPath))
        return Path.GetFullPath(artifactPath);

      var candidateRoots = new[]
      {
        _lastResultsDirectory ?? "",
        OutputDir
      };

      foreach (string root in candidateRoots.Where(root => !string.IsNullOrWhiteSpace(root)).Distinct(StringComparer.OrdinalIgnoreCase))
      {
        var candidates = new[]
        {
          Path.Combine(root, artifactPath),
          Path.Combine(root, apiName, testName, artifactPath),
          Path.Combine(root, apiName, testName, Path.GetFileName(artifactPath))
        };

        foreach (string candidate in candidates)
        {
          if (File.Exists(candidate))
            return candidate;
        }
      }

      return null;
    }
  }
}
