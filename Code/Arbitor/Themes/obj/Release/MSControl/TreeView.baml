   M S B A M L   `   `   `  ���� k  gSyncfusion.Themes.basetheme.WPF, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=e1da1d9bc9725b9dQ-clr-namespace:Syncfusion.Themes.basetheme.WPFSyncfusion.Themes.basetheme.WPF  a ]Syncfusion.Shared.Wpf, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=3d67ed1f87d44c89dFclr-namespace:Syncfusion.Windows.Shared;assembly=Syncfusion.Shared.WPFSyncfusion.Windows.Shared h dSyncfusion.SfSkinManager.WPF, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=3d67ed1f87d44c89iLclr-namespace:Syncfusion.SfSkinManager;assembly=Syncfusion.SfSkinManager.WPFSyncfusion.SfSkinManager O Kmscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e0891&clr-namespace:System;assembly=mscorlibSystem W SPresentationCore, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NWindowsBase, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35\ XPresentationFramework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NSystem.Xaml, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089  �� MergedDictionaries9   3Syncfusion.SfSkinManager.ResourceDictionaryProvider    DictionaryName    	ThemeName �� Source0   *Syncfusion.Themes.basetheme.WPF.RevealItem   treeviewRevealItem  ContentBackgroundHovered   HoverBackground  RevealBorder   HoverBorder   RevealPrimaryLightBackground   PressedBackground  WPFTreeViewStyle �� 
TargetType �� Property	 �� Value  	BorderAlt  basetheme.BorderThickness  ContentForeground  basetheme.ThemeFontFamily 	 basetheme.BodyTextStyle 
 basetheme.FontWeightNormal
 �� 
TargetType ��Name ' #basetheme.ThemeCornerRadiusVariant2 �� CornerRadius J� SnapsToDevicePixels ��Name �� Triggers P� Property P� Value  DisabledForeground �� BasedOn  TreeViewItemFocusVisual  WPFTreeViewItemStyle �� RelativeSource �� AncestorType  basetheme.MinHeight1 � ColumnDefinitions � RowDefinitions d�Name �� 	ClickMode # WPFGlyphTreeExpanderToggleStyle d� 	IsChecked �� Padding  basetheme.CornerRadius22   ,Syncfusion.Themes.basetheme.WPF.FluentHelper   
RevealItem ��Name ��Name �� 
TargetName P� 
SourceName  HoveredForeground  ContentBackgroundSelected  SelectedForeground  ]� 
Conditions! �� Property" �� Value  ContentBackgroundInactive,  &Syncfusion.SfSkinManager.SfSkinManager�� D 9http://schemas.microsoft.com/winfx/2006/xaml/presentation    :x,http://schemas.microsoft.com/winfx/2006/xaml     <
themelocal-clr-namespace:Syncfusion.Themes.basetheme.WPF  QsharedFclr-namespace:Syncfusion.Windows.Shared;assembly=Syncfusion.Shared.WPF  WsfskinLclr-namespace:Syncfusion.SfSkinManager;assembly=Syncfusion.SfSkinManager.WPF  .sys&clr-namespace:System;assembly=mscorlib       $ Common��$ 	basetheme��   $ Brushes��$ 	basetheme���� $Q K/Syncfusion.Themes.basetheme.WPF;component/MSControl/GlyphTreeExpander.xaml?�%�	  &	        7  7  7  &	      7  7  7  7  7 	 7 
 7  7  7  'R� p    7  &	     &	 �    7  7  7  7  7  7  7  7   7  7  7  7  7  7  7  7  7  7  7  'Q� �    7    8   8  8  �� " R�.���  �@�FocusVisualStyle	 Z� ��  � ��
	 ���� ��  � ��8	   ��  � ��8	  ��  � ��8	  ��  � ��8	  ��  � ��8	  ��  � ��8	  ��  � ��$	 1q���  � ��$	 Center=���  � ��$
	 Auto=���  � ��$
	 Auto=���  �@��PanningMode$
	 Both=���  �@��IsFlicksEnabled	 .  ��  � ��	 �� "
 R�.R��� $ border��8  #��z��#��z�� . ._��� $ scrollviewer��#��z��#��z����.  |�.  #��z��#��z��# z .���� $��0q� P�  � {� .  . ���  � ��
	 ���� �� 8	   � ���� 8	   � ���� 8   " R��� .���  � ��	 �� .R��� �� " Q�.���  �@�FocusVisualStyle8	   ��  � ��
	 ���� ��  � ��8	  ��  � ��8	  ��  � ��8	  ��  � ��$	 2q���  � ��	 �� *HorizontalContentAlignment+ �� " ����  � ��	 �� *VerticalContentAlignment+ �� " ����  � ��8	  ��  � ��$	 0q���  � ��	 �� "
 Q�.R��  �� $��19��$
��Auto ��� $
��Auto ��� $��* � �� $
��Auto ��� .+�d� $ Expander��$ Press=�8��  �� *
IsExpanded+ �� *TemplatedParent+�� $ SelectionIndicator��$��1��$
��Left=�$	 1.5��$��10��$��2��$y�	Collapsed=��� $ border��$��1��#��z��#��z��#��z��# z��8   . 8  ._��� $ PART_Header��$��Header��#��z��#��z��# z $��0q��� $ 	ItemsHost��$��2��$��1��$��1��$��0q� P�  �@Q�
IsExpanded .  . ��� $ 	ItemsHost�� � y�$	 	Collapsed=�P�  �@��HasItems .  . ��� $ Expander�� � y�$	 Hidden=�P� $ PART_Header�� �@J�IsMouseOver . . ��� $ border��8	   � ���� $ border��8	 	  � ���� 8	 
  � ��P�  �@Q�
IsSelected . . ��� $ border��8	   � ���� $ border��8	   � ���� 8	   � ��]�   �� ! �@Q�
IsSelected" . �� ! �@Q�IsSelectionActive" .  ./��� $ border��8	   � ���� $ border��8	   � ���� 8	   � ��P�  �@ SizeMode$ Touch=�. ��� $ border�� � ��$	 30��P�  � {� .  . ��� $ border�� � ��
	 ���� �� $ border��8	   � ���� 8	   � ���� 8   " Q