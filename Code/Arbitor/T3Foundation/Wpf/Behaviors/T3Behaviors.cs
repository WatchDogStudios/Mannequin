/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 *   You are only allowed access to this code, if given WRITTEN permission by Watch Dogs LLC.
 */

#nullable enable

using System;
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;

namespace T3Foundation.Wpf.Behaviors
{
  /// <summary>
  /// Attached behaviors for common WPF patterns.
  /// </summary>
  public static class T3Behaviors
  {
    // ───────────────────────────────── Watermark ─────────────────────────────────

    /// <summary>
    /// Attached property to show placeholder/watermark text on an empty TextBox.
    /// </summary>
    public static readonly DependencyProperty WatermarkProperty =
      DependencyProperty.RegisterAttached("Watermark", typeof(string), typeof(T3Behaviors),
        new PropertyMetadata(null, OnWatermarkChanged));

    public static string GetWatermark(DependencyObject obj) => (string)obj.GetValue(WatermarkProperty);
    public static void SetWatermark(DependencyObject obj, string value) => obj.SetValue(WatermarkProperty, value);

    private static void OnWatermarkChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
      if (d is not TextBox textBox) return;

      textBox.GotFocus -= Watermark_GotFocus;
      textBox.LostFocus -= Watermark_LostFocus;
      textBox.TextChanged -= Watermark_TextChanged;

      if (e.NewValue is string)
      {
        textBox.GotFocus += Watermark_GotFocus;
        textBox.LostFocus += Watermark_LostFocus;
        textBox.TextChanged += Watermark_TextChanged;
        UpdateWatermarkVisual(textBox);
      }
    }

    private static void Watermark_GotFocus(object sender, RoutedEventArgs e) => UpdateWatermarkVisual((TextBox)sender);
    private static void Watermark_LostFocus(object sender, RoutedEventArgs e) => UpdateWatermarkVisual((TextBox)sender);
    private static void Watermark_TextChanged(object sender, TextChangedEventArgs e) => UpdateWatermarkVisual((TextBox)sender);

    private static void UpdateWatermarkVisual(TextBox textBox)
    {
      var layer = AdornerLayer.GetAdornerLayer(textBox);
      if (layer == null) return;

      // Remove existing watermark adorners
      var adorners = layer.GetAdorners(textBox);
      if (adorners != null)
      {
        foreach (var adorner in adorners)
        {
          if (adorner is WatermarkAdorner)
            layer.Remove(adorner);
        }
      }

      // Show watermark if empty and not focused
      if (string.IsNullOrEmpty(textBox.Text) && !textBox.IsFocused)
      {
        var watermarkText = GetWatermark(textBox);
        if (!string.IsNullOrEmpty(watermarkText))
          layer.Add(new WatermarkAdorner(textBox, watermarkText));
      }
    }

    private class WatermarkAdorner : Adorner
    {
      private readonly TextBlock _textBlock;

      public WatermarkAdorner(UIElement adornedElement, string watermark) : base(adornedElement)
      {
        IsHitTestVisible = false;
        _textBlock = new TextBlock
        {
          Text = watermark,
          Foreground = new SolidColorBrush(Color.FromArgb(128, 160, 160, 160)),
          Margin = new Thickness(4, 2, 0, 0),
          FontStyle = FontStyles.Italic
        };
      }

      protected override int VisualChildrenCount => 1;
      protected override Visual GetVisualChild(int index) => _textBlock;

      protected override Size MeasureOverride(Size constraint)
      {
        _textBlock.Measure(constraint);
        return _textBlock.DesiredSize;
      }

      protected override Size ArrangeOverride(Size finalSize)
      {
        _textBlock.Arrange(new Rect(finalSize));
        return finalSize;
      }
    }

    // ───────────────────────────────── Focus ─────────────────────────────────

    /// <summary>
    /// Bindable attached property for focus state.
    /// </summary>
    public static readonly DependencyProperty IsFocusedProperty =
      DependencyProperty.RegisterAttached("IsFocused", typeof(bool), typeof(T3Behaviors),
        new FrameworkPropertyMetadata(false, FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, OnIsFocusedChanged));

    public static bool GetIsFocused(DependencyObject obj) => (bool)obj.GetValue(IsFocusedProperty);
    public static void SetIsFocused(DependencyObject obj, bool value) => obj.SetValue(IsFocusedProperty, value);

