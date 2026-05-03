/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Runtime.CompilerServices;
using CommunityToolkit.Mvvm.ComponentModel;

namespace T3Foundation.Mvvm
{
  /// <summary>
  /// Base observable class for all T3 Framework ViewModels and models.
  /// Extends CommunityToolkit's ObservableObject with dispose pattern and logging integration.
  /// </summary>
  public class T3ObservableObject : ObservableObject, IDisposable
  {
    private bool _isDisposed;

    /// <summary>
    /// Whether this object has been disposed.
    /// </summary>
    protected bool IsDisposed => _isDisposed;

    /// <summary>
    /// Log a message through the T3 Framework logging system.
    /// </summary>
    protected void Log(string message, T3LogLevel level = T3LogLevel.Info)
    {
      T3Core.Log(message, level);
    }

    /// <summary>
    /// Throws <see cref="ObjectDisposedException"/> if this object has been disposed.
    /// </summary>
    protected void ThrowIfDisposed()
    {
      if (_isDisposed)
        throw new ObjectDisposedException(GetType().FullName);
    }

    /// <summary>
    /// Sets a property value and logs the change at Debug level when the value actually changes.
    /// </summary>
    protected bool SetPropertyAndLog<T>(ref T field, T value, [CallerMemberName] string? propertyName = null)
    {
      if (!SetProperty(ref field, value, propertyName))
        return false;

      T3Core.Log($"{GetType().Name}.{propertyName} changed to '{value}'", T3LogLevel.Debug);
      return true;
    }

    public void Dispose()
    {
      Dispose(true);
      GC.SuppressFinalize(this);
    }

    protected virtual void Dispose(bool disposing)
    {
      if (_isDisposed)
        return;

      _isDisposed = true;
    }
  }
}
