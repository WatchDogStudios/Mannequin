/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.IO;
using System.Windows;
using Microsoft.Extensions.DependencyInjection;
using T3Foundation.Mvvm;
using T3Foundation.Plugin;
using T3Foundation.Services.DI;
using T3Foundation.Services.Dialog;
using T3Foundation.Services.Navigation;
using T3Foundation.Services.Settings;
using T3Foundation.Services.Shell;
using T3Foundation.Services.Theme;

namespace T3Foundation.Application
{
  /// <summary>
  /// Interface for T3 applications, providing access to core framework services.
  /// </summary>
  public interface CT3Application
  {
    /// <summary>
    /// Display name of the application.
    /// </summary>
    string ApplicationName { get; set; }

    /// <summary>
    /// Whether this application interacts with lower-level languages (e.g. C++, C-ABI).
    /// </summary>
    bool ManagedApplication { get; set; }

    /// <summary>
    /// The built DI service provider.
    /// </summary>
    IServiceProvider Services { get; }

    /// <summary>
    /// Navigation service for ViewModel-first navigation.
    /// </summary>
    IT3NavigationService Navigation { get; }

    /// <summary>
    /// Dialog and notification service.
    /// </summary>
    IT3DialogService Dialogs { get; }

    /// <summary>
    /// Settings persistence service.
    /// </summary>
    IT3SettingsService Settings { get; }

    /// <summary>
    /// Theme management service.
    /// </summary>
    T3ThemeService Theme { get; }

    /// <summary>Tool window registry. Apps register dockable views here; the shell hosts them.</summary>
    IT3ToolWindowRegistry ToolWindows { get; }

    /// <summary>Data-driven menu service. Renders into the shell's PART_MenuHost.</summary>
    IT3MenuService Menu { get; }

    /// <summary>Data-driven toolbar service. Renders into the shell's PART_ToolbarHost.</summary>
    IT3ToolbarService Toolbar { get; }

    /// <summary>Per-workspace dock layout persistence.</summary>
    IT3LayoutService Layout { get; }
  }

  /// <summary>
  /// WPF Application base class for T3 applications.
  /// Orchestrates framework initialization: logging, DI, plugins, themes, and settings.
  /// Inherit from this class instead of <see cref="System.Windows.Application"/>.
  /// </summary>
  public class T3WpfApplication : System.Windows.Application, CT3Application
  {
    private readonly T3PluginManager _pluginManager = new();

    public string ApplicationName { get; set; } = "T3Application";
    public bool ManagedApplication { get; set; }

    public IServiceProvider Services => T3ServiceCollection.Provider;
    public IT3NavigationService Navigation => T3ServiceCollection.Resolve<IT3NavigationService>();
    public IT3DialogService Dialogs => T3ServiceCollection.Resolve<IT3DialogService>();
    public IT3SettingsService Settings => T3ServiceCollection.Resolve<IT3SettingsService>();
    public T3ThemeService Theme { get; } = new();
    public IT3ToolWindowRegistry ToolWindows => T3ServiceCollection.Resolve<IT3ToolWindowRegistry>();
    public IT3MenuService Menu => T3ServiceCollection.Resolve<IT3MenuService>();
    public IT3ToolbarService Toolbar => T3ServiceCollection.Resolve<IT3ToolbarService>();
    public IT3LayoutService Layout => T3ServiceCollection.Resolve<IT3LayoutService>();

    /// <summary>
    /// Override to register application-specific services into the DI container.
    /// Framework services (navigation, dialogs, settings) are already registered.
    /// </summary>
    protected virtual void ConfigureServices(IServiceCollection services) { }

    /// <summary>
    /// Override to provide a path to scan for plugin assemblies.
    /// Return null to skip plugin discovery.
    /// </summary>
    protected virtual string? GetPluginDirectory() => null;

    /// <summary>
    /// Override for the default theme name to apply at startup.
    /// </summary>
    protected virtual string GetDefaultTheme() => "MaterialDark";

    /// <summary>
    /// Called after all framework initialization is complete.
    /// Override this to create your main window and perform app-specific startup.
    /// </summary>
    protected virtual void OnReady() { }

    protected override void OnStartup(StartupEventArgs e)
    {
      base.OnStartup(e);

      // 1. Initialize core logging
      T3Core.Initialize();

      // Tee log to a file in the exe directory so issues that block UI rendering
      // (template parse failures, missing services, etc.) remain diagnosable.
      try
      {
        var logPath = Path.Combine(AppDomain.CurrentDomain.BaseDirectory, $"{ApplicationName}.log");
        var sw = new StreamWriter(logPath, append: false) { AutoFlush = true };
        T3Core.OnLogMessage += (msg, lvl) =>
        {
          try { sw.WriteLine(msg); } catch { /* file may be in use */ }
        };
        T3Core.Log($"File log opened at {logPath}", T3LogLevel.Info);
      }
      catch (Exception ex)
      {
        T3Core.Log($"Failed to open file log: {ex.Message}", T3LogLevel.Warning);
      }

      // 2. Global exception handlers
      AppDomain.CurrentDomain.UnhandledException += (s, args) =>
        T3Core.Log($"Unhandled exception: {args.ExceptionObject}", T3LogLevel.Critical);

      DispatcherUnhandledException += (s, args) =>
      {
        T3Core.Log($"Dispatcher exception: {args.Exception}", T3LogLevel.Critical);
        args.Handled = true;
      };

      // 3. Configure DI
      T3ServiceCollection.Configure(services =>
      {
        // Framework services
        services.AddSingleton<IT3NavigationService, T3NavigationService>();
        services.AddSingleton<IT3DialogService, T3DialogService>();
        services.AddSingleton<IT3SettingsService>(sp => new T3JsonSettingsService());
        services.AddSingleton(Theme);
        services.AddSingleton(_pluginManager);

        // Shell services (Phase 3): registry + menu/toolbar + layout persistence.
        services.AddSingleton<IT3ToolWindowRegistry, T3ToolWindowRegistry>();
        services.AddSingleton<IT3MenuService, T3MenuService>();
        services.AddSingleton<IT3ToolbarService, T3ToolbarService>();
        services.AddSingleton<IT3LayoutService, T3LayoutService>();

        // Discover and register plugins
        var pluginDir = GetPluginDirectory();
        if (pluginDir != null)
        {
          _pluginManager.DiscoverPlugins(pluginDir);
          _pluginManager.RegisterAll(services);
        }

        // App-specific services
        ConfigureServices(services);
      });

      // 4. Build the container
      T3ServiceCollection.Build();

      // 5. Initialize plugins
      _pluginManager.InitializeAll(T3ServiceCollection.Provider);

      // 6. Initialize theme (apply to main window when it's created)
      Theme.Initialize();

      // 7. Load settings
      T3ServiceCollection.Resolve<IT3SettingsService>().LoadAsync().FireAndForget();

      T3Core.Log($"{ApplicationName} startup complete.", T3LogLevel.Info);

      // 8. App-specific startup
      OnReady();
    }

    protected override void OnExit(ExitEventArgs e)
    {
      T3Core.Log($"{ApplicationName} shutting down...", T3LogLevel.Info);

      // Save settings
      try
      {
        T3ServiceCollection.Resolve<IT3SettingsService>().SaveAsync().GetAwaiter().GetResult();
      }
      catch (Exception ex)
      {
        T3Core.Log($"Failed to save settings on exit: {ex.Message}", T3LogLevel.Error);
      }

      // Shutdown plugins
      _pluginManager.ShutdownAll();

      T3Core.Shutdown();
      base.OnExit(e);
    }
  }
}
