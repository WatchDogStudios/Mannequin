/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

#nullable enable

using System;
using System.Windows.Media;
using T3.Models;

namespace T3.ViewModels
{
  public sealed class ApiCallCheckViewModel
  {
    private readonly T3ApiCallCheck _check;

    public ApiCallCheckViewModel(T3ApiCallCheck check)
    {
      _check = check;
    }

    public string CallName => _check.CallName;
    public string Category => _check.Category;
    public string Message => _check.Message;
    public string Recommendation => _check.Recommendation;
    public string StatusText => _check.StatusText;
    public string DetailText => string.IsNullOrWhiteSpace(_check.Recommendation)
      ? _check.Message
      : $"{_check.Message} {_check.Recommendation}";

    public Brush StatusBrush
    {
      get
      {
        if (_check.Passed)
          return Brushes.LimeGreen;

        if (string.Equals(StatusText, "WARN", StringComparison.OrdinalIgnoreCase))
          return Brushes.Goldenrod;

        if (string.Equals(StatusText, "FATAL", StringComparison.OrdinalIgnoreCase))
          return Brushes.OrangeRed;

        return Brushes.Red;
      }
    }
  }
}
