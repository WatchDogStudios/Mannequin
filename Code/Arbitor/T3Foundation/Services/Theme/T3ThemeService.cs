/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Windows;
using Syncfusion.SfSkinManager;

namespace T3Foundation.Services.Theme
{
  /// <summary>
  /// Runtime theme management service wrapping Syncfusion SfSkinManager.
  /// Centralizes theme registration and switching that was previously inline in window constructors.
  /// </summary>
  public class T3ThemeService
  {
    private static readonly string[] s_DefaultThemes = { "MaterialDark", "MaterialLight", "FluentDark", "FluentLight" };
    private readonly List<string> _availableThemes = new(s_DefaultThemes);
    private string _currentTheme = "MaterialDark";
    private bool _initialized;

    /// <summary>
    /// The currently applied theme name (e.g. "MaterialDark").
    /// </summary>
    public string CurrentTheme => _currentTheme;

    /// <summary>
    /// List of available theme names.
    /// </summary>
    public IReadOnlyList<string> AvailableThemes => _availableThemes;

    /// <summary>
    /// Raised after the theme is changed.
    /// </summary>
    public event Action<string>? OnThemeChanged;

    /// <summary>
    /// Initialize the theme system and register the custom basetheme skin helper.
    /// Call once at application startup before applying any theme.
    /// </summary>
    public void Initialize()
    {
      if (_initialized)
        return;

      // Register the custom basetheme from the Themes assembly
      string style = "basetheme";
      var skinHelperStr = $"Syncfusion.Themes.{style}.WPF.{style}SkinHelper, Syncfusion.Themes.{style}.WPF";
      var skinHelperType = Type.GetType(skinHelperStr);
      if (skinHelperType != null)
      {
        var styleInstance = Activator.CreateInstance(skinHelperType) as SkinHelper;
        if (styleInstance != null)
        {
          SfSkinManager.RegisterTheme(style, styleInstance);
          T3Core.Log("Registered basetheme skin helper.", T3LogLevel.Debug);
        }
      }

      _initialized = true;
      T3Core.Log("Theme service initialized.", T3LogLevel.Info);
    }

    /// <summary>
    /// Apply a theme to a WPF window or the entire application.
    /// </summary>
    /// <param name="themeName">Theme variant (e.g. "MaterialDark", "MaterialLight")</param>
    /// <param name="target">The DependencyObject to apply the theme to (Window or Application).</param>
    public void ApplyTheme(string themeName, DependencyObject target)
    {
      if (!_initialized)
        Initialize();

      // SfSkinManager.SetTheme overwrites the window Style with its own implicit
      // theme style. T3ShellWindow ships its own chrome via the framework Style,
      // so applying a Syncfusion theme on the window wipes out the template and
      // the user sees a black, empty window. Defer the theme: store the pending
      // name; the shell will apply it to its DockHost in OnApplyTemplate, where
      // it cascades to the Syncfusion controls inside without touching the chrome.
      if (target is Wpf.Shell.T3ShellWindow shell)
      {
        shell.PendingSyncfusionTheme = themeName;
        if (shell.DockHost != null)
          SfSkinManager.SetTheme(shell.DockHost, new Syncfusion.SfSkinManager.Theme($"basetheme;{themeName}"));
        _currentTheme = themeName;
        T3Core.Log($"Theme '{themeName}' deferred for shell dock host (window Style preserved).", T3LogLevel.Info);
        OnThemeChanged?.Invoke(themeName);
        return;
      }

      SfSkinManager.SetTheme(target, new Syncfusion.SfSkinManager.Theme($"basetheme;{themeName}"));
      _currentTheme = themeName;
      T3Core.Log($"Applied theme: {themeName}", T3LogLevel.Info);
      OnThemeChanged?.Invoke(themeName);
    }

    /// <summary>
    /// Register an additional theme name as available.
    /// </summary>
    public void RegisterTheme(string themeName)
    {
      if (!_availableThemes.Contains(themeName))
        _availableThemes.Add(themeName);
    }
  }
}
