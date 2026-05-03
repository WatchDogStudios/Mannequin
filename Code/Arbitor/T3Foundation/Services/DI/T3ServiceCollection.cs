/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using Microsoft.Extensions.DependencyInjection;

namespace T3Foundation.Services.DI
{
  /// <summary>
  /// Lightweight DI container wrapper for T3 applications.
  /// Call <see cref="Configure"/> at startup, then <see cref="Build"/> to freeze registration.
  /// </summary>
  public static class T3ServiceCollection
  {
    private static IServiceCollection? s_services;
    private static IServiceProvider? s_provider;
    private static bool s_built;

    /// <summary>
    /// The raw service collection. Available only before <see cref="Build"/> is called.
    /// </summary>
    public static IServiceCollection Services
    {
      get
      {
        if (s_services == null)
          throw new InvalidOperationException("T3ServiceCollection.Configure() has not been called.");
        if (s_built)
          throw new InvalidOperationException("Cannot modify services after Build() has been called.");
        return s_services;
      }
    }

    /// <summary>
    /// The built service provider. Available only after <see cref="Build"/> is called.
    /// </summary>
    public static IServiceProvider Provider
    {
      get
      {
        if (s_provider == null)
          throw new InvalidOperationException("T3ServiceCollection.Build() has not been called.");
        return s_provider;
      }
    }

    /// <summary>
    /// Initialize the service collection and run the configuration delegate.
    /// </summary>
    public static void Configure(Action<IServiceCollection> configure)
    {
      if (s_built)
        throw new InvalidOperationException("Cannot reconfigure after Build() has been called.");

      s_services ??= new ServiceCollection();
      configure?.Invoke(s_services);
    }

    /// <summary>
    /// Build the service provider, freezing all registrations.
    /// </summary>
    public static void Build()
    {
      if (s_services == null)
        throw new InvalidOperationException("T3ServiceCollection.Configure() has not been called.");

      var serviceProvider = s_services.BuildServiceProvider();
      s_provider = serviceProvider;
      s_built = true;
      T3Core.Log("DI container built.", T3LogLevel.Info);
    }

    /// <summary>
    /// Resolve a required service. Throws if not registered.
    /// </summary>
    public static T Resolve<T>() where T : notnull
    {
      return Provider.GetRequiredService<T>();
    }

    /// <summary>
    /// Resolve an optional service. Returns default(T) if not registered.
    /// </summary>
    public static T? ResolveOptional<T>() where T : class
    {
      return Provider.GetService<T>();
    }

    /// <summary>
    /// Reset the container (for testing or application restart).
    /// </summary>
    internal static void Reset()
    {
      if (s_provider is IDisposable disposable)
        disposable.Dispose();

      s_services = null;
      s_provider = null;
      s_built = false;
    }
  }
}
