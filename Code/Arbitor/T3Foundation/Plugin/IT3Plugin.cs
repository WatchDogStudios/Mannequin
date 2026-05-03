/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using Microsoft.Extensions.DependencyInjection;

namespace T3Foundation.Plugin
{
  /// <summary>
  /// Modern plugin interface for T3 applications.
  /// Plugins register their services into DI and have a defined lifecycle.
  /// </summary>
  public interface IT3Plugin
  {
    /// <summary>
    /// Unique identifier for this plugin.
    /// </summary>
    string PluginId { get; }

    /// <summary>
    /// Human-readable display name.
    /// </summary>
    string PluginName { get; }

    /// <summary>
    /// Plugin version string (e.g. "1.0.0").
    /// </summary>
    string Version { get; }

    /// <summary>
    /// Short description of the plugin's purpose.
    /// </summary>
    string Description { get; }

    /// <summary>
    /// Called during application startup to register the plugin's services into the DI container.
    /// </summary>
    void RegisterServices(IServiceCollection services);

    /// <summary>
    /// Called after the DI container is built. Use for post-initialization that requires resolved services.
    /// </summary>
    void Initialize(IServiceProvider provider);

    /// <summary>
    /// Called during application shutdown for graceful cleanup.
    /// </summary>
    void Shutdown();
  }

  /// <summary>
  /// Marks a class as a T3 plugin for automatic assembly scanning discovery.
  /// </summary>
  [AttributeUsage(AttributeTargets.Class, AllowMultiple = false, Inherited = false)]
  public class T3PluginAttribute : Attribute
  {
  }
}
