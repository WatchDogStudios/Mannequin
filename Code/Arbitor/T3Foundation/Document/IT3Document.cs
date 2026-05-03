/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Collections.Generic;
using T3Foundation.Context;
using T3Foundation.Panel;

namespace T3Foundation.Documents
{
  /// <summary>
  /// A Document defines HOW data is edited by configuring which Panels
  /// appear and where they are laid out for a specific workflow.
  ///
  /// Documents do NOT contain business logic. They are workflow configurators
  /// that compose reusable Panels around a Context.
  ///
  /// Based on Workflow-Driven Tool Design
  /// (See Remedy's Talk: https://www.youtube.com/watch?v=kAfb0yx07Po)
  /// </summary>
  public interface IT3Document
  {
    /// <summary>
    /// Unique instance identifier for this document.
    /// </summary>
    string DocumentId { get; }

    /// <summary>
    /// Display name shown in tab headers.
    /// </summary>
    string DocumentName { get; }

    /// <summary>
    /// The Context this document is editing (set during <see cref="Configure"/>).
    /// </summary>
    IT3Context? Context { get; }

    /// <summary>
    /// The panel layout configuration. Describes which panels are shown and where.
    /// Populated during <see cref="Configure"/>.
    /// </summary>
    IReadOnlyList<T3PanelSlot> PanelLayout { get; }

    /// <summary>
    /// Configure this document for a given context.
    /// Called by the Workflow Manager when opening a new workflow.
    /// Implementations should add panels via the layout and bind them to the context.
    /// </summary>
    void Configure(IT3Context context);

    /// <summary>
    /// Called when this document becomes the active/focused tab.
    /// </summary>
    void OnActivated();

    /// <summary>
    /// Called when the user switches away from this document.
    /// </summary>
    void OnDeactivated();

    /// <summary>
    /// Check whether this document can be closed.
    /// Return false to prevent closing (e.g., prompt for unsaved changes).
    /// </summary>
    bool CanClose();

    /// <summary>
    /// Close this document. Unbinds all panels from the context.
    /// </summary>
    void Close();
  }
}
