/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;

namespace T3Foundation.Context
{
  /// <summary>
  /// Typed property accessor that wraps a Context path prefix for convenient
  /// property access on sub-objects.
  ///
  /// Instead of: <c>context.GetProperty&lt;float&gt;("physics.rigidbody.mass")</c>
  /// You write:  <c>accessor.Get&lt;float&gt;("mass")</c>
  ///
  /// Useful when a Panel or Action operates on a known subsection of a Context's data.
  /// </summary>
  public class T3PropertyAccessor
  {
    private readonly IT3Context _context;
    private readonly string _prefix;

    /// <summary>
    /// The context this accessor reads from and writes to.
    /// </summary>
    public IT3Context Context => _context;

    /// <summary>
    /// The path prefix prepended to all property paths.
    /// </summary>
    public string Prefix => _prefix;

    public T3PropertyAccessor(IT3Context context, string prefix)
    {
      _context = context ?? throw new ArgumentNullException(nameof(context));
      _prefix = string.IsNullOrEmpty(prefix) ? "" : prefix.TrimEnd('.') + ".";
    }

    /// <summary>
    /// Read a property relative to this accessor's prefix.
    /// </summary>
    public T Get<T>(string relativePath)
    {
      return _context.GetProperty<T>(_prefix + relativePath);
    }

    /// <summary>
    /// Write a property relative to this accessor's prefix.
    /// </summary>
    public void Set<T>(string relativePath, T value)
    {
      _context.SetProperty(_prefix + relativePath, value);
    }

    /// <summary>
    /// Check if a property exists relative to this accessor's prefix.
    /// </summary>
    public bool Has(string relativePath)
    {
      return _context.HasProperty(_prefix + relativePath);
    }

    /// <summary>
    /// Subscribe to changes on a property relative to this accessor's prefix.
    /// </summary>
    public void Subscribe(string relativePath, Action<string> onChange)
    {
      _context.SubscribeToChanges(_prefix + relativePath, onChange);
    }

    /// <summary>
    /// Unsubscribe from changes on a property relative to this accessor's prefix.
    /// </summary>
    public void Unsubscribe(string relativePath, Action<string> onChange)
    {
      _context.UnsubscribeFromChanges(_prefix + relativePath, onChange);
    }

    /// <summary>
    /// Create a nested accessor with an additional prefix segment.
    /// </summary>
    public T3PropertyAccessor Nested(string subPrefix)
    {
      return new T3PropertyAccessor(_context, _prefix + subPrefix);
    }
  }
}
