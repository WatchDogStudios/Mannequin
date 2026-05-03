/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Collections.Generic;
using Syncfusion.Windows.Tools.Controls;

namespace T3Foundation.Services.Shell
{
  /// <summary>
  /// Persistence for dock layouts. Wraps Syncfusion's
  /// <c>SaveDockState()</c>/<c>LoadDockState()</c> and stores XML blobs in the
  /// app's <c>IT3SettingsService</c> under per-workspace keys.
  /// </summary>
  public interface IT3LayoutService
  {
    /// <summary>Bind the dock host whose state we'll save/restore. Idempotent.</summary>
    void AttachDockHost(DockingManager dockHost);

    /// <summary>Capture the current dock layout as an XML blob and persist under <paramref name="workspaceName"/>.</summary>
    void SaveCurrentAs(string workspaceName);

    /// <summary>Restore a previously saved workspace. Returns false when no such snapshot exists.</summary>
    bool LoadWorkspace(string workspaceName);

    /// <summary>List all workspace names currently persisted.</summary>
    IReadOnlyList<string> ListWorkspaces();

    /// <summary>Forget the persisted "Default" snapshot so next launch falls back to first-run placement.</summary>
    void ResetToDefault();
  }
}
