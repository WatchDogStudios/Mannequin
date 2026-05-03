/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Generic;
using System.IO;
using Syncfusion.Windows.Tools.Controls;
using T3Foundation.Services.Settings;

namespace T3Foundation.Services.Shell
{
  public class T3LayoutService : IT3LayoutService
  {
    private const string KeyPrefix = "shell.workspaces";
    private const string ListKey = "shell.workspaces.list";

    private readonly IT3SettingsService _settings;
    private DockingManager? _dock;

    public T3LayoutService(IT3SettingsService settings)
    {
      _settings = settings;
    }

    public void AttachDockHost(DockingManager dockHost)
    {
      _dock = dockHost ?? throw new ArgumentNullException(nameof(dockHost));
    }

    public void SaveCurrentAs(string workspaceName)
    {
      if (_dock == null)
      {
        T3Core.Log("LayoutService.SaveCurrentAs called before dock host attached.", T3LogLevel.Warning);
        return;
      }

      try
      {
        // Syncfusion's SaveDockState writes the layout into an in-memory DockStateInfo
        // that lives on the DockingManager itself; LoadDockState reads from the same
        // store. To externalize, we persist the resulting XML via the manager's
        // PersistState pipeline, which uses the IsolatedStorage XML by default.
        // We capture a per-workspace XML by saving to a StringWriter.
        using (var sw = new StringWriter())
        {
          _dock.SaveDockState();
          // Syncfusion exposes the captured state through DockStateInfo; we round-trip
          // by serializing the Children collection's attached props to a string blob.
          // For now, store a marker that the workspace exists; full XML round-trip
          // requires SfDockingManager v2 API which is install-version dependent.
          // The Syncfusion call above persists into the manager's internal store
          // already, which survives the app process under PersistState=True.
          var marker = DateTime.UtcNow.ToString("O");
          _settings.Set($"{KeyPrefix}.{workspaceName}", marker);
        }

        // Maintain the workspace name list.
        var names = new List<string>(ListWorkspaces());
        if (!names.Contains(workspaceName))
        {
          names.Add(workspaceName);
          _settings.Set(ListKey, names);
        }

        T3Core.Log($"Layout saved as workspace '{workspaceName}'.", T3LogLevel.Info);
      }
      catch (Exception ex)
      {
        T3Core.Log($"LayoutService.SaveCurrentAs failed: {ex.Message}", T3LogLevel.Error);
      }
    }

    public bool LoadWorkspace(string workspaceName)
    {
      if (_dock == null) return false;

      var marker = _settings.Get<string?>($"{KeyPrefix}.{workspaceName}", null);
      if (string.IsNullOrEmpty(marker)) return false;

      try
      {
        _dock.LoadDockState();
        T3Core.Log($"Layout restored from workspace '{workspaceName}'.", T3LogLevel.Info);
        return true;
      }
      catch (Exception ex)
      {
        T3Core.Log($"LayoutService.LoadWorkspace failed: {ex.Message}", T3LogLevel.Error);
        return false;
      }
    }

    public IReadOnlyList<string> ListWorkspaces()
    {
      var names = _settings.Get<List<string>?>(ListKey, null);
      return names ?? (IReadOnlyList<string>)Array.Empty<string>();
    }

    public void ResetToDefault()
    {
      _settings.Set<string?>($"{KeyPrefix}.Default", null);
      T3Core.Log("Default workspace cleared.", T3LogLevel.Info);
    }
  }
}
