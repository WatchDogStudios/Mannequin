   M S B A M L   `   `   `  ���� k  gSyncfusion.Themes.basetheme.WPF, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=e1da1d9bc9725b9dQ-clr-namespace:Syncfusion.Themes.basetheme.WPFSyncfusion.Themes.basetheme.WPF  O Kmscorlib, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e0891&clr-namespace:System;assembly=mscorlibSystem h dSyncfusion.SfSkinManager.WPF, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=3d67ed1f87d44c89iLclr-namespace:Syncfusion.SfSkinManager;assembly=Syncfusion.SfSkinManager.WPFSyncfusion.SfSkinManager f bSyncfusion.SfScheduler.WPF, Version=27.1462.48.0, Culture=neutral, PublicKeyToken=3d67ed1f87d44c89oNclr-namespace:Syncfusion.UI.Xaml.Scheduler;assembly=Syncfusion.SfScheduler.WPFSyncfusion.UI.Xaml.Scheduler W SPresentationCore, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NWindowsBase, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35\ XPresentationFramework, Version=4.0.0.0, Culture=neutral, PublicKeyToken=31bf3856ad364e35R NSystem.Xaml, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b77a5c561934e089  �� MergedDictionaries9   3Syncfusion.SfSkinManager.ResourceDictionaryProvider    DictionaryName    	ThemeName 0  ,SfScheduler.TodayHighlight.Static.Background �� Color;  5Syncfusion.UI.Xaml.Scheduler.TodayBorderSizeConverter  TodayBorderSizeConverter  basetheme.TitleTextStyle   FontSize # SyncfusionMonthViewControlStyle �� 
TargetType3  -Syncfusion.UI.Xaml.Scheduler.MonthViewControl �� Property  	BorderAlt �� Value �� 
TargetType	 � ColumnDefinitions
 �� Path �� Mode �� RelativeSource2  ,Syncfusion.UI.Xaml.Scheduler.WeekNumberPanel  Name1  +Syncfusion.UI.Xaml.Scheduler.MonthViewPanel  Name �� BasedOn % !SyncfusionMonthCellFluentTemplate �Name ��Name �� ElementName ��Name �� TextAlignment �� Triggers ^� 
Conditions �� Binding �� Value F�Name J� Opacity �� Stretch �� 
StartPoint �� EndPoint �� Triggers �� Value  �� Binding! �� 
TargetName  PrimaryColorForeground  ContentBackgroundAlt1  SyncfusionMonthCellStyle,  &Syncfusion.UI.Xaml.Scheduler.MonthCell" P� Property# P� Value 	 DisabledForeground 
 ContentBackground$ ]� 
Conditions% �� Property  PlaceholderForeground  basetheme.ThemeFontFamily  basetheme.FontWeightNormal4  .Syncfusion.UI.Xaml.Scheduler.SchedulerCellBase  SelectedForeground  ContentForeground'  !System.Windows.VisualStateManager&   VisualStateGroups%  System.Windows.VisualStateGroup'   States 	  System.Windows.VisualState( 	  
Storyboard) g� 	BeginTime* �� 
TargetName+ �� TargetProperty, R� KeyTime
  System.Windows.Visibility- 
  Visible. R� Value  ContentBackgroundHovered  HoveredForeground  ContentBackgroundSelected/ �� CornerRadius0 ��Name1 � DataContext2   ContentTemplate3 �� Triggers0  *Syncfusion.SfSkinManager.SkinManagerHelper   FlatKeyboardFocusVisualStyle * &SyncfusionSchedulerWeekNumberCellStyle1  +Syncfusion.UI.Xaml.Scheduler.WeekNumberCell  ContentForegroundAlt1  basetheme.CaptionText�� D 9http://schemas.microsoft.com/winfx/2006/xaml/presentation    <
themelocal-clr-namespace:Syncfusion.Themes.basetheme.WPF  :x,http://schemas.microsoft.com/winfx/2006/xaml     1system&clr-namespace:System;assembly=mscorlib  \skinmanagerLclr-namespace:Syncfusion.SfSkinManager;assembly=Syncfusion.SfSkinManager.WPF  \	schedulerNclr-namespace:Syncfusion.UI.Xaml.Scheduler;assembly=Syncfusion.SfScheduler.WPF       $ Common��$ 	basetheme��   $ Brushes��$ 	basetheme��%�  &	        &	      7  &	      7  '  m    7  &	 |    7  7  7  7  7  7  7  7  &	 }    7 	 7 
 7  7 
 7  7 
 7  7  7  7  7  7 
 7  7  7  7  7  7 
 7  7 
 7  7 
 7 	 7 
 7 	 7 
 7 	 7 
 7  7 
 7  7  7  7  7  7  7  7  7  7  7  7  '  �    7  &	 �    7  7  7  7  7  7  '  =    7  �� $ #0279FF��  8   �� "  .���  � ��8   ��  � �� u�0,1,0,0q���  � �� �� "  .R��� #��z��#��z��._�� #��z��	 �� $��0 ����� $
 WeekNumberColumnWidth�$ OneWay=� �� *TemplatedParent+�� $��* �.+�  $ PART_WeekNumberPanel��$��0��$y�	Collapsed=�  $ PART_MonthViewPanel��$��1���� 8   "  �� .R�� $ 	PART_Grid��
������ .+��� $ PART_Border��
������ ._�� $��
7 ,3 ,0, 0q�	 �� $
��0.1* ����� $ PART_TextBlock��$
 ActualWidth��� $
��0.9* �.+��� $ PART_TextBlock��$ Center=�$��Stretch=�$	��Top=����� $
 DateText�����  ^�  ��  �� $ PART_TextBlock��$
 IsMouseOver� true��  �� $
 AllowViewNavigation� True0���  � ��$ 	Underline{���  �@�Cursor$
 Hand��F� $ PART_BlackOutDate��$
��Left=�$	��Top=�$	 0.7Q�$��1��$y�	Collapsed=�$
 Fill=����� $ PART_TextBlock��$
 ActualWidth����� $ PART_TextBlock��$
 ActualHeight����� $ PART_TextBlock��$
 
Foreground����� $	 0,0&�$	 1,1&� �� $ Today��  �� $
 DayType�.a��� $! PART_TextBlock��8    � ���� $! PART_TextBlock�� � ��$ Medium"��� $! PART_Border��8   � ���� $! PART_Border�� � ��$ 0,-1,0,0q��� $! PART_Border�� � ��$ 0,2,0,0q��� $ NormalDay, BlackoutDay��  �� $
 DayType�.a��� $! PART_BlackOutDate�� � y�$ Visible=��� $! 	PART_Grid��8   � ���� $ Today, BlackoutDay��  �� $
 DayType�.a��� $! PART_BlackOutDate�� � y�$ Visible=��� $! PART_TextBlock�� � ��$ Medium"��� $! 	PART_Grid��8   � ���� $! PART_TextBlock��8   � ���� $! PART_Border��8   � ���� $! PART_Border�� � ��$ 0,-1,0,0q��� $! PART_Border�� � ��$ 0,2,0,0q��� $ LeadingDay, BlackoutDay��  �� $
 DayType�.a��� $! PART_BlackOutDate�� � y�$ Visible=��� $! 	PART_Grid��8   � ���� $ TrailingDay, BlackoutDay��  �� $
 DayType�.a��� $! PART_BlackOutDate�� � y�$ Visible=��� $! 	PART_Grid��8   � ���� "   P� " � {�# .  . ���  � ��8   ��  � ��8  ]� $ �� % �@ DayType$ Today=��� % � {� . /���  � ��8  ��  � ��8  ]� $ �� % �@ DayType$ 
LeadingDay=��� % � {� . /���  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��8 	 ]� $ �� % �@ DayType$ TrailingDay=��� % � {� . /���  � ��8 
 ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ]� $ �� % �@ DayType$ Today,BlackoutDay=��� % � {� . /���  � ��8  ��  � ��8  ]� $ �� % �@ DayType$ LeadingDay,BlackoutDay=��� % � {� . /���  � ��8  ��  � ��8  ]� $ �� % �@ DayType$ TrailingDay,BlackoutDay=��� % � {� . /���  � ��8  ��  � ��8  ]� $ �� % �@ DayType$ Today,BlackoutDay=��� % � {� .  /���  � ��8  ��  � ��8  ]� $ �� % �@ DayType$ LeadingDay,BlackoutDay=��� % � {� .  /���  � ��8  ��  � ��8  ]� $ �� % �@ DayType$ TrailingDay,BlackoutDay=��� % � {� .  /���  � ��8  ��  � ��8  ]� $ �� % �@ DayType$ 	NormalDay=��� % �@ 
IsSelected . /���  � ��8  .���  � ��8  ��  � ��8  ��  � ��8  ��  � ��8   ��  � ��8 ! ��  � ��8 " ��  � ��$ 0,0,1,1q���  � ��$ 0q���  �@�UseLayoutRounding .  ��  � ��$	 Top=���  � ��$ Stretch=���  � ��$ Stretch=���  � ��$ Center=���  �@�FocusVisualStyle Z� ��  �@ ContentTemplate8 # ��  �@J�	AllowDrop . ��  � �� �� "  .R��� #��z��#��z��&   SelectionStates��.' 	  
Unselected��	  Selected��.( �� .��T� $) 00:00:00X�$* PART_SelectionBorder��$+ (UIElement.Visibility)�..�i� $, 00:00:00��#. Z- 	  	MouseOver��.( �� .��T� $) 0X�$* PART_MonthCellGrid��$+ 
Background�..�i� $, 0��8. $ 	  NormalDayMouseOver��.( �� .��T� $) 0X�$$* PART_MonthCellContentPresenter��$+ (TextBlock.Foreground)�..�i� $, 0��8. % T� $) 0X�$* PART_MonthCellGrid��$+ 
Background�..�i� $, 0��8. & ._�� $ PART_MonthCellGrid��#��z��.+��� $ PART_SelectionBorder��8��' 8��( $��0q�$/ 0��$y�	Collapsed=��� $$0 PART_MonthCellContentPresenter��#��z1 #��z��#��z��#��z��#��z2 #1 z1 #��z��#��z��#��z��#��z��3 P� " �@ FocusVisualKind$# HighVisibility=�. ��� 8 )  �@�FocusVisualStyle�� 8   "  �� "  .���  � ��8   ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��8  ��  � ��$ 0,0,1,1q���  � ��$ 0,4,0,0q���  � ��$ Center=���  �@�UseLayoutRounding .  ��  � �� �� "  .R��� #��z��#��z��._�� #��z��.+��� #��z��#��z��#��z��#��z��#��z��#��z��#��z��$��Center=��� 8   "  