    private static void OnIsFocusedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
      if (d is not UIElement element) return;
      if (e.NewValue is true)
        element.Focus();
    }

    // ───────────────────────────────── AutoScroll ─────────────────────────────────

    /// <summary>
    /// Attached property that auto-scrolls a ListBox or ScrollViewer to the bottom when new items are added.
    /// </summary>
    public static readonly DependencyProperty AutoScrollProperty =
      DependencyProperty.RegisterAttached("AutoScroll", typeof(bool), typeof(T3Behaviors),
        new PropertyMetadata(false, OnAutoScrollChanged));

    public static bool GetAutoScroll(DependencyObject obj) => (bool)obj.GetValue(AutoScrollProperty);
    public static void SetAutoScroll(DependencyObject obj, bool value) => obj.SetValue(AutoScrollProperty, value);

    private static void OnAutoScrollChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
      if (d is ListBox listBox)
      {
        if (e.NewValue is true)
        {
          ((INotifyCollectionChanged)listBox.Items).CollectionChanged += (s, args) =>
          {
            if (args.Action == NotifyCollectionChangedAction.Add && listBox.Items.Count > 0)
              listBox.ScrollIntoView(listBox.Items[listBox.Items.Count - 1]);
          };
        }
      }
    }

    // ───────────────────────────────── DragDrop ─────────────────────────────────

    /// <summary>
    /// Enables file drag-and-drop on an element. The dropped file paths are passed
    /// to the command specified by <see cref="FileDropCommandProperty"/>.
    /// </summary>
    public static readonly DependencyProperty AllowFileDropProperty =
      DependencyProperty.RegisterAttached("AllowFileDrop", typeof(bool), typeof(T3Behaviors),
        new PropertyMetadata(false, OnAllowFileDropChanged));

    public static bool GetAllowFileDrop(DependencyObject obj) => (bool)obj.GetValue(AllowFileDropProperty);
    public static void SetAllowFileDrop(DependencyObject obj, bool value) => obj.SetValue(AllowFileDropProperty, value);

    /// <summary>
    /// Command to execute when files are dropped. Receives string[] of file paths.
    /// </summary>
    public static readonly DependencyProperty FileDropCommandProperty =
      DependencyProperty.RegisterAttached("FileDropCommand", typeof(ICommand), typeof(T3Behaviors));

    public static ICommand GetFileDropCommand(DependencyObject obj) => (ICommand)obj.GetValue(FileDropCommandProperty);
    public static void SetFileDropCommand(DependencyObject obj, ICommand value) => obj.SetValue(FileDropCommandProperty, value);

    private static void OnAllowFileDropChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
      if (d is not UIElement element) return;

      element.AllowDrop = e.NewValue is true;
      if (e.NewValue is true)
      {
        element.DragOver += FileDrop_DragOver;
        element.Drop += FileDrop_Drop;
      }
      else
      {
        element.DragOver -= FileDrop_DragOver;
        element.Drop -= FileDrop_Drop;
      }
    }

    private static void FileDrop_DragOver(object sender, DragEventArgs e)
    {
      e.Effects = e.Data.GetDataPresent(DataFormats.FileDrop) ? DragDropEffects.Copy : DragDropEffects.None;
      e.Handled = true;
    }

    private static void FileDrop_Drop(object sender, DragEventArgs e)
    {
      if (e.Data.GetDataPresent(DataFormats.FileDrop) && sender is DependencyObject d)
      {
        var files = (string[])e.Data.GetData(DataFormats.FileDrop)!;
        var command = GetFileDropCommand(d);
        if (command?.CanExecute(files) == true)
          command.Execute(files);
      }
    }

    // ───────────────────────────────── SelectAllOnFocus ─────────────────────────────────

    /// <summary>
    /// Selects all text in a TextBox when it receives focus.
    /// </summary>
    public static readonly DependencyProperty SelectAllOnFocusProperty =
      DependencyProperty.RegisterAttached("SelectAllOnFocus", typeof(bool), typeof(T3Behaviors),
        new PropertyMetadata(false, OnSelectAllOnFocusChanged));

    public static bool GetSelectAllOnFocus(DependencyObject obj) => (bool)obj.GetValue(SelectAllOnFocusProperty);
    public static void SetSelectAllOnFocus(DependencyObject obj, bool value) => obj.SetValue(SelectAllOnFocusProperty, value);

    private static void OnSelectAllOnFocusChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
    {
      if (d is not TextBox textBox) return;

      if (e.NewValue is true)
        textBox.GotFocus += SelectAll_GotFocus;
      else
        textBox.GotFocus -= SelectAll_GotFocus;
    }

    private static void SelectAll_GotFocus(object sender, RoutedEventArgs e)
    {
      if (sender is TextBox tb)
        tb.SelectAll();
    }
  }
}
