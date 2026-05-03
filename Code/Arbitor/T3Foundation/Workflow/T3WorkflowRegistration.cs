/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using T3Foundation.Context;
using T3Foundation.Documents;

namespace T3Foundation.Workflow
{
  /// <summary>
  /// Fluent helper for registering workflows.
  /// Supports conditional registrations based on Context state.
  ///
  /// <code>
  /// var registration = new T3WorkflowRegistration(workflowManager);
  /// registration.For&lt;AssetContext&gt;().Use&lt;AssetEditorDocument&gt;();
  /// registration.For&lt;AssetContext&gt;().When(c => c.IsReadOnly).Use&lt;AssetViewerDocument&gt;();
  /// </code>
  /// </summary>
  public class T3WorkflowRegistration
  {
    private readonly IT3WorkflowManager _manager;

    public T3WorkflowRegistration(IT3WorkflowManager manager)
    {
      _manager = manager;
    }

    /// <summary>
    /// Begin a registration for a specific Context type.
    /// </summary>
    public ContextRegistration<TContext> For<TContext>() where TContext : IT3Context
    {
      return new ContextRegistration<TContext>(_manager);
    }

    public class ContextRegistration<TContext> where TContext : IT3Context
    {
      private readonly IT3WorkflowManager _manager;
      private Func<IT3Context, bool>? _predicate;

      internal ContextRegistration(IT3WorkflowManager manager)
      {
        _manager = manager;
      }

      /// <summary>
      /// Add a condition that must be true for this workflow to be selected.
      /// </summary>
      public ContextRegistration<TContext> When(Func<TContext, bool> predicate)
      {
        _predicate = ctx => ctx is TContext typed && predicate(typed);
        return this;
      }

      /// <summary>
      /// Complete the registration with the Document type to use.
      /// </summary>
      public void Use<TDocument>() where TDocument : IT3Document
      {
        if (_predicate != null)
          _manager.RegisterWorkflow(typeof(TContext), typeof(TDocument), _predicate);
        else
          _manager.RegisterWorkflow<TContext, TDocument>();
      }
    }
  }
}
