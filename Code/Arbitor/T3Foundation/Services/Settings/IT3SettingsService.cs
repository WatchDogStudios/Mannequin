/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Threading.Tasks;

namespace T3Foundation.Services.Settings
{
  /// <summary>
  /// Generic settings persistence service.
  /// </summary>
  public interface IT3SettingsService
  {
    /// <summary>
    /// Get a setting value by key, returning <paramref name="defaultValue"/> if not found.
    /// </summary>
    T Get<T>(string key, T defaultValue = default!);

    /// <summary>
    /// Set a setting value by key.
    /// </summary>
    void Set<T>(string key, T value);

    /// <summary>
    /// Persist all settings to storage.
    /// </summary>
    Task SaveAsync();

    /// <summary>
    /// Load settings from storage.
    /// </summary>
    Task LoadAsync();

    /// <summary>
    /// Raised when a setting value changes. Parameter is the setting key.
    /// </summary>
    event Action<string> OnSettingChanged;
  }
}
