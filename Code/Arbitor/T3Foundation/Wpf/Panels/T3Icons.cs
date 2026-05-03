/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

namespace T3Foundation.Wpf.Panels
{
  /// <summary>
  /// Named icon constants used by the framework's built-in panels and
  /// recommended for app-defined tool windows. Values are FontAwesome5 enum
  /// names parsed by <see cref="MarkupExtensions.T3IconExtension"/> and
  /// <see cref="Converters.T3IconKeyConverter"/>.
  /// <para>
  /// Treat this as the icon API: external code should reference
  /// <c>T3Icons.Outliner</c> rather than the raw "Solid_Sitemap" string so we
  /// can swap to a custom T3 icon font in v2 with a single-file change.
  /// </para>
  /// </summary>
  public static class T3Icons
  {
    // Editor panels
    public const string Outliner = "Solid_Sitemap";
    public const string Details = "Solid_SlidersH";
    public const string ContentBrowser = "Solid_FolderOpen";
    public const string Log = "Solid_Stream";
    public const string CommandPalette = "Solid_Terminal";
    public const string Viewport = "Solid_Cube";
    public const string NodeGraph = "Solid_ProjectDiagram";

    // Common actions
    public const string Open = "Solid_FolderOpen";
    public const string Save = "Solid_Save";
    public const string Search = "Solid_Search";
    public const string Filter = "Solid_Filter";
    public const string Clear = "Solid_Broom";
    public const string Refresh = "Solid_SyncAlt";
    public const string Play = "Solid_Play";
    public const string Pause = "Solid_Pause";
    public const string Stop = "Solid_Stop";
    public const string Plus = "Solid_Plus";
    public const string Minus = "Solid_Minus";
    public const string Cog = "Solid_Cog";
    public const string Times = "Solid_Times";
    public const string Check = "Solid_Check";
    public const string Warning = "Solid_ExclamationTriangle";
    public const string Error = "Solid_TimesCircle";
    public const string Info = "Solid_InfoCircle";
    public const string Bug = "Solid_Bug";
  }
}
