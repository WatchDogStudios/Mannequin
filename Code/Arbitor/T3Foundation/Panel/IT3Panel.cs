/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using T3Foundation.Context;

namespace T3Foundation.Panel
{
  /// <summary>
  /// A Panel is a "dumb" reusable visual component that displays data from a Context.
  ///
  /// Panels contain ZERO business logic. They only:
  /// 1. Display data read from the bound Context
  /// 2. Invoke Actions when the user interacts (edit, click, drag)
  ///
  /// The same Panel type can be reused across different Documents and workflows.
  /// One PropertyEditor panel, one ImageViewer panel — not 10 copies with slight variations.
  ///
  /// Based on Workflow-Driven Tool Design
  /// (See Remedy's Talk: https://www.youtube.com/watch?v=kAfb0yx07Po)
  /// </summary>
  public interface IT3Panel
  {
    /// <summary>
    /// Unique identifier for this panel instance.
    /// </summary>
    string PanelId { get; }

    /// <summary>
    /// Human-readable name for this panel type (e.g. "Property Editor", "Image Viewer").
    /// </summary>
    string PanelName { get; }

    /// <summary>
    /// The Context this panel is currently bound to, or null if unbound.
    /// </summary>
    IT3Context? BoundContext { get; }

    /// <summary>
    /// Bind this panel to a Context. The panel should subscribe to context changes
    /// and populate its visual state from the context's data.
    /// </summary>
    void Bind(IT3Context context);

    /// <summary>
    /// Unbind from the current Context. Clear visual state and unsubscribe
    /// from all change notifications.
    /// </summary>
    void Unbind();
  }
}
