/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.IO;
using System.Threading.Tasks;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using T3Foundation.Mvvm;

namespace T3Foundation.Services.Settings
{
  /// <summary>
  /// JSON-file-based settings persistence using Newtonsoft.Json.
  /// Thread-safe via <see cref="AsyncLock"/>.
  /// </summary>
  public class T3JsonSettingsService : IT3SettingsService
  {
    private readonly string _filePath;
    private readonly AsyncLock _lock = new();
    private Dictionary<string, JToken> _settings = new();

    /// <inheritdoc/>
    public event Action<string>? OnSettingChanged;

    /// <summary>
    /// Create a settings service that persists to the given file path.
    /// Defaults to {AppData}/{AppName}/settings.json.
    /// </summary>
    public T3JsonSettingsService(string? filePath = null)
    {
      if (filePath != null)
      {
        _filePath = filePath;
      }
      else
      {
        var appData = Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData);
        var appName = AppDomain.CurrentDomain.FriendlyName;
        _filePath = Path.Combine(appData, appName, "settings.json");
      }
    }

    /// <inheritdoc/>
    public T Get<T>(string key, T defaultValue = default!)
    {
      if (_settings.TryGetValue(key, out var token))
      {
        try
        {
          return token.ToObject<T>()!;
        }
        catch
        {
          return defaultValue;
        }
      }
      return defaultValue;
    }

    /// <inheritdoc/>
    public void Set<T>(string key, T value)
    {
      _settings[key] = JToken.FromObject(value!);
      T3Core.Log($"Setting '{key}' updated.", T3LogLevel.Debug);
      OnSettingChanged?.Invoke(key);
    }

    /// <inheritdoc/>
    public async Task SaveAsync()
    {
      using (await _lock.LockAsync())
      {
        var dir = Path.GetDirectoryName(_filePath);
        if (!string.IsNullOrEmpty(dir) && !Directory.Exists(dir))
          Directory.CreateDirectory(dir);

        var json = JsonConvert.SerializeObject(_settings, Formatting.Indented);
        await Task.Run(() => File.WriteAllText(_filePath, json));
        T3Core.Log($"Settings saved to {_filePath}", T3LogLevel.Debug);
      }
    }

    /// <inheritdoc/>
    public async Task LoadAsync()
    {
      using (await _lock.LockAsync())
      {
        if (!File.Exists(_filePath))
        {
          T3Core.Log($"No settings file found at {_filePath}, using defaults.", T3LogLevel.Debug);
          return;
        }

        try
        {
          var json = await Task.Run(() => File.ReadAllText(_filePath));
          _settings = JsonConvert.DeserializeObject<Dictionary<string, JToken>>(json)
                      ?? new Dictionary<string, JToken>();
          T3Core.Log($"Settings loaded from {_filePath}", T3LogLevel.Debug);
        }
        catch (Exception ex)
        {
          T3Core.Log($"Failed to load settings: {ex.Message}", T3LogLevel.Error);
          _settings = new Dictionary<string, JToken>();
        }
      }
    }
  }
}
