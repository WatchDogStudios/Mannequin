/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Threading.Tasks;

namespace T3Foundation.Mvvm
{
  /// <summary>
  /// Base class for all ViewModels in T3 applications.
  /// Provides lifecycle hooks, busy state tracking, and a title property.
  /// </summary>
  public class T3ViewModelBase : T3ObservableObject
  {
    private bool _isBusy;
    private string _title = string.Empty;
    private bool _isInitialized;

    /// <summary>
    /// Indicates the ViewModel is performing a long-running operation.
    /// Bind UI busy indicators to this property.
    /// </summary>
    public bool IsBusy
    {
      get => _isBusy;
      set => SetProperty(ref _isBusy, value);
    }

    /// <summary>
    /// Display title for this ViewModel (tab headers, window titles, etc.).
    /// </summary>
    public string Title
    {
      get => _title;
      set => SetProperty(ref _title, value);
    }

    /// <summary>
    /// Whether <see cref="InitializeAsync"/> has been called.
    /// </summary>
    public bool IsInitialized
    {
      get => _isInitialized;
      private set => SetProperty(ref _isInitialized, value);
    }

    /// <summary>
    /// One-time async initialization. Called by the navigation service
    /// the first time this ViewModel is navigated to.
    /// </summary>
    public virtual Task InitializeAsync()
    {
      IsInitialized = true;
      return Task.CompletedTask;
    }

    /// <summary>
    /// Called each time this ViewModel becomes the active view.
    /// </summary>
    public virtual void OnActivated() { }

    /// <summary>
    /// Called each time navigation moves away from this ViewModel.
    /// </summary>
    public virtual void OnDeactivated() { }

    /// <summary>
    /// Called before navigating away. Return false to prevent navigation (e.g., unsaved changes).
    /// </summary>
    public virtual bool CanClose() => true;
  }
}
