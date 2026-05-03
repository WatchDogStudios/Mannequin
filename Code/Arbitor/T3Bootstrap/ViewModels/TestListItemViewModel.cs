/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

#nullable enable

using System.Windows.Media;
using T3.Models;
using T3.Services;
using T3Foundation.Mvvm;

namespace T3.ViewModels
{
  /// <summary>
  /// ViewModel for individual test items in the test list.
  /// </summary>
  public class TestListItemViewModel : T3ObservableObject
  {
    private string _testName = "";
    private string _api = "";
    private bool _passed;
    private double _meanError;
    private T3VisualTestResult? _result;
    private bool _isIncluded;

    public string TestName
    {
      get => _testName;
      set
      {
        if (SetProperty(ref _testName, value))
          OnPropertyChanged(nameof(DisplayName));
      }
    }

    public string API
    {
      get => _api;
      set
      {
        if (SetProperty(ref _api, value))
          OnPropertyChanged(nameof(DisplayName));
      }
    }

    public bool Passed
    {
      get => _passed;
      set
      {
        if (SetProperty(ref _passed, value))
          OnPropertyChanged(nameof(StatusBrush));
      }
    }

    public double MeanError
    {
      get => _meanError;
      set
      {
        if (SetProperty(ref _meanError, value))
          OnPropertyChanged(nameof(MeanErrorText));
      }
    }

    public T3VisualTestResult? Result
    {
      get => _result;
      set
      {
        if (SetProperty(ref _result, value))
        {
          OnPropertyChanged(nameof(StatusBrush));
          OnPropertyChanged(nameof(MeanErrorText));
        }
      }
    }

    public bool IsIncluded
    {
      get => _isIncluded;
      set => SetProperty(ref _isIncluded, value);
    }

    public string DisplayName => string.IsNullOrWhiteSpace(API) ? TestName : $"{TestName} [{API}]";

    public string MeanErrorText => Result == null ? "(not run)" : (MeanError > 0 ? $"(err: {MeanError:F4})" : "");

    public Brush StatusBrush => Result == null ? Brushes.Gray :
      (Result.StatusText == "SKIPPED" ? Brushes.Gray :
      (Result.Passed ? Brushes.LimeGreen :
      (Result.RenderSucceeded ? Brushes.OrangeRed : Brushes.Red)));

    /// <summary>
    /// Create from a test result.
    /// </summary>
    public static TestListItemViewModel FromResult(T3VisualTestResult result)
    {
      return new TestListItemViewModel
      {
        TestName = result.TestName,
        API = result.API,
        Passed = result.Passed,
        MeanError = result.MeanError,
        Result = result
      };
    }

    /// <summary>
    /// Create from a discovered test entry before it has been run.
    /// </summary>
    public static TestListItemViewModel FromDiscoveredTest(T3VisualTestResult result)
    {
      return new TestListItemViewModel
      {
        TestName = result.TestName,
        API = result.API,
        Passed = false,
        MeanError = 0,
        Result = null
      };
    }
  }
}
