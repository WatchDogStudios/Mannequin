/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;

namespace T3Foundation.Wpf.Shell
{
  /// <summary>
  /// Footer strip for <see cref="T3ShellWindow"/>. Three regions:
  /// <list type="bullet">
  ///   <item>left: status text bound from the shell VM</item>
  ///   <item>center: tail line of <see cref="T3Core.OnLogMessage"/></item>
  ///   <item>right: an accessory text (API selector, dirty marker, etc.)</item>
  /// </list>
  /// Subscribes to log events on the UI thread; auto-unsubscribes when unloaded.
  /// </summary>
  public class T3StatusBar : Control
  {
    public static readonly DependencyProperty StatusTextProperty =
      DependencyProperty.Register(nameof(StatusText), typeof(string), typeof(T3StatusBar),
        new PropertyMetadata(string.Empty));

    public static readonly DependencyProperty AccessoryTextProperty =
      DependencyProperty.Register(nameof(AccessoryText), typeof(string), typeof(T3StatusBar),
        new PropertyMetadata(string.Empty));

    public static readonly DependencyProperty LogTailTextProperty =
      DependencyProperty.Register(nameof(LogTailText), typeof(string), typeof(T3StatusBar),
        new PropertyMetadata(string.Empty));

    public string StatusText
    {
      get => (string)GetValue(StatusTextProperty);
      set => SetValue(StatusTextProperty, value);
    }

    public string AccessoryText
    {
      get => (string)GetValue(AccessoryTextProperty);
      set => SetValue(AccessoryTextProperty, value);
    }

    public string LogTailText
    {
      get => (string)GetValue(LogTailTextProperty);
      private set => SetValue(LogTailTextProperty, value);
    }

    static T3StatusBar()
    {
      DefaultStyleKeyProperty.OverrideMetadata(
        typeof(T3StatusBar),
        new FrameworkPropertyMetadata(typeof(T3StatusBar)));
    }

    public T3StatusBar()
    {
      Loaded += OnLoaded;
      Unloaded += OnUnloaded;
    }

    private void OnLoaded(object sender, RoutedEventArgs e)
    {
      T3Core.OnLogMessage += OnLogMessage;
    }

    private void OnUnloaded(object sender, RoutedEventArgs e)
    {
      T3Core.OnLogMessage -= OnLogMessage;
    }

    private void OnLogMessage(string message, T3LogLevel level)
    {
      // Dispatch to UI thread; log events can fire from any thread.
      if (Dispatcher.CheckAccess())
        LogTailText = message;
      else
        Dispatcher.BeginInvoke(DispatcherPriority.Background, new System.Action(() => LogTailText = message));
    }
  }
}
