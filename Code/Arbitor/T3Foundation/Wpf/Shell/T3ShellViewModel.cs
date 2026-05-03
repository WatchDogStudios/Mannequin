/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Input;
using CommunityToolkit.Mvvm.Input;
using T3Foundation.Mvvm;

namespace T3Foundation.Wpf.Shell
{
  /// <summary>
  /// ViewModel for <see cref="T3ShellWindow"/>.
  /// Backs the chromed title bar, data-driven menu/toolbar, status bar, and
  /// workspace switcher. Apps typically do not subclass this directly: they
  /// override <c>RegisterMenu</c>/<c>RegisterToolbar</c>/<c>RegisterToolWindows</c>
  /// on the shell window, and the framework populates these collections.
  /// </summary>
  public class T3ShellViewModel : T3ViewModelBase
  {
    private string _statusText = string.Empty;
    private string _statusAccessoryText = string.Empty;
    private bool _isMaximized;
    private T3WorkspaceDescriptor? _currentWorkspace;

    public T3ShellViewModel()
    {
      MinimizeCommand = new RelayCommand(() => RequestMinimize?.Invoke());
      MaximizeRestoreCommand = new RelayCommand(() => RequestMaximizeRestore?.Invoke());
      CloseCommand = new RelayCommand(() => RequestClose?.Invoke());
      OpenCommandPaletteCommand = new RelayCommand(() => RequestCommandPalette?.Invoke());
    }

    public ObservableCollection<T3MenuItem> MenuItems { get; } = new ObservableCollection<T3MenuItem>();
    public ObservableCollection<T3ToolbarItem> ToolbarItems { get; } = new ObservableCollection<T3ToolbarItem>();
    public ObservableCollection<T3WorkspaceDescriptor> Workspaces { get; } = new ObservableCollection<T3WorkspaceDescriptor>();

    public T3WorkspaceDescriptor? CurrentWorkspace
    {
      get => _currentWorkspace;
      set
      {
        if (SetProperty(ref _currentWorkspace, value) && value != null)
          WorkspaceChanged?.Invoke(value);
      }
    }

    public string StatusText
    {
      get => _statusText;
      set => SetProperty(ref _statusText, value);
    }

    public string StatusAccessoryText
    {
      get => _statusAccessoryText;
      set => SetProperty(ref _statusAccessoryText, value);
    }

    public bool IsMaximized
    {
      get => _isMaximized;
      set => SetProperty(ref _isMaximized, value);
    }

    public ICommand MinimizeCommand { get; }
    public ICommand MaximizeRestoreCommand { get; }
    public ICommand CloseCommand { get; }
    public ICommand OpenCommandPaletteCommand { get; }

    /// <summary>Raised when the user clicks the title-bar minimize button.</summary>
    public event Action? RequestMinimize;

    /// <summary>Raised when the user clicks the title-bar maximize/restore button.</summary>
    public event Action? RequestMaximizeRestore;

    /// <summary>Raised when the user clicks the title-bar close button.</summary>
    public event Action? RequestClose;

    /// <summary>Raised when the user invokes the command palette (Ctrl+Shift+P).</summary>
    public event Action? RequestCommandPalette;

    /// <summary>Raised when <see cref="CurrentWorkspace"/> changes.</summary>
    public event Action<T3WorkspaceDescriptor>? WorkspaceChanged;
  }
}
