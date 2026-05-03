/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Windows;
using Microsoft.Extensions.DependencyInjection;
using T3Foundation.Services.Shell;

namespace T3Foundation.Plugin
{
  /// <summary>
  /// Discovers and manages plugins via assembly scanning.
  /// Replaces the previous MEF-based system with a simpler, DI-integrated approach.
  /// </summary>
  public class T3PluginManager
  {
    private readonly List<IT3Plugin> _plugins = new();

    /// <summary>
    /// All discovered and loaded plugins.
    /// </summary>
    public IReadOnlyList<IT3Plugin> LoadedPlugins => _plugins;

    /// <summary>
    /// Scan a directory for assemblies containing types marked with <see cref="T3PluginAttribute"/>.
    /// </summary>
    public void DiscoverPlugins(string pluginDirectory)
    {
      if (!Directory.Exists(pluginDirectory))
      {
        T3Core.Log($"Plugin directory not found: {pluginDirectory}", T3LogLevel.Warning);
        return;
      }

      var dllFiles = Directory.GetFiles(pluginDirectory, "*.dll", SearchOption.AllDirectories);

      foreach (var dll in dllFiles)
      {
        try
        {
          var assembly = Assembly.LoadFrom(dll);

          var pluginTypes = assembly.GetTypes()
            .Where(t => t.GetCustomAttribute<T3PluginAttribute>() != null
                     && typeof(IT3Plugin).IsAssignableFrom(t)
                     && !t.IsAbstract);

          foreach (var pluginType in pluginTypes)
          {
            if (Activator.CreateInstance(pluginType) is IT3Plugin plugin)
            {
              _plugins.Add(plugin);
              T3Core.Log($"Discovered plugin: {plugin.PluginName} v{plugin.Version} ({plugin.PluginId})", T3LogLevel.Info);
            }
          }
        }
        catch (ReflectionTypeLoadException ex)
        {
          // Some types may fail to load — log loader exceptions and continue
          var loaderMessages = string.Join("; ", ex.LoaderExceptions?.Select(e => e?.Message) ?? Array.Empty<string>());
          T3Core.Log($"Partial load of '{Path.GetFileName(dll)}': {loaderMessages}", T3LogLevel.Warning);
        }
        catch (Exception ex)
        {
          T3Core.Log($"Failed to load plugin assembly '{Path.GetFileName(dll)}': {ex.Message}", T3LogLevel.Warning);
        }
      }

      T3Core.Log($"Plugin discovery complete. {_plugins.Count} plugin(s) found.", T3LogLevel.Info);
    }

    /// <summary>
    /// Call <see cref="IT3Plugin.RegisterServices"/> on all discovered plugins.
    /// </summary>
    public void RegisterAll(IServiceCollection services)
    {
      foreach (var plugin in _plugins)
      {
        try
        {
          plugin.RegisterServices(services);
          T3Core.Log($"Plugin '{plugin.PluginName}' registered services.", T3LogLevel.Debug);
        }
        catch (Exception ex)
        {
          T3Core.Log($"Plugin '{plugin.PluginName}' failed to register services: {ex.Message}", T3LogLevel.Error);
        }
      }
    }

    /// <summary>
    /// Call <see cref="IT3Plugin.Initialize"/> on all discovered plugins.
    /// </summary>
    public void InitializeAll(IServiceProvider provider)
    {
      foreach (var plugin in _plugins)
      {
        try
        {
          plugin.Initialize(provider);
          T3Core.Log($"Plugin '{plugin.PluginName}' initialized.", T3LogLevel.Debug);
        }
        catch (Exception ex)
        {
          T3Core.Log($"Plugin '{plugin.PluginName}' failed to initialize: {ex.Message}", T3LogLevel.Error);
        }
      }
    }

    /// <summary>
    /// Scan all currently-loaded assemblies for types decorated with
    /// <see cref="T3ToolWindowAttribute"/> and register each with the supplied
    /// <paramref name="registry"/>. Types must derive from
    /// <see cref="FrameworkElement"/> and have a parameterless constructor.
    /// </summary>
    public void DiscoverToolWindows(IT3ToolWindowRegistry registry)
    {
      foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
      {
        // Skip system / framework / Syncfusion / NuGet assemblies that we know
        // can't carry [T3ToolWindow]. Cheap pre-filter that also avoids the
        // ReflectionTypeLoadException noise from BCL assemblies.
        if (assembly.IsDynamic) continue;
        var name = assembly.GetName().Name;
        if (name == null) continue;
        if (name.StartsWith("System") ||
            name.StartsWith("mscorlib") ||
            name.StartsWith("Microsoft.") ||
            name.StartsWith("PresentationCore") ||
            name.StartsWith("PresentationFramework") ||
            name.StartsWith("WindowsBase") ||
            name.StartsWith("Syncfusion.") ||
            name.StartsWith("AdonisUI") ||
            name.StartsWith("FontAwesome") ||
            name.StartsWith("CommunityToolkit.") ||
            name.StartsWith("Newtonsoft."))
          continue;

        Type[] types;
        try
        {
          types = assembly.GetTypes();
        }
        catch (ReflectionTypeLoadException ex)
        {
          types = (ex.Types ?? Array.Empty<Type>()).Where(t => t != null).ToArray()!;
        }
        catch
        {
          continue;
        }

        foreach (var type in types)
        {
          // Each type is in its own try/catch because reflection on partially-loaded
          // types (after a ReflectionTypeLoadException) can throw on attribute access,
          // assignability checks, or even type identity comparisons.
          try
          {
            if (type == null) continue;
            var attr = type.GetCustomAttribute<T3ToolWindowAttribute>();
            if (attr == null) continue;
            if (!typeof(FrameworkElement).IsAssignableFrom(type)) continue;
            if (type.IsAbstract) continue;

            var ctor = type.GetConstructor(Type.EmptyTypes);
            if (ctor == null)
            {
              T3Core.Log($"[T3ToolWindow] type '{type.FullName}' has no parameterless ctor; skipped.", T3LogLevel.Warning);
              continue;
            }

            var capturedType = type;
            var descriptor = new T3ToolWindowDescriptor(
              attr.Id,
              attr.Title,
              () => (FrameworkElement)Activator.CreateInstance(capturedType)!)
            {
              DefaultSide = attr.DefaultSide,
              IconKey = attr.IconKey,
              TabbedWith = attr.TabbedWith,
              DefaultWidth = attr.DefaultWidth,
              DefaultHeight = attr.DefaultHeight,
              IsSingleton = attr.IsSingleton,
              MenuPath = attr.MenuPath
            };

            registry.Register(descriptor);
          }
          catch (Exception ex)
          {
            T3Core.Log($"[T3ToolWindow] discovery skipped '{type?.FullName ?? "<null>"}': {ex.Message}", T3LogLevel.Debug);
          }
        }
      }
    }

    /// <summary>
    /// Call <see cref="IT3Plugin.Shutdown"/> on all discovered plugins.
    /// </summary>
    public void ShutdownAll()
    {
      foreach (var plugin in _plugins)
      {
        try
        {
          plugin.Shutdown();
          T3Core.Log($"Plugin '{plugin.PluginName}' shut down.", T3LogLevel.Debug);
        }
        catch (Exception ex)
        {
          T3Core.Log($"Plugin '{plugin.PluginName}' failed to shut down: {ex.Message}", T3LogLevel.Error);
        }
      }
    }
  }
}
