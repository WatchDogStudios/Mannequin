/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Data;
using CommunityToolkit.Mvvm.Input;
using T3.ViewModels;
using T3.Views.Panels;
using T3Foundation.Services.DI;
using T3Foundation.Services.Shell;
using T3Foundation.Wpf.Panels;
using T3Foundation.Wpf.Shell;

namespace T3
{
  /// <summary>
  /// Mannequin main window. Inherits the chromed title bar, dockable layout,
  /// data-driven menu/toolbar, command palette, and FrostEd x Unreal theme
  /// from <see cref="T3ShellWindow"/>. Per-app concerns are limited to the
  /// three Register* overrides below.
  /// </summary>
  public partial class MainWindow : T3ShellWindow
  {
    public MainWindow()
    {
      InitializeComponent();
    }

    protected override string DefaultWorkspaceName => "Default";

    protected override void RegisterToolWindows(IT3ToolWindowRegistry registry)
    {
      registry.Register(new T3ToolWindowDescriptor("tests", "Tests", () => new TestListPanel())
      {
        DefaultSide = T3DockSide.Left,
        DefaultWidth = 300,
        IconKey = T3Icons.Outliner,
        MenuPath = "View/Tests"
      });

      registry.Register(new T3ToolWindowDescriptor("comparison", "Image Comparison", () => new ImageComparisonPanel())
      {
        DefaultSide = T3DockSide.Document,
        IconKey = T3Icons.Viewport
      });

      registry.Register(new T3ToolWindowDescriptor("details", "Test Details", () => new TestDetailsPanel())
      {
        DefaultSide = T3DockSide.Right,
        DefaultWidth = 280,
        IconKey = T3Icons.Details,
        MenuPath = "View/Test Details"
      });

      registry.Register(new T3ToolWindowDescriptor("resources", "Resource Inspector", () => new ResourceInspectorPanel())
      {
        DefaultSide = T3DockSide.Tabbed,
        TabbedWith = "details",
        IconKey = T3Icons.Bug,
        MenuPath = "View/Resource Inspector"
      });

      registry.Register(new T3ToolWindowDescriptor("summary", "Summary", () => new SummaryPanel())
      {
        DefaultSide = T3DockSide.Tabbed,
        TabbedWith = "t3.log",
        IconKey = T3Icons.Info,
        MenuPath = "View/Summary"
      });

      // Open the default workspace panels (Mannequin's golden-path layout).
      foreach (var id in new[] { "tests", "comparison", "details", "resources", "t3.log", "summary" })
        registry.Open(id);
    }

    protected override void RegisterToolbar(IT3ToolbarService toolbar)
    {
      var vm = T3ServiceCollection.Resolve<MainViewModel>();

      toolbar.AddButton("Load Results", vm.LoadResultsFileCommand, T3Icons.Open, "Load a saved results file");
      toolbar.AddButton("Run All", vm.RunAllTestsCommand, T3Icons.Play, "Run every discovered test");
      toolbar.AddButton("Run Checked", vm.RunSelectedTestsCommand, T3Icons.Check, "Run only the checked tests");
      toolbar.AddButton("Stop", vm.StopTestsCommand, T3Icons.Stop, "Cancel the running test pass");
      toolbar.AddSeparator();

      toolbar.AddLabel("Application");
      toolbar.AddCustom(BuildBoundComboBox(
        vm,
        nameof(MainViewModel.AvailableApplications),
        nameof(MainViewModel.SelectedApplication),
        width: 240,
        displayMemberPath: "DisplayName"));
      toolbar.AddButton("Browse...", vm.BrowseApplicationCommand, T3Icons.Search, "Pick a different application binary");
      toolbar.AddButton("Refresh Tests", vm.RefreshTestsCommand, T3Icons.Refresh, "Re-discover the tests on disk");
      toolbar.AddSeparator();

      toolbar.AddButton("Update Baselines", vm.UpdateBaselinesCommand, T3Icons.Save, "Promote the current outputs to references");
      toolbar.AddSeparator();

      toolbar.AddLabel("API");
      toolbar.AddCustom(BuildBoundComboBox(
        vm,
        nameof(MainViewModel.AvailableAPIs),
        nameof(MainViewModel.SelectedAPI),
        width: 120));
      toolbar.AddSeparator();

      toolbar.AddButton("Export", vm.ExportResultsCommand, T3Icons.Save, "Export the current results");
    }

    /// <summary>
    /// Builds a ComboBox bound to a source's items + selection. Used for the
    /// Application and API toolbar dropdowns - data-driven toolbar items can be
    /// arbitrary controls via <c>AddCustom</c>.
    /// </summary>
    private static ComboBox BuildBoundComboBox(object source, string itemsPath, string selectedPath, double width, string displayMemberPath = null)
    {
      var combo = new ComboBox { Width = width, MinHeight = 28 };
      if (displayMemberPath != null) combo.DisplayMemberPath = displayMemberPath;
      combo.SetBinding(ItemsControl.ItemsSourceProperty, new Binding(itemsPath) { Source = source });
      combo.SetBinding(Selector.SelectedItemProperty, new Binding(selectedPath) { Source = source, Mode = BindingMode.TwoWay });
      return combo;
    }

    protected override void RegisterMenu(IT3MenuService menu)
    {
      var vm = T3ServiceCollection.Resolve<MainViewModel>();

      menu.AddMenu("File")
        .AddItem("Load Results...", vm.LoadResultsFileCommand, iconKey: T3Icons.Open)
        .AddItem("Export...", vm.ExportResultsCommand, iconKey: T3Icons.Save)
        .AddSeparator()
        .AddItem("Exit", new RelayCommand(() => Application.Current.Shutdown()), iconKey: T3Icons.Times);

      menu.AddMenu("Run")
        .AddItem("Run All", vm.RunAllTestsCommand, iconKey: T3Icons.Play)
        .AddItem("Run Checked", vm.RunSelectedTestsCommand, iconKey: T3Icons.Check)
        .AddItem("Stop", vm.StopTestsCommand, iconKey: T3Icons.Stop)
        .AddSeparator()
        .AddItem("Refresh Tests", vm.RefreshTestsCommand, iconKey: T3Icons.Refresh)
        .AddItem("Update Baselines", vm.UpdateBaselinesCommand, iconKey: T3Icons.Save);

      // View menu - one entry per registered tool window.
      var registry = T3ServiceCollection.Resolve<IT3ToolWindowRegistry>();
      var view = menu.AddMenu("View");
      foreach (var desc in registry.All)
      {
        var captured = desc;
        view.AddItem(captured.Title, new RelayCommand(() => registry.Toggle(captured.Id)), iconKey: captured.IconKey);
      }
    }
  }
}
