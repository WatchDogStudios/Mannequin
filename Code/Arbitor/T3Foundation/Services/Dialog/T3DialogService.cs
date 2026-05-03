/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Threading.Tasks;
using System.Windows;

namespace T3Foundation.Services.Dialog
{
  /// <summary>
  /// Default WPF implementation of <see cref="IT3DialogService"/>.
  /// </summary>
  public class T3DialogService : IT3DialogService
  {
    /// <inheritdoc/>
    public event Action<string, T3NotificationType, int>? OnNotificationRequested;

    /// <inheritdoc/>
    public Task<T3DialogResult> ShowMessageAsync(string title, string message, T3DialogButton buttons = T3DialogButton.OK)
    {
      var wpfButton = buttons switch
      {
        T3DialogButton.OK => MessageBoxButton.OK,
        T3DialogButton.OKCancel => MessageBoxButton.OKCancel,
        T3DialogButton.YesNo => MessageBoxButton.YesNo,
        T3DialogButton.YesNoCancel => MessageBoxButton.YesNoCancel,
        _ => MessageBoxButton.OK
      };

      var result = MessageBox.Show(message, title, wpfButton);

      var t3Result = result switch
      {
        MessageBoxResult.OK => T3DialogResult.OK,
        MessageBoxResult.Cancel => T3DialogResult.Cancel,
        MessageBoxResult.Yes => T3DialogResult.Yes,
        MessageBoxResult.No => T3DialogResult.No,
        _ => T3DialogResult.None
      };

      return Task.FromResult(t3Result);
    }

    /// <inheritdoc/>
    public Task<string?> ShowOpenFileDialogAsync(string filter, string? title = null)
    {
      var dialog = new Microsoft.Win32.OpenFileDialog
      {
        Filter = filter,
        Title = title ?? "Open File"
      };

      return Task.FromResult(dialog.ShowDialog() == true ? dialog.FileName : null);
    }

    /// <inheritdoc/>
    public Task<string?> ShowSaveFileDialogAsync(string filter, string? title = null)
    {
      var dialog = new Microsoft.Win32.SaveFileDialog
      {
        Filter = filter,
        Title = title ?? "Save File"
      };

      return Task.FromResult(dialog.ShowDialog() == true ? dialog.FileName : null);
    }

    /// <inheritdoc/>
    public Task<string?> ShowFolderDialogAsync(string? title = null)
    {
      using (var dialog = new System.Windows.Forms.FolderBrowserDialog())
      {
        dialog.Description = title ?? "Select Folder";
        dialog.ShowNewFolderButton = true;
        var result = dialog.ShowDialog();
        return Task.FromResult(result == System.Windows.Forms.DialogResult.OK ? dialog.SelectedPath : (string?)null);
      }
    }

    /// <inheritdoc/>
    public void ShowNotification(string message, T3NotificationType type, int durationMs = 3000)
    {
      T3Core.Log($"[{type}] {message}", type switch
      {
        T3NotificationType.Error => T3LogLevel.Error,
        T3NotificationType.Warning => T3LogLevel.Warning,
        _ => T3LogLevel.Info
      });

      OnNotificationRequested?.Invoke(message, type, durationMs);
    }
  }
}
