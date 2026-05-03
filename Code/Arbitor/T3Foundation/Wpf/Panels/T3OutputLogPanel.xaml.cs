/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;
using T3Foundation.Plugin;
using T3Foundation.Services.Shell;

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// Replaces hand-rolled per-app log panels: subscribes to
  /// <see cref="T3Core.OnLogMessage"/>, renders color-coded rows, supports
  /// level filtering, full-text search, auto-scroll, and copy/clear.
  /// </summary>
  [T3ToolWindow(
    "t3.log",
    "Output Log",
    DefaultSide = T3DockSide.Bottom,
    IconKey = T3Icons.Log,
    DefaultHeight = 200,
    MenuPath = "View/Output Log")]
  public partial class T3OutputLogPanel : UserControl
  {
    /// <summary>All entries received since this panel was loaded - the visible list filters this in-place.</summary>
    private readonly ObservableCollection<T3LogEntry> _allEntries = new ObservableCollection<T3LogEntry>();

    /// <summary>Bound to the ListBox; holds the filtered view.</summary>
    public ObservableCollection<T3LogEntry> Entries { get; } = new ObservableCollection<T3LogEntry>();

    private T3LogLevel _minLevel = T3LogLevel.Debug;
    private string _searchText = string.Empty;

    public T3OutputLogPanel()
    {
      InitializeComponent();

      // Backfill with anything already in T3Core's buffer so opening the panel
      // mid-session doesn't show an empty list.
      foreach (var existing in T3Core.GetLogMessages())
      {
        var entry = new T3LogEntry(existing, GuessLevel(existing));
        _allEntries.Add(entry);
      }
      RebuildFilteredView();

      Loaded += OnLoaded;
      Unloaded += OnUnloaded;
    }

    private void OnLoaded(object sender, RoutedEventArgs e)
    {
      T3Core.OnLogMessage += OnLogMessage;
    }

    private void OnUnloaded(object sender, RoutedEventArgs e)
    {
      T3Core.OnLogMessage -= OnLogMessage;
    }

    private void OnLogMessage(string message, T3LogLevel level)
    {
      var entry = new T3LogEntry(message, level);

      if (Dispatcher.CheckAccess())
        AppendEntry(entry);
      else
        Dispatcher.BeginInvoke(DispatcherPriority.Background, new Action(() => AppendEntry(entry)));
    }

    private void AppendEntry(T3LogEntry entry)
    {
      _allEntries.Add(entry);
      if (PassesFilter(entry))
        Entries.Add(entry);
    }

    private bool PassesFilter(T3LogEntry entry)
    {
      if (entry.Level < _minLevel) return false;
      if (!string.IsNullOrEmpty(_searchText) &&
          entry.Text.IndexOf(_searchText, StringComparison.OrdinalIgnoreCase) < 0)
        return false;
      return true;
    }

    private void RebuildFilteredView()
    {
      Entries.Clear();
      foreach (var e in _allEntries)
      {
        if (PassesFilter(e)) Entries.Add(e);
      }
    }

    private void OnLevelFilterChanged(object sender, SelectionChangedEventArgs e)
    {
      // ComboBox indices map to: 0 All, 1 Debug+, 2 Info+, 3 Warning+, 4 Error+
      switch (LevelFilter.SelectedIndex)
      {
        case 0: case 1: _minLevel = T3LogLevel.Debug; break;
        case 2: _minLevel = T3LogLevel.Info; break;
        case 3: _minLevel = T3LogLevel.Warning; break;
        case 4: _minLevel = T3LogLevel.Error; break;
      }
      RebuildFilteredView();
    }

    private void OnSearchChanged(object sender, TextChangedEventArgs e)
    {
      _searchText = SearchBox.Text ?? string.Empty;
      RebuildFilteredView();
    }

    private void OnClearClick(object sender, RoutedEventArgs e)
    {
      _allEntries.Clear();
      Entries.Clear();
      T3Core.ClearLog();
    }

    private void OnCopyClick(object sender, RoutedEventArgs e)
    {
      if (LogList.SelectedItem is T3LogEntry entry)
        SetClipboardSafe(entry.Text);
    }

    private void OnCopyAllClick(object sender, RoutedEventArgs e)
    {
      var sb = new StringBuilder();
      foreach (var entry in Entries) sb.AppendLine(entry.Text);
      SetClipboardSafe(sb.ToString());
    }

    /// <summary>
    /// Clipboard.SetText can throw COMException when another process owns the
    /// clipboard. Suppress and log instead of crashing the host app.
    /// </summary>
    private static void SetClipboardSafe(string text)
    {
      try { Clipboard.SetText(text); }
      catch (Exception ex) { T3Core.Log($"Clipboard copy failed: {ex.Message}", T3LogLevel.Warning); }
    }

    /// <summary>
    /// Pre-existing buffer entries are formatted strings without explicit
    /// level metadata. Look for the bracketed level token T3Core writes.
    /// </summary>
    private static T3LogLevel GuessLevel(string formatted)
    {
      if (formatted.IndexOf("[Critical]", StringComparison.OrdinalIgnoreCase) >= 0) return T3LogLevel.Critical;
      if (formatted.IndexOf("[Error]", StringComparison.OrdinalIgnoreCase) >= 0) return T3LogLevel.Error;
      if (formatted.IndexOf("[Warning]", StringComparison.OrdinalIgnoreCase) >= 0) return T3LogLevel.Warning;
      if (formatted.IndexOf("[Debug]", StringComparison.OrdinalIgnoreCase) >= 0) return T3LogLevel.Debug;
      return T3LogLevel.Info;
    }
  }
}
