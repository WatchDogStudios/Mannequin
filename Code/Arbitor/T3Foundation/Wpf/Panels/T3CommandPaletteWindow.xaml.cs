/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Windows;
using System.Windows.Input;

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// UE-style command palette. Borderless overlay window summoned by
  /// Ctrl+Shift+P (handled by <see cref="Shell.T3ShellWindow"/>). Shows a fuzzy
  /// search box over a flat list of every registered tool window + menu item.
  /// </summary>
  public partial class T3CommandPaletteWindow : Window
  {
    /// <summary>All possible entries; supplied by the caller.</summary>
    private readonly List<T3CommandEntry> _all;

    /// <summary>The currently visible (filtered) subset bound to the ListBox.</summary>
    public ObservableCollection<T3CommandEntry> Filtered { get; } = new ObservableCollection<T3CommandEntry>();

    public T3CommandPaletteWindow(IEnumerable<T3CommandEntry> entries)
    {
      InitializeComponent();
      _all = entries?.ToList() ?? new List<T3CommandEntry>();
      RebuildFiltered(string.Empty);
      Loaded += (_, __) => QueryBox.Focus();
      Deactivated += (_, __) => Close();
    }

    private void OnQueryChanged(object sender, System.Windows.Controls.TextChangedEventArgs e)
    {
      RebuildFiltered(QueryBox.Text ?? string.Empty);
    }

    private void RebuildFiltered(string query)
    {
      Filtered.Clear();

      IEnumerable<T3CommandEntry> source = _all;
      if (!string.IsNullOrEmpty(query))
      {
        // Simple substring + token-prefix match. Sort by score: exact prefix > substring.
        source = _all
          .Where(e => e.Title.IndexOf(query, StringComparison.OrdinalIgnoreCase) >= 0
                   || e.Source.IndexOf(query, StringComparison.OrdinalIgnoreCase) >= 0)
          .OrderByDescending(e => e.Title.StartsWith(query, StringComparison.OrdinalIgnoreCase) ? 1 : 0);
      }

      foreach (var entry in source)
        Filtered.Add(entry);

      if (Filtered.Count > 0)
        ResultList.SelectedIndex = 0;
    }

    private void OnQueryKeyDown(object sender, KeyEventArgs e)
    {
      switch (e.Key)
      {
        case Key.Escape:
          Close();
          e.Handled = true;
          break;

        case Key.Down:
          MoveSelection(1);
          e.Handled = true;
          break;

        case Key.Up:
          MoveSelection(-1);
          e.Handled = true;
          break;

        case Key.Enter:
          ExecuteSelected();
          e.Handled = true;
          break;
      }
    }

    private void MoveSelection(int delta)
    {
      if (Filtered.Count == 0) return;
      var i = ResultList.SelectedIndex + delta;
      if (i < 0) i = Filtered.Count - 1;
      if (i >= Filtered.Count) i = 0;
      ResultList.SelectedIndex = i;
      ResultList.ScrollIntoView(ResultList.SelectedItem);
    }

    private void OnResultDoubleClick(object sender, MouseButtonEventArgs e) => ExecuteSelected();

    private void ExecuteSelected()
    {
      if (ResultList.SelectedItem is T3CommandEntry entry)
      {
        Close();
        // Defer execution so the window finishes closing before the action runs.
        Dispatcher.BeginInvoke(new Action(() =>
        {
          try { entry.Invoke(); }
          catch (Exception ex) { T3Core.Log($"Command palette entry '{entry.Title}' threw: {ex.Message}", T3LogLevel.Error); }
        }));
      }
    }
  }
}
