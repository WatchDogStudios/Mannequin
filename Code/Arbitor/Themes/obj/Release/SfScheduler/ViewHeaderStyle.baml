   M S B A M L   `   `   `  ���� k  gSyncfusion.Themes.basetheme.WPF, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=e1da1d9bc9725b9dQ-clr-namespace:Syncfusion.Themes.basetheme.WPFSyncfusion.Themes.basetheme.WPF  O Kmscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e0891&clr-namespace:System;assembly=mscorlibSystem h dSyncfusion.SfSkinManager.WPF, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=3d67ed1f87d44c89iLclr-namespace:Syncfusion.SfSkinManager;assembly=Syncfusion.SfSkinManager.WPFSyncfusion.SfSkinManager f bSyncfusion.SfScheduler.WPF, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=3d67ed1f87d44c89oNclr-namespace:Syncfusion.UI.Xaml.Scheduler;assembly=Syncfusion.SfScheduler.WPFSyncfusion.UI.Xaml.Scheduler W SPresentationCore, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NWindowsBase, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35\ XPresentationFramework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NSystem.Xaml, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089  �� MergedDictionaries9   3Syncfusion.SfSkinManager.ResourceDictionaryProvider    DictionaryName    	ThemeName 0  ,SfScheduler.TodayHighlight.Static.Background �� Color;  5Syncfusion.UI.Xaml.Scheduler.TodayBorderSizeConverter  TodayBorderSizeConverter  basetheme.HeaderTextStyle   FontSize $  SyncfusionViewHeaderControlStyle �� 
TargetType4  .Syncfusion.UI.Xaml.Scheduler.ViewHeaderControl �� Property  ContentBackground �� Value  	BorderAlt �� 
TargetType	 � ColumnDefinitions
 ��Name! �� IsDeferredScrollingEnabled � 	Resources �Name � RowDefinitions6  0Syncfusion.UI.Xaml.Scheduler.ResourceHeaderPanel  Name5  /Syncfusion.UI.Xaml.Scheduler.ViewHeaderRowPanel  Name �� BasedOn ! SyncfusionViewHeaderBaseStyle1  +Syncfusion.UI.Xaml.Scheduler.ViewHeaderBase �� Triggers P� Property P� Value  PrimaryColorForeground  ContentForeground 	 basetheme.ThemeFontFamily 
 basetheme.TitleTextStyle  basetheme.FontWeightNormal � DataContext  DayViewHeaderFluentTemplate ��Name ��Name �� TextAlignment ^� 
Conditions �� Binding �� ElementName �� Path �� Value �� Triggers �� Value  �� Binding! �� 
TargetName  DisabledForeground  ContentBackgroundHovered 
 Border  PrimaryColorDark1   SyncfusionDayViewHeaderStyle0  *Syncfusion.UI.Xaml.Scheduler.DayViewHeader  basetheme.BorderThickness" ��Name % !SyncfusionTimelineViewHeaderStyle5  /Syncfusion.UI.Xaml.Scheduler.TimelineViewHeader " SyncfusionMonthViewHeaderStyle2  ,Syncfusion.UI.Xaml.Scheduler.MonthViewHeader# �� Padding�� D 9http://schemas.microsoft.com/winfx/2006/xaml/presentation    <
themelocal-clr-namespace:Syncfusion.Themes.basetheme.WPF  :x,http://schemas.microsoft.com/winfx/2006/xaml     1system&clr-namespace:System;assembly=mscorlib  \skinmanagerLclr-namespace:Syncfusion.SfSkinManager;assembly=Syncfusion.SfSkinManager.WPF  \	schedulerNclr-namespace:Syncfusion.UI.Xaml.Scheduler;assembly=Syncfusion.SfScheduler.WPF       $ Common��$ 	basetheme��   $ Brushes��$ 	basetheme��%_  &	        &	      7  &	      7  7  '      7  &	      7  7  7  7  7 	 7 
 7  '  �    7  &	 �    7  7 	 7  7  7  7  7  7  7  7  7  &	 v	    7  7  7  7  7  7  7  '  s    7  &	 �    7  7  7  7 
 '  �    7  &	 �    7  7  '  2    7  �� $ Green��  8   �� "  .���  � ��8   ��  � ��8  ��  � ��$ 0,0,0,1q���  � �� �� "  .R��� #��z��#��z����u�0q�._�� 	 �� �� $��* �.+��� #��z����u�0,1,1,0q��� $!
 PART_ViewHeaderScrollViewer��$��1��$��Hidden=� .  $��Hidden=� ���� '��       " ��.���  � �� �� " ��.R��� 
������ .��� $ PART_ViewHeaderGrid�� �� $
��auto ��� $
��auto �.+�  $ PART_ResourcePanel��  $ PART_ViewHeaderRowPanel���� 8   "  �� "   P�  �@ IsToday . . ���  � ��8   .���  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��$ Stretch=���  � ��$
 Left=���  � ��$ Stretch=���  � ��$ Center=���  � ��
 ���� ��  � ��$ 8,0,0,0q���  � {� . ��  � �� �� "  .R��� #��z��#��z��._��� #��z #��z��#��z��#��z��#��z���� 8   "  �� .R�� $ 	PART_Grid������ *
Background+.+��� $  PART_TodayHightlightBorder��$��0,0,1,0q�8��  $��0,2,0,0q�$y�	Collapsed=�� $
��Left=�
������ ���� *	Padding+���� *VerticalContentAlignment+ �� $��* ��� $��27 �.+��� $ PART_DayText��$��Center=�$��8,0,0,0q����� *
FontFamily+���� *
FontSize+���� *	FontStyle+���� *
Foreground+���� *	DayText+�� $ PART_DateText��$��1��$	��Top=�$
 Left=�$��8,0,0,0q�8�� 8�� 8�� ���� *	FontStyle+���� *
DateText+����  ^�  ��  �� $ PART_DateText��$ IsMouseOver� True��  �� $ 	IsDayView�	 False��  �� $ AllowViewNavigation� True0���  � ��$ 	Underline{���  �@�Cursor$
 Hand�� �� $
 True��  �� *	IsToday+.a��� $! PART_DateText��8   � ���� $ ! PART_TodayHightlightBorder�� � y�$ Visible=��� $ False��  �� *	IsEnabled+.a��� $! PART_DayText��8   � ���� $! PART_DateText��8   � ��P�  �@J�IsMouseOver . . ��� $! 	PART_Grid��8   � ���� $ ! PART_TodayHightlightBorder��8   � ���� $ ! PART_TodayHightlightBorder�� � y�$ Visible=�^�  ��  �� $ 	PART_Grid��$ IsMouseOver� true��  �� *	IsToday+ True0��� $! 	PART_Grid��8 	  � ���� $ ! PART_TodayHightlightBorder��8 
  � ���� "  8    P�  �@ IsToday . . ���  � ��8  ��  � ��$ 0,2,0,0q���  � ��8  .���  � ��$ 0q���  � ��8  ��  � ��$ Stretch=���  � ��$ Stretch=���  � ��8  ��  � �� �� "  .R��� #��z��$��Stretch=�$��Stretch=�8�� ._�� .+��� $! PART_DayViewHeaderTopBorder��8�� $��Stretch=���u�0,1,1,0q��� $(" "PART_DayViewHeaderContentPresenter��#��z��$��0q�#��z��# z #��z #��z��$��Stretch=�$��Stretch=��� 8   "  �� "  8    P�  �@ IsToday . . ���  � ��$ Normal"�.���  � �� �� .R�� $��Stretch=�$��Stretch=����� *
Background+.+��� $ PART_DateText��$
��Left=�$��Center=�8�� ���� *
FontFamily+���� *	FontStyle+���� *
Foreground+���� *
DateText+����  ^�  ��  �� $ PART_DateText��$ IsMouseOver� True��  �� $ 	IsDayView�	 False��  �� $ AllowViewNavigation� True0���  � ��$ 	Underline{���  �@�Cursor$
 Hand�� �� $ False��  �� *	IsEnabled+.a��� $! PART_DateText��8   � ���� $ False��  �� *IsHitTestVisible+.a��� $! PART_DateText�� � ��$ Strikethrough{��� $ TimelineMonth��  �� *
ViewType+.a��� $! PART_DateText��8   � ���� $! PART_DateText�� � ��$ Center=��� 8   "  �� "  8   .���  � �� �� .R�� 
������ $��Stretch=�$��Stretch=�.+��� $ PART_ViewHeaderTopBorder��8�� $��0,1,0,0q�$��Stretch=��� $
��Left=�$# 8,0,0,0q�$��Center=����� *
FontFamily+���� *
FontSize+���� *	FontStyle+���� *
Foreground+���� *	DayText+�� 8   "  