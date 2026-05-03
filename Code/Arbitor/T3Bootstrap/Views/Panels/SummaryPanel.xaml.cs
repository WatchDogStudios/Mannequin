/*
 *   Copyright (c) 2023-present WD Studios L.L.C.
 *   All rights reserved.
 */

using System.Windows.Controls;
using T3.ViewModels;
using T3Foundation.Services.DI;

namespace T3.Views.Panels
{
  public partial class SummaryPanel : UserControl
  {
    public SummaryPanel()
    {
      InitializeComponent();
      DataContext = T3ServiceCollection.Resolve<MainViewModel>();
    }
  }
}
