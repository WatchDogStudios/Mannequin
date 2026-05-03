/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Threading.Tasks;

namespace T3Foundation.Services.Dialog
{
  /// <summary>
  /// Result from a dialog interaction.
  /// </summary>
  public enum T3DialogResult
  {
    None,
    OK,
    Cancel,
    Yes,
    No
  }

  /// <summary>
  /// Button combinations for message dialogs.
  /// </summary>
  public enum T3DialogButton
  {
    OK,
    OKCancel,
    YesNo,
    YesNoCancel
  }

  /// <summary>
  /// Notification severity types for toast-style notifications.
  /// </summary>
  public enum T3NotificationType
  {
    Info,
    Success,
    Warning,
    Error
  }

  /// <summary>
  /// Abstraction for dialogs and notifications, decoupled from WPF MessageBox.
  /// </summary>
  public interface IT3DialogService
  {
    /// <summary>
    /// Show a message dialog and return the user's choice.
    /// </summary>
    Task<T3DialogResult> ShowMessageAsync(string title, string message, T3DialogButton buttons = T3DialogButton.OK);

    /// <summary>
    /// Show an open-file dialog and return the selected path, or null if cancelled.
    /// </summary>
    Task<string?> ShowOpenFileDialogAsync(string filter, string? title = null);

    /// <summary>
    /// Show a save-file dialog and return the selected path, or null if cancelled.
    /// </summary>
    Task<string?> ShowSaveFileDialogAsync(string filter, string? title = null);

    /// <summary>
    /// Show a folder-browser dialog and return the selected path, or null if cancelled.
    /// </summary>
    Task<string?> ShowFolderDialogAsync(string? title = null);

    /// <summary>
    /// Request a toast-style notification. The UI layer subscribes to
    /// <see cref="OnNotificationRequested"/> and renders the toast.
    /// </summary>
    void ShowNotification(string message, T3NotificationType type, int durationMs = 3000);

    /// <summary>
    /// Raised when <see cref="ShowNotification"/> is called.
    /// Parameters: message, type, durationMs.
    /// </summary>
    event Action<string, T3NotificationType, int> OnNotificationRequested;
  }
}
