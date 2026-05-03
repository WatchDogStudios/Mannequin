/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Threading;
using T3Foundation.Services.Dialog;

namespace T3Foundation.Wpf.Controls
{
  /// <summary>
  /// Data object representing a single toast notification.
  /// </summary>
  public class T3Notification
  {
    public string Message { get; set; } = string.Empty;
    public T3NotificationType Type { get; set; }
    public int DurationMs { get; set; }
  }

  /// <summary>
  /// Toast notification overlay control.
  /// Place at the root of your window to display slide-in/fade-out notifications.
  /// Subscribes to <see cref="IT3DialogService.OnNotificationRequested"/>.
  /// </summary>
  public class T3NotificationHost : Control
  {
    private readonly ObservableCollection<T3Notification> _notifications = new();
    private ItemsControl? _itemsControl;

    /// <summary>
    /// Maximum number of visible notifications.
    /// </summary>
    public int MaxVisible
    {
      get => (int)GetValue(MaxVisibleProperty);
      set => SetValue(MaxVisibleProperty, value);
    }

    public static readonly DependencyProperty MaxVisibleProperty =
      DependencyProperty.Register(nameof(MaxVisible), typeof(int), typeof(T3NotificationHost), new PropertyMetadata(5));

    static T3NotificationHost()
    {
      DefaultStyleKeyProperty.OverrideMetadata(typeof(T3NotificationHost),
        new FrameworkPropertyMetadata(typeof(T3NotificationHost)));
    }

    public T3NotificationHost()
    {
      HorizontalAlignment = HorizontalAlignment.Right;
      VerticalAlignment = VerticalAlignment.Bottom;
      Margin = new Thickness(0, 0, 16, 16);
      IsHitTestVisible = false;

      Loaded += (_, _) => BuildVisualTree();
    }

    /// <summary>
    /// Subscribe this host to a dialog service's notification events.
    /// </summary>
    public void SubscribeTo(IT3DialogService dialogService)
    {
      dialogService.OnNotificationRequested += OnNotificationRequested;
    }

    private void OnNotificationRequested(string message, T3NotificationType type, int durationMs)
    {
      Dispatcher.BeginInvoke(() =>
      {
        var notification = new T3Notification { Message = message, Type = type, DurationMs = durationMs };
        _notifications.Add(notification);

        while (_notifications.Count > MaxVisible)
          _notifications.RemoveAt(0);

        // Auto-remove after duration
        var timer = new DispatcherTimer { Interval = TimeSpan.FromMilliseconds(durationMs) };
        timer.Tick += (_, _) =>
        {
          _notifications.Remove(notification);
          timer.Stop();
        };
        timer.Start();
      });
    }

    private void BuildVisualTree()
    {
      _itemsControl = new ItemsControl
      {
        ItemsSource = _notifications,
        ItemTemplate = CreateNotificationTemplate()
      };

      var grid = new Grid();
      grid.Children.Add(_itemsControl);
      AddVisualChild(grid);
    }

    private DataTemplate CreateNotificationTemplate()
    {
      var factory = new FrameworkElementFactory(typeof(Border));
      factory.SetValue(Border.CornerRadiusProperty, new CornerRadius(6));
      factory.SetValue(Border.PaddingProperty, new Thickness(16, 10, 16, 10));
      factory.SetValue(Border.MarginProperty, new Thickness(0, 4, 0, 4));
      factory.SetValue(MinWidthProperty, 250d);
      factory.SetValue(MaxWidthProperty, 400d);
      factory.SetValue(Border.BackgroundProperty, new SolidColorBrush(Color.FromArgb(230, 45, 45, 48)));
      factory.SetValue(Border.BorderBrushProperty, new SolidColorBrush(Color.FromArgb(128, 100, 100, 100)));
      factory.SetValue(Border.BorderThicknessProperty, new Thickness(1));

      var textFactory = new FrameworkElementFactory(typeof(TextBlock));
      textFactory.SetBinding(TextBlock.TextProperty, new System.Windows.Data.Binding("Message"));
      textFactory.SetValue(TextBlock.ForegroundProperty, Brushes.White);
      textFactory.SetValue(TextBlock.TextWrappingProperty, TextWrapping.Wrap);
      textFactory.SetValue(TextBlock.FontSizeProperty, 12d);

      factory.AppendChild(textFactory);

      return new DataTemplate { VisualTree = factory };
    }

    protected override int VisualChildrenCount => 1;

    protected override Visual GetVisualChild(int index)
    {
      if (_itemsControl?.Parent is Grid grid)
        return grid;
      return _itemsControl ?? (Visual)new Grid();
    }

    protected override Size MeasureOverride(Size availableSize)
    {
      if (_itemsControl?.Parent is Grid grid)
      {
        grid.Measure(availableSize);
        return grid.DesiredSize;
      }
      return base.MeasureOverride(availableSize);
    }

    protected override Size ArrangeOverride(Size finalSize)
    {
      if (_itemsControl?.Parent is Grid grid)
      {
        grid.Arrange(new Rect(finalSize));
        return finalSize;
      }
      return base.ArrangeOverride(finalSize);
    }
  }
}
