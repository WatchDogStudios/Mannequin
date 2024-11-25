/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

using System.ComponentModel.Composition;
using System.ComponentModel.Composition.Hosting;
using System.Configuration;
using System.IO;
namespace T3Foundation
{
  /// <summary>
  /// Interface to serve as the base for UserControls to use for plugging into any T3 Application.
  /// See: https://www.c-sharpcorner.com/article/plugin-architecture-using-mef-framework/ for concept inspiration. 
  /// </summary>
  public interface IT3Plugin
  {
    /// <summary>
    /// Name of the plugin. This will display on the T3 Application. the end-application can change this behavior if it wishes.
    /// </summary>
    string PluginName { get; set; }

    /// <summary>
    /// Get the user control of the loaded plugin.
    /// </summary>
    /// <returns>The UserControl object.</returns>
    UserControl GetPluginControl();
  }

  /// <summary>
  /// struct for holding needed objects to allow T3 Plugins to work with T3 Applications.
  /// Any T3 Application REQUIRES this to be created within the MainWindow Class.
  /// </summary>
  public struct ST3PluginAppObjects
  {
    public CompositionContainer _container;

    [ImportMany(typeof(IT3Plugin))]
    public IEnumerable<Lazy<IT3Plugin>> _pluggers;
  }

  public static class CT3PluginUtils
  {
    /// <summary>
    /// Loads the plugin view from said Plugin DLL using MEF.
    /// This doesn't actually load the UI that is viewable to the user, you must load that your self.
    /// </summary>
    /// NOTE: Revamp this class, it is not clear.
    [Obsolete]
    public static void LoadPluginView(ST3PluginAppObjects appobj)
    {
      string plugName = ConfigurationSettings.AppSettings["Plugin"].ToString();
      var connectors = Directory.GetDirectories(plugName);
      var catalog = new AggregateCatalog();
      connectors.ToList().ForEach(connect => catalog.Catalogs.Add(new DirectoryCatalog(connect)));
      appobj._container = new CompositionContainer(catalog);
      appobj._container.ComposeParts(appobj._container);
    }
  }
}
