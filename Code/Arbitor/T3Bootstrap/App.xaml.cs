/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using Microsoft.Extensions.DependencyInjection;
using T3.Services;
using T3.ViewModels;
using T3Foundation.Application;
using T3Foundation.Commands;
using T3Foundation.Services.Dialog;
using T3Foundation.Services.Settings;

namespace T3
{
  /// <summary>
  /// Mannequin application entry point.
  /// Inherits from T3WpfApplication for automatic framework initialization.
  /// </summary>
  public partial class App : T3WpfApplication
  {
    public App()
    {
      ApplicationName = "Mannequin";
      ManagedApplication = true;
    }

    protected override void ConfigureServices(IServiceCollection services)
    {
      // Mannequin-specific services
      services.AddSingleton<T3TestRunnerService>(sp =>
        new T3TestRunnerService(sp.GetRequiredService<IT3SettingsService>()));
      services.AddSingleton<T3CommandManager>();

      // ViewModels - Main is Singleton so the shell window and every extracted
      // panel share the same instance (each panel resolves it from DI).
      services.AddSingleton<MainViewModel>();
      services.AddTransient<SettingsViewModel>();
    }

    protected override void OnReady()
    {
      var mainVm = Services.GetRequiredService<MainViewModel>();
      mainVm.InitializeAsync();

      var mainWindow = new MainWindow
      {
        DataContext = mainVm
      };

      // Apply theme to the window
      Theme.ApplyTheme(GetDefaultTheme(), mainWindow);

      // Register and apply keyboard shortcuts
      var commandManager = Services.GetRequiredService<T3CommandManager>();
      mainVm.RegisterShortcuts();
      commandManager.ApplyTo(mainWindow);

      // Set up notification host
      var dialogService = Services.GetRequiredService<IT3DialogService>();

      mainWindow.Show();
    }

    protected override string GetDefaultTheme() => "MaterialDark";
  }
}
