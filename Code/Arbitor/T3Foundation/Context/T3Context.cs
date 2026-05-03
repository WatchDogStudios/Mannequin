/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Threading.Tasks;

namespace T3Foundation.Context
{
  /// <summary>
  /// Base implementation of <see cref="IT3Context"/> with in-memory property storage
  /// and change tracking. Subclass this for specific data sources.
  ///
  /// Override <see cref="SaveAsync"/> and <see cref="ReloadAsync"/> to implement
  /// persistence for your data format (JSON file, C++ DLL, database, etc.).
  /// </summary>
  public class T3Context : IT3Context
  {
    private readonly Dictionary<string, object?> _properties = new();
    private readonly Dictionary<string, List<Action<string>>> _subscribers = new();
    private bool _isDirty;
    private bool _disposed;

    public string ContextId { get; }
    public string ContextName { get; protected set; }
    public Type DataType { get; protected set; }
    public bool IsReadOnly { get; set; }

    public bool IsDirty
    {
      get => _isDirty;
      protected set => _isDirty = value;
    }

    public event Action<IT3Context, string>? OnPropertyChanged;
    public event Action<IT3Context>? OnSaved;

    public T3Context(string contextName, Type dataType)
    {
      ContextId = Guid.NewGuid().ToString("N");
      ContextName = contextName;
      DataType = dataType;
    }

    // ─────────────────── Property Access ───────────────────

    public virtual T GetProperty<T>(string path)
    {
      if (_properties.TryGetValue(path, out var value) && value is T typed)
        return typed;

      return default!;
    }

    public virtual void SetProperty<T>(string path, T value)
    {
      if (IsReadOnly)
        throw new InvalidOperationException($"Context '{ContextName}' is read-only.");

      var oldValue = _properties.TryGetValue(path, out var existing) ? existing : null;
      _properties[path] = value;
      _isDirty = true;

      NotifyPropertyChanged(path);
    }

    public virtual bool HasProperty(string path)
    {
      return _properties.ContainsKey(path);
    }

    /// <summary>
    /// Directly set a property value without dirty tracking or notifications.
    /// Use this during initial data loading in <see cref="ReloadAsync"/>.
    /// </summary>
    protected void SetPropertySilent(string path, object? value)
    {
      _properties[path] = value;
    }

    /// <summary>
    /// Get the raw property dictionary for serialization in <see cref="SaveAsync"/>.
    /// </summary>
    protected IReadOnlyDictionary<string, object?> GetAllProperties() => _properties;

    /// <summary>
    /// Clear all properties. Use during <see cref="ReloadAsync"/>.
    /// </summary>
    protected void ClearProperties()
    {
      _properties.Clear();
    }

    // ─────────────────── Change Notifications ───────────────────

    public void SubscribeToChanges(string path, Action<string> onChange)
    {
      if (!_subscribers.TryGetValue(path, out var list))
      {
        list = new List<Action<string>>();
        _subscribers[path] = list;
      }
      list.Add(onChange);
    }

    public void UnsubscribeFromChanges(string path, Action<string> onChange)
    {
      if (_subscribers.TryGetValue(path, out var list))
        list.Remove(onChange);
    }

    /// <summary>
    /// Fire change notifications for a property path.
    /// Notifies both path-specific subscribers and the global event.
    /// </summary>
    protected void NotifyPropertyChanged(string path)
    {
      // Path-specific subscribers
      if (_subscribers.TryGetValue(path, out var list))
      {
        foreach (var handler in list)
        {
          try { handler(path); }
          catch (Exception ex)
          {
            T3Core.Log($"Context change handler error on '{path}': {ex.Message}", T3LogLevel.Error);
          }
        }
      }

      // Wildcard subscribers (subscribed to "*")
      if (_subscribers.TryGetValue("*", out var wildcardList))
      {
        foreach (var handler in wildcardList)
        {
          try { handler(path); }
          catch (Exception ex)
          {
            T3Core.Log($"Context wildcard handler error: {ex.Message}", T3LogLevel.Error);
          }
        }
      }

      // Global event
      OnPropertyChanged?.Invoke(this, path);
    }

    // ─────────────────── Persistence ───────────────────

    /// <summary>
    /// Override to persist data to the backing store.
    /// Call base.SaveAsync() to clear the dirty flag and fire <see cref="OnSaved"/>.
    /// </summary>
    public virtual Task SaveAsync()
    {
      _isDirty = false;
      OnSaved?.Invoke(this);
      T3Core.Log($"Context '{ContextName}' saved.", T3LogLevel.Debug);
      return Task.CompletedTask;
    }

    /// <summary>
    /// Override to reload data from the backing store.
    /// Use <see cref="ClearProperties"/> and <see cref="SetPropertySilent"/> during reload.
    /// </summary>
    public virtual Task ReloadAsync()
    {
      _isDirty = false;
      T3Core.Log($"Context '{ContextName}' reloaded.", T3LogLevel.Debug);
      return Task.CompletedTask;
    }

    // ─────────────────── Dispose ───────────────────

    public void Dispose()
    {
      if (_disposed) return;
      _disposed = true;

      _subscribers.Clear();
      OnPropertyChanged = null;
      OnSaved = null;

      GC.SuppressFinalize(this);
    }
  }
}
