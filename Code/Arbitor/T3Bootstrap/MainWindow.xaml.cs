using Syncfusion.SfSkinManager;
using System;
using System.Windows;

namespace T3
{
  /// <summary>
  /// Interaction logic for MainWindow.xaml
  /// </summary>
  public partial class MainWindow : Window
  {
    public MainWindow()
    {
      // TODO: Move this to T3Foundation!
      string style = "basetheme";
      SkinHelper styleInstance = null;
      var skinHelpterStr = "Syncfusion.Themes." + style + ".WPF." + style + "SkinHelper, Syncfusion.Themes." + style + ".WPF";
      Type skinHelpterType = Type.GetType(skinHelpterStr);
      if (skinHelpterType != null)
        styleInstance = Activator.CreateInstance(skinHelpterType) as SkinHelper;
      if (styleInstance != null)
      {
        SfSkinManager.RegisterTheme("basetheme", styleInstance);
      }
      SfSkinManager.SetTheme(this, new Theme("basetheme;MaterialDark"));
      InitializeComponent();
    }
  }
}
