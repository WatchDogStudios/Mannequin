/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// One row in the command palette. <see cref="Source"/> is the human-readable
  /// origin ("View", "File", "Tool Windows") shown in the right-hand column.
  /// </summary>
  public sealed class T3CommandEntry
  {
    public string Title { get; }
    public string Source { get; }
    public string? IconKey { get; }
    public Action Invoke { get; }

    public T3CommandEntry(string title, string source, Action invoke, string? iconKey = null)
    {
      Title = title;
      Source = source;
      Invoke = invoke;
      IconKey = iconKey;
    }
  }
}
