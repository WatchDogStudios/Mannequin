/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Threading.Tasks;

namespace T3Foundation.Context
{
  /// <summary>
  /// Data-layer abstraction for the Workflow-Driven Tool Design pattern.
  ///
  /// A Context defines WHAT is being edited. It abstracts reading, writing,
  /// and change-tracking away from the UI. Panels and Documents never access
  /// raw data directly — they go through the Context.
  ///
  /// Subclass this for specific data sources: JSON files, C++ DLL interop,
  /// databases, in-memory models, etc.
  ///
  /// Based on Workflow-Driven Tool Design
  /// (See Remedy's Talk: https://www.youtube.com/watch?v=kAfb0yx07Po)
  /// </summary>
  public interface IT3Context : IDisposable
  {
    /// <summary>
    /// Unique instance identifier for this context.
    /// </summary>
    string ContextId { get; }

    /// <summary>
    /// Human-readable display name (shown in tab titles, breadcrumbs, etc.).
    /// </summary>
    string ContextName { get; }

    /// <summary>
    /// The .NET type of the underlying data this context wraps.
    /// Used by the Workflow Manager to find the correct Document type.
    /// </summary>
    Type DataType { get; }

    /// <summary>
    /// Whether this context is read-only (e.g. locked by another user, Perforce checkout).
    /// </summary>
    bool IsReadOnly { get; }

    /// <summary>
    /// Whether this context has unsaved modifications.
    /// Set automatically on <see cref="SetProperty{T}"/>, cleared on <see cref="SaveAsync"/>.
    /// </summary>
    bool IsDirty { get; }

    // ─────────────────── Property Access ───────────────────

    /// <summary>
    /// Read a named property by path. Paths use dot notation for nesting
    /// (e.g. "physics.rigidbody.mass").
    /// </summary>
    T GetProperty<T>(string path);

    /// <summary>
    /// Write a named property. Marks the context dirty and fires change notifications.
    /// Throws <see cref="InvalidOperationException"/> if <see cref="IsReadOnly"/>.
    /// </summary>
    void SetProperty<T>(string path, T value);

    /// <summary>
    /// Check whether a property path exists in this context.
    /// </summary>
    bool HasProperty(string path);

    // ─────────────────── Change Notifications ───────────────────

    /// <summary>
    /// Subscribe to changes on a specific property path.
    /// The callback receives the path that changed.
    /// </summary>
    void SubscribeToChanges(string path, Action<string> onChange);

    /// <summary>
    /// Unsubscribe from changes on a specific property path.
    /// </summary>
    void UnsubscribeFromChanges(string path, Action<string> onChange);

    /// <summary>
    /// Fired when any property in this context changes.
    /// Parameters: (context, propertyPath).
    /// </summary>
    event Action<IT3Context, string>? OnPropertyChanged;

    // ─────────────────── Persistence ───────────────────

    /// <summary>
    /// Persist all changes to the backing store. Clears <see cref="IsDirty"/>.
    /// </summary>
    Task SaveAsync();

    /// <summary>
    /// Reload data from the backing store, discarding unsaved changes.
    /// </summary>
    Task ReloadAsync();

    /// <summary>
    /// Fired after a successful save.
    /// </summary>
    event Action<IT3Context>? OnSaved;
  }
}
