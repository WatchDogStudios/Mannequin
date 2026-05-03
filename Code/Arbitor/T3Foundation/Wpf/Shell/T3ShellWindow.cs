/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Syncfusion.Windows.Tools.Controls;
using T3Foundation.Plugin;
using T3Foundation.Services.DI;
using T3Foundation.Services.Shell;
using T3Foundation.Wpf.Panels;

namespace T3Foundation.Wpf.Shell
{
  /// <summary>
  /// Base class for T3 application main windows. Inheriting from this gives
  /// you the chromed title bar, data-driven menu/toolbar, dockable tool
  /// windows, status bar, and FrostEd x Unreal theme - all from one type.
  /// <para>
  /// Apps typically write:
  /// <code>
  /// &lt;shell:T3ShellWindow x:Class="MyApp.MainWindow"
  ///                       xmlns:shell="clr-namespace:T3Foundation.Wpf.Shell;assembly=T3Foundation"
  ///                       Title="My App"/&gt;
  /// </code>
  /// and override <see cref="RegisterMenu"/>, <see cref="RegisterToolbar"/>, and
  /// (in Phase 3) <c>RegisterToolWindows</c> in the code-behind.
  /// </para>
  /// </summary>
  [TemplatePart(Name = PART_TitleBar, Type = typeof(FrameworkElement))]
  [TemplatePart(Name = PART_MenuHost, Type = typeof(ContentControl))]
  [TemplatePart(Name = PART_ToolbarHost, Type = typeof(ContentControl))]
  [TemplatePart(Name = PART_DockHost, Type = typeof(DockingManager))]
  [TemplatePart(Name = PART_StatusHost, Type = typeof(T3StatusBar))]
  public class T3ShellWindow : Window
  {
    public const string PART_TitleBar = "PART_TitleBar";
    public const string PART_MenuHost = "PART_MenuHost";
    public const string PART_ToolbarHost = "PART_ToolbarHost";
    public const string PART_DockHost = "PART_DockHost";
    public const string PART_StatusHost = "PART_StatusHost";

    private FrameworkElement? _titleBar;

    /// <summary>The shell ViewModel. Set as DataContext if no other context is supplied.</summary>
    public T3ShellViewModel ShellViewModel { get; }

    /// <summary>The DockingManager hosted in the shell template. Available after <see cref="OnApplyTemplate"/>.</summary>
    public DockingManager? DockHost { get; private set; }

    /// <summary>The status bar control hosted in the shell template. Available after <see cref="OnApplyTemplate"/>.</summary>
    public T3StatusBar? StatusBar { get; private set; }

    /// <summary>
    /// Set by <c>T3ThemeService.ApplyTheme</c> when called on a shell window
    /// before its template is applied. The shell consumes this in
    /// <see cref="OnApplyTemplate"/> to push the Syncfusion theme onto its dock host.
    /// </summary>
    internal string? PendingSyncfusionTheme { get; set; }

    static T3ShellWindow()
    {
      DefaultStyleKeyProperty.OverrideMetadata(
        typeof(T3ShellWindow),
        new FrameworkPropertyMetadata(typeof(T3ShellWindow)));
    }

    public T3ShellWindow()
    {
      ShellViewModel = new T3ShellViewModel();

      // Use the shell VM as the DataContext so the template's bindings resolve;
      // apps that need their own VM should set a Grid.DataContext inside the
      // dock host instead of replacing the window-level context.
      if (DataContext == null)
        DataContext = ShellViewModel;

      ShellViewModel.RequestMinimize += () => SystemCommands.MinimizeWindow(this);
      ShellViewModel.RequestMaximizeRestore += ToggleMaximizeRestore;
      ShellViewModel.RequestClose += () => SystemCommands.CloseWindow(this);
      ShellViewModel.RequestCommandPalette += OpenCommandPalette;

      StateChanged += (_, __) => ShellViewModel.IsMaximized = (WindowState == WindowState.Maximized);

      // Ctrl+Shift+P opens the command palette. Wired here so apps don't have
      // to opt in - the gesture works as soon as a T3ShellWindow is shown.
      InputBindings.Add(new KeyBinding(
        ShellViewModel.OpenCommandPaletteCommand,
        new KeyGesture(Key.P, ModifierKeys.Control | ModifierKeys.Shift)));

      Loaded += OnShellLoadedHandler;

      // WPF implicit styles match the EXACT type only; subclasses (apps' MainWindow)
      // would otherwise render with native chrome. Look up the framework style by
      // T3ShellWindow type and apply it explicitly so derived windows inherit it.
      ApplyShellStyle();
    }

    private void ApplyShellStyle()
    {
      if (Style != null) return;

      var found = TryFindResource(typeof(T3ShellWindow)) as Style
               ?? System.Windows.Application.Current?.TryFindResource(typeof(T3ShellWindow)) as Style;
      if (found != null)
      {
        Style = found;
        T3Core.Log("T3ShellWindow style applied to derived window.", T3LogLevel.Debug);
      }
      else
      {
        T3Core.Log("T3ShellWindow style not found in resources - shell template will not render. Ensure App.xaml merges T3Resources.xaml.", T3LogLevel.Error);
      }
    }

    private void OnShellLoadedHandler(object sender, RoutedEventArgs e)
    {
      Loaded -= OnShellLoadedHandler;

      // Defense in depth: force-reapply our framework Style. Other code (notably
      // Syncfusion's SfSkinManager.SetTheme) can overwrite the Style between the
      // ctor and Loaded; force-reapplying here guarantees the chrome wins.
      var found = TryFindResource(typeof(T3ShellWindow)) as Style
               ?? System.Windows.Application.Current?.TryFindResource(typeof(T3ShellWindow)) as Style;
      if (found != null && Style != found)
      {
        Style = found;
        T3Core.Log("T3ShellWindow style force-reapplied at Loaded.", T3LogLevel.Debug);
      }

      OnShellLoaded();
    }

    public override void OnApplyTemplate()
    {
      base.OnApplyTemplate();

      if (_titleBar != null)
        _titleBar.MouseLeftButtonDown -= OnTitleBarMouseLeftButtonDown;

      _titleBar = GetTemplateChild(PART_TitleBar) as FrameworkElement;
      if (_titleBar != null)
        _titleBar.MouseLeftButtonDown += OnTitleBarMouseLeftButtonDown;

      DockHost = GetTemplateChild(PART_DockHost) as DockingManager;
      StatusBar = GetTemplateChild(PART_StatusHost) as T3StatusBar;

      T3Core.Log($"Shell template applied: titleBar={_titleBar != null}, dockHost={DockHost != null}, statusBar={StatusBar != null}", T3LogLevel.Debug);

      if (DockHost != null)
      {
        ConfigureDockingManager(DockHost);

        // Apply any theme that was deferred while DockHost was still null. The
        // theme cascades from here to the Syncfusion controls inside (TabItemExt,
        // ToolBarAdv) without touching this window's Style.
        if (PendingSyncfusionTheme != null)
        {
          try
          {
            Syncfusion.SfSkinManager.SfSkinManager.SetTheme(DockHost,
              new Syncfusion.SfSkinManager.Theme($"basetheme;{PendingSyncfusionTheme}"));
            T3Core.Log($"Deferred Syncfusion theme '{PendingSyncfusionTheme}' applied to dock host.", T3LogLevel.Debug);
          }
          catch (Exception ex)
          {
            T3Core.Log($"Failed to apply deferred theme: {ex.Message}", T3LogLevel.Warning);
          }
          PendingSyncfusionTheme = null;
        }
      }
    }

    /// <summary>
    /// Override to customize the DockingManager. The default enables document
    /// container + state persistence and installs T3 brushes.
    /// </summary>
    protected virtual void ConfigureDockingManager(DockingManager dock)
    {
      dock.UseDocumentContainer = true;
      dock.PersistState = true;
    }

    /// <summary>
    /// Override to register tool windows with the supplied registry. Called
    /// after the template has been applied so the dock host is bound.
    /// </summary>
    protected virtual void RegisterToolWindows(IT3ToolWindowRegistry registry) { }

    /// <summary>Override to populate menus via the supplied builder.</summary>
    protected virtual void RegisterMenu(IT3MenuService menu) { }

    /// <summary>Override to populate toolbar items.</summary>
    protected virtual void RegisterToolbar(IT3ToolbarService toolbar) { }

    /// <summary>Workspace name to load (or save under) on startup. Override per app.</summary>
    protected virtual string DefaultWorkspaceName => "Default";

    /// <summary>
    /// Called once after the shell is fully loaded (template applied + window shown).
    /// Resolves shell services from DI, attaches the dock host, runs auto-discovery
    /// for <c>[T3ToolWindow]</c>-decorated types, then invokes the user's register hooks.
    /// </summary>
    protected virtual void OnShellLoaded()
    {
      var registry = T3ServiceCollection.ResolveOptional<IT3ToolWindowRegistry>();
      var menu = T3ServiceCollection.ResolveOptional<IT3MenuService>();
      var toolbar = T3ServiceCollection.ResolveOptional<IT3ToolbarService>();
      var layout = T3ServiceCollection.ResolveOptional<IT3LayoutService>();
      var pluginManager = T3ServiceCollection.ResolveOptional<T3PluginManager>();

      if (DockHost != null)
      {
        registry?.AttachDockHost(DockHost);
        layout?.AttachDockHost(DockHost);
      }

      // Wire VM collections to the services so the template's bindings light up.
      if (menu != null)
      {
        ShellViewModel.MenuItems.Clear();
        foreach (var item in menu.Items)
          ShellViewModel.MenuItems.Add(item);
        menu.Items.CollectionChanged += (_, __) =>
        {
          ShellViewModel.MenuItems.Clear();
          foreach (var item in menu.Items)
            ShellViewModel.MenuItems.Add(item);
        };
      }

      if (toolbar != null)
      {
        ShellViewModel.ToolbarItems.Clear();
        foreach (var item in toolbar.Items)
          ShellViewModel.ToolbarItems.Add(item);
        toolbar.Items.CollectionChanged += (_, __) =>
        {
          ShellViewModel.ToolbarItems.Clear();
          foreach (var item in toolbar.Items)
            ShellViewModel.ToolbarItems.Add(item);
        };
      }

      // Auto-discover [T3ToolWindow] types in loaded assemblies first, then let
      // the app supply explicit registrations (which can override the discovered ones).
      if (registry != null)
      {
        pluginManager?.DiscoverToolWindows(registry);
        RegisterToolWindows(registry);
      }

      if (menu != null) RegisterMenu(menu);
      if (toolbar != null) RegisterToolbar(toolbar);
    }

    private void OnTitleBarMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
    {
      if (e.ClickCount == 2)
      {
        ToggleMaximizeRestore();
        return;
      }

      if (e.ButtonState == MouseButtonState.Pressed)
      {
        try { DragMove(); }
        catch (InvalidOperationException) { /* DragMove can throw if the mouse is released mid-drag */ }
      }
    }

    private void ToggleMaximizeRestore()
    {
      if (WindowState == WindowState.Maximized)
        SystemCommands.RestoreWindow(this);
      else
        SystemCommands.MaximizeWindow(this);
    }

    /// <summary>
    /// Build the command palette entry list from current menu items + tool windows
    /// and show the palette window centered on the shell.
    /// </summary>
    protected virtual void OpenCommandPalette()
    {
      var entries = new List<T3CommandEntry>();

      var menu = T3ServiceCollection.ResolveOptional<IT3MenuService>();
      if (menu != null)
      {
        foreach (var top in menu.Items)
          CollectMenuEntries(top, top.Header, entries);
      }

      var registry = T3ServiceCollection.ResolveOptional<IT3ToolWindowRegistry>();
      if (registry != null)
      {
        foreach (var desc in registry.All)
        {
          var captured = desc;
          entries.Add(new T3CommandEntry(
            $"View: {captured.Title}",
            "Tool Windows",
            () => registry.Toggle(captured.Id),
            captured.IconKey));
        }
      }

      var window = new T3CommandPaletteWindow(entries) { Owner = this };
      window.Show();
    }

    private static void CollectMenuEntries(Wpf.Shell.T3MenuItem item, string source, List<T3CommandEntry> sink)
    {
      if (item.Children.Count > 0)
      {
        foreach (var child in item.Children)
          CollectMenuEntries(child, source, sink);
        return;
      }
      if (item.IsSeparator) return;
      if (item.Command == null) return;

      var captured = item;
      sink.Add(new T3CommandEntry(captured.Header, source, () =>
      {
        if (captured.Command?.CanExecute(captured.CommandParameter) == true)
          captured.Command.Execute(captured.CommandParameter);
      }, captured.IconKey));
    }
  }
}
