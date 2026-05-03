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

namespace T3.ViewModels
{
  public sealed class ResourceSnapshotViewModel
  {
    private readonly T3ResourceSnapshot _snapshot;

    public ResourceSnapshotViewModel(T3ResourceSnapshot snapshot, T3TestRunnerService runner, string apiName, string testName)
    {
      _snapshot = snapshot;
      Values = snapshot.Values ?? new List<string>();
      StateChecks = new ObservableCollection<ApiCallCheckViewModel>((snapshot.StateChecks ?? new List<T3ResourceStateCheck>())
        .Select(check => new ApiCallCheckViewModel(new T3ApiCallCheck
        {
          Category = "State",
          CallName = check.Name,
          Passed = check.Passed,
          Severity = string.IsNullOrWhiteSpace(check.Severity) ? (check.Passed ? "Info" : "Error") : check.Severity,
          Message = string.IsNullOrWhiteSpace(check.Message)
            ? $"Actual: {check.Actual}; expected: {check.Expected}"
            : check.Message,
          Recommendation = check.Recommendation ?? ""
        })));

      string? previewPath = runner.ResolveArtifactPath(apiName, testName, snapshot.PreviewPath);
      PreviewImage = T3ImageService.LoadImage(previewPath ?? "");
    }

    public string Name => _snapshot.Name;
    public string Type => _snapshot.Type;
    public string Slot => string.IsNullOrWhiteSpace(_snapshot.Slot) ? "-" : _snapshot.Slot;
    public string Format => string.IsNullOrWhiteSpace(_snapshot.Format) ? "-" : _snapshot.Format;
    public string State => string.IsNullOrWhiteSpace(_snapshot.State) ? "-" : _snapshot.State;
    public string Summary => _snapshot.Summary;
    public string Dimensions => _snapshot.Width > 0 || _snapshot.Height > 0 || _snapshot.Depth > 0
      ? $"{_snapshot.Width} x {_snapshot.Height} x {Math.Max(1, _snapshot.Depth)}"
      : "-";
    public string Memory => _snapshot.ByteSize > 0 ? $"{_snapshot.ByteSize:N0} bytes" : "-";
    public string Layout => _snapshot.RowPitch > 0 ? $"row pitch {_snapshot.RowPitch:N0}" :
      (_snapshot.ElementCount > 0 ? $"{_snapshot.ElementCount:N0} elements" : "-");
    public IReadOnlyList<string> Values { get; }
    public ObservableCollection<ApiCallCheckViewModel> StateChecks { get; }
    public ImageSource? PreviewImage { get; }

    public string StatusText
    {
      get
      {
        if (StateChecks.Any(check => string.Equals(check.StatusText, "FATAL", StringComparison.OrdinalIgnoreCase)))
          return "FATAL";
        if (StateChecks.Any(check => string.Equals(check.StatusText, "WARN", StringComparison.OrdinalIgnoreCase)))
          return "WARN";
        if (StateChecks.Count == 0)
          return "INFO";
        return "PASS";
      }
    }

    public Brush StatusBrush
    {
      get
      {
        if (StatusText == "FATAL")
          return Brushes.OrangeRed;
        if (StatusText == "WARN")
          return Brushes.Goldenrod;
        if (StatusText == "PASS")
          return Brushes.LimeGreen;
        return Brushes.Gray;
      }
    }
  }
}
