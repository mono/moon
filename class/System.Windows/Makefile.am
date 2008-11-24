assembly = System.Windows

NET_2_1_ASSEMBLY_DEPS=-r:System.Runtime.Serialization.dll
NET_3_0_ASSEMBLY_DEPS=

if INCLUDE_BROWSER_MANAGED_CODE
assemblysl = ../lib/2.1/$(assembly).dll
endif

if INCLUDE_DESKTOP_MANAGED_CODE
assemblydesktop = ../lib/3.0/$(assembly).dll
endif

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -d:MOONLIGHT -debug+ -noconfig -r:System -r:System.Core -r:System.Xml -r:Mono.Moonlight -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS) -lib:../lib/3.0
SMCS = MONO_PATH="../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe -r:System.Net $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib

MSCONTROLS=$(srcdir)/../Microsoft.SilverlightControls/Controls
WPFTOOLKIT=$(srcdir)/../WPF.Toolkit

system_windows_sources =							\
	$(srcdir)/Assembly/AssemblyInfo.cs					\
	$(srcdir)/Assembly/MonoTODOAttribute.cs					\
	$(srcdir)/../Consts.cs							\
	$(srcdir)/Microsoft.Internal/IManagedFrameworkInternalHelper.cs		\
	$(srcdir)/Mono/EventHandlerList.cs					\
	$(srcdir)/Mono/Events.cs						\
	$(srcdir)/Mono/Hosting.cs						\
	$(srcdir)/Mono/ManagedXamlLoader.cs					\
	$(srcdir)/Mono/Report.cs						\
	$(srcdir)/Mono/StreamWrapper.cs						\
	$(srcdir)/System.ComponentModel/DesignerProperties.cs			\
	$(srcdir)/System.Collections.ObjectModel/ObservableCollection_T.cs	\
	$(srcdir)/System.Collections.Specialized/INotifyCollectionChanged.cs	\
	$(srcdir)/System.Collections.Specialized/NotifyCollectionChangedAction.cs \
	$(srcdir)/System.Collections.Specialized/NotifyCollectionChangedEventArgs.cs \
	$(srcdir)/System.Collections.Specialized/NotifyCollectionChangedEventHandler.cs \
	$(srcdir)/System.IO.IsolatedStorage/IsolatedStorageSettings.cs		\
	$(srcdir)/System.Windows.Automation/DockPosition.cs			\
	$(srcdir)/System.Windows.Automation/ExpandCollapseState.cs		\
	$(srcdir)/System.Windows.Automation/RowOrColumnMajor.cs			\
	$(srcdir)/System.Windows.Automation/ScrollAmount.cs			\
	$(srcdir)/System.Windows.Automation/ToggleState.cs			\
	$(srcdir)/System.Windows.Automation/WindowInteractionState.cs		\
	$(srcdir)/System.Windows.Automation/WindowVisualState.cs		\
	$(srcdir)/System.Windows.Automation.Peers/AutomationPeer.cs		\
	$(srcdir)/System.Windows.Automation.Peers/AutomationControlType.cs	\
	$(srcdir)/System.Windows.Automation.Peers/FrameworkElementAutomationPeer.cs	\
	$(srcdir)/System.Windows.Automation.Peers/ItemAutomationPeer.cs		\
	$(srcdir)/System.Windows.Automation.Peers/ItemsControlAutomationPeer.cs	\
	$(srcdir)/System.Windows.Automation.Peers/PatternInterface.cs		\
	$(srcdir)/System.Windows.Automation.Provider/IDockProvider.cs		\
	$(srcdir)/System.Windows.Automation.Provider/IExpandCollapseProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/IGridItemProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/IGridProvider.cs		\
	$(srcdir)/System.Windows.Automation.Provider/IInvokeProvider.cs		\
	$(srcdir)/System.Windows.Automation.Provider/IMultipleViewProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/IRangeValueProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/IRawElementProviderSimple.cs	\
	$(srcdir)/System.Windows.Automation.Provider/IScrollItemProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/IScrollProvider.cs		\
	$(srcdir)/System.Windows.Automation.Provider/ISelectionItemProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/ISelectionProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/ITableItemProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/ITableProvider.cs		\
	$(srcdir)/System.Windows.Automation.Provider/IToggleProvider.cs		\
	$(srcdir)/System.Windows.Automation.Provider/ITransformProvider.cs	\
	$(srcdir)/System.Windows.Automation.Provider/IValueProvider.cs		\
	$(srcdir)/System.Windows.Automation.Provider/IWindowProvider.cs		\
	$(srcdir)/System.Windows.Controls/Border.cs				\
	$(srcdir)/System.Windows.Controls/InkPresenter.cs			\
	$(srcdir)/System.Windows.Controls/MediaElement.cs			\
	$(srcdir)/System.Windows.Controls/Orientation.cs			\
	$(srcdir)/System.Windows.Controls/ValidationErrorEventAction.cs		\
	$(srcdir)/System.Windows.Controls/Canvas.cs				\
	$(srcdir)/System.Windows.Controls/ColumnDefinition.cs			\
	$(srcdir)/System.Windows.Controls/ColumnDefinitionCollection.cs		\
	$(srcdir)/System.Windows.Controls/Control.cs				\
	$(srcdir)/System.Windows.Controls/FileDialogFileInfo.cs			\
	$(srcdir)/System.Windows.Controls/Grid.cs				\
	$(srcdir)/System.Windows.Controls/Image.cs				\
	$(srcdir)/System.Windows.Controls/ItemCollection.cs			\
	$(srcdir)/System.Windows.Controls/ItemsControl.cs			\
	$(srcdir)/System.Windows.Controls/ItemsPanelTemplate.cs			\
	$(srcdir)/System.Windows.Controls/MultiScaleImage.cs			\
	$(srcdir)/System.Windows.Controls/MultiScaleSubImage.cs			\
	$(srcdir)/System.Windows.Controls/OpenFileDialog.cs			\
	$(srcdir)/System.Windows.Controls/Panel.cs				\
	$(srcdir)/System.Windows.Controls/StackPanel.cs				\
	$(srcdir)/System.Windows.Controls/TextBlock.cs				\
	$(srcdir)/System.Windows.Controls/TextBox.cs				\
	$(srcdir)/System.Windows.Controls/TextChangedEventArgs.cs		\
	$(srcdir)/System.Windows.Controls/TextChangedEventHandler.cs		\
	$(srcdir)/System.Windows.Controls/UserControl.cs			\
	$(srcdir)/System.Windows.Controls/UIElementCollection.cs		\
	$(srcdir)/System.Windows.Controls/RowDefinition.cs			\
	$(srcdir)/System.Windows.Controls/RowDefinitionCollection.cs		\
	$(srcdir)/System.Windows.Controls/ControlTemplate.cs			\
	$(srcdir)/System.Windows.Controls.Primitives/LayoutInformation.cs	\
	$(srcdir)/System.Windows.Controls.Primitives/Popup.cs			\
	$(srcdir)/System.Windows.Controls.Primitives/ScrollEventHandler.cs	\
	$(srcdir)/System.Windows.Ink/DrawingAttributes.cs			\
	$(srcdir)/System.Windows.Ink/Stroke.cs					\
	$(srcdir)/System.Windows.Ink/StrokeCollection.cs			\
	$(srcdir)/System.Windows.Input/Cursor.cs				\
	$(srcdir)/System.Windows.Input/Cursors.cs				\
	$(srcdir)/System.Windows.Input/FocusManager.cs				\
	$(srcdir)/System.Windows.Input/ICommand.cs				\
	$(srcdir)/System.Windows.Input/InputMethod.cs				\
	$(srcdir)/System.Windows.Input/KeyboardNavigationMode.cs		\
	$(srcdir)/System.Windows.Input/ModifierKeys.cs				\
	$(srcdir)/System.Windows.Input/StylusDevice.cs				\
	$(srcdir)/System.Windows.Input/StylusPoint.cs				\
	$(srcdir)/System.Windows.Input/StylusPointCollection.cs			\
	$(srcdir)/System.Windows.Input/Key.cs					\
	$(srcdir)/System.Windows.Input/Keyboard.cs				\
	$(srcdir)/System.Windows.Input/KeyEventArgs.cs				\
	$(srcdir)/System.Windows.Input/KeyEventHandler.cs			\
	$(srcdir)/System.Windows.Input/MouseButtonEventArgs.cs			\
	$(srcdir)/System.Windows.Input/MouseButtonEventHandler.cs		\
	$(srcdir)/System.Windows.Input/MouseEventArgs.cs			\
	$(srcdir)/System.Windows.Input/MouseEventHandler.cs			\
	$(srcdir)/System.Windows.Interop/BrowserHost.cs				\
	$(srcdir)/System.Windows.Interop/Content.cs				\
	$(srcdir)/System.Windows.Interop/HostingRenderTargetBitmap.cs		\
	$(srcdir)/System.Windows.Interop/Settings.cs				\
	$(srcdir)/System.Windows.Interop/SilverlightHost.cs			\
	$(srcdir)/System.Windows.Media.Animation/ClockState.cs			\
	$(srcdir)/System.Windows.Media.Animation/ColorAnimationUsingKeyFrames.cs\
	$(srcdir)/System.Windows.Media.Animation/ColorKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/ColorKeyFrameCollection.cs	\
	$(srcdir)/System.Windows.Media.Animation/DiscreteColorKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/DiscreteDoubleKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/DiscreteObjectKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/DiscretePointKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/DoubleKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/DoubleKeyFrameCollection.cs	\
	$(srcdir)/System.Windows.Media.Animation/KeyTime.cs			\
	$(srcdir)/System.Windows.Media.Animation/KeyTimeType.cs			\
	$(srcdir)/System.Windows.Media.Animation/LinearColorKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/LinearDoubleKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/LinearPointKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/ObjectAnimationUsingKeyFrames.cs		\
	$(srcdir)/System.Windows.Media.Animation/ObjectKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/ObjectKeyFrameCollection.cs	\
	$(srcdir)/System.Windows.Media.Animation/PointAnimationUsingKeyFrames.cs\
	$(srcdir)/System.Windows.Media.Animation/PointKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/PointKeyFrameCollection.cs	\
	$(srcdir)/System.Windows.Media.Animation/SplineColorKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/SplinePointKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/TimelineCollection.cs		\
	$(srcdir)/System.Windows/Application.cs					\
	$(srcdir)/System.Windows/ApplicationInternal.cs				\
	$(srcdir)/System.Windows/ApplicationUnhandledExceptionEventArgs.cs	\
	$(srcdir)/System.Windows/AssemblyPart.cs				\
	$(srcdir)/System.Windows/AssemblyPartCollection.cs			\
	$(srcdir)/System.Windows/CornerRadius.cs				\
	$(srcdir)/System.Windows/CustomDependencyProperty.cs			\
	$(srcdir)/System.Windows/CrossDomainAccess.cs				\
	$(srcdir)/System.Windows/DataTemplate.cs				\
	$(srcdir)/System.Windows/Deployment.cs					\
	$(srcdir)/System.Windows/DependencyObject.cs				\
	$(srcdir)/System.Windows/DependencyObject.g.cs				\
	$(srcdir)/System.Windows/DependencyProperty.cs				\
	$(srcdir)/System.Windows/DependencyProperty.g.cs			\
	$(srcdir)/System.Windows/DependencyPropertyChangedEventArgs.cs		\
	$(srcdir)/System.Windows/DependencyPropertyChangedEventHandler.cs	\
	$(srcdir)/System.Windows/DurationConverter.cs				\
	$(srcdir)/System.Windows/Duration.cs					\
	$(srcdir)/System.Windows/ExceptionRoutedEventArgs.cs			\
	$(srcdir)/System.Windows/ExceptionRoutedEventHandler.cs			\
	$(srcdir)/System.Windows/Expression.cs					\
	$(srcdir)/System.Windows/EventTrigger.cs				\
	$(srcdir)/System.Windows/FontStretch.cs					\
	$(srcdir)/System.Windows/FontStretchKind.cs				\
	$(srcdir)/System.Windows/FontStretches.cs				\
	$(srcdir)/System.Windows/FontStyle.cs					\
	$(srcdir)/System.Windows/FontStyleKind.cs				\
	$(srcdir)/System.Windows/FontStyles.cs					\
	$(srcdir)/System.Windows/FontWeight.cs					\
	$(srcdir)/System.Windows/FontWeights.cs					\
	$(srcdir)/System.Windows/FontWeightKind.cs				\
	$(srcdir)/System.Windows/FrameworkElement.cs				\
	$(srcdir)/System.Windows/FrameworkTemplate.cs				\
	$(srcdir)/System.Windows/GridLength.cs					\
	$(srcdir)/System.Windows/GridUnitType.cs				\
	$(srcdir)/System.Windows/HorizontalAlignment.cs				\
	$(srcdir)/System.Windows/LineStackingStrategy.cs			\
	$(srcdir)/System.Windows/MessageBox.cs					\
	$(srcdir)/System.Windows/MessageBoxButton.cs				\
	$(srcdir)/System.Windows/MessageBoxResult.cs				\
	$(srcdir)/System.Windows/Point.cs					\
	$(srcdir)/System.Windows/PresentationFrameworkCollection.cs		\
	$(srcdir)/System.Windows/PropertyChangedCallback.cs			\
	$(srcdir)/System.Windows/PropertyMetadata.cs				\
	$(srcdir)/System.Windows/PropertyPathConverter.cs			\
	$(srcdir)/System.Windows/PropertyPath.cs				\
	$(srcdir)/System.Windows/ResourceDictionary.cs				\
	$(srcdir)/System.Windows/Rect.cs					\
	$(srcdir)/System.Windows/RoutedEvent.cs					\
	$(srcdir)/System.Windows/RoutedEventArgs.cs				\
	$(srcdir)/System.Windows/RoutedEventHandler.cs				\
	$(srcdir)/System.Windows/SetterBaseCollection.cs			\
	$(srcdir)/System.Windows/SetterBase.cs					\
	$(srcdir)/System.Windows/Setter.cs					\
	$(srcdir)/System.Windows/Size.cs					\
	$(srcdir)/System.Windows/SizeChangedEventArgs.cs			\
	$(srcdir)/System.Windows/SizeChangedEventHandler.cs			\
	$(srcdir)/System.Windows/StartupEventArgs.cs				\
	$(srcdir)/System.Windows/StartupEventHandler.cs				\
	$(srcdir)/System.Windows/Style.cs					\
	$(srcdir)/System.Windows/StyleTypedPropertyAttribute.cs			\
	$(srcdir)/System.Windows/SystemParameters.cs				\
	$(srcdir)/System.Windows/TemplateBindingExpression.cs			\
	$(srcdir)/System.Windows/TemplatePart.cs				\
	$(srcdir)/System.Windows/TextAlignment.cs				\
	$(srcdir)/System.Windows/TextDecorationCollection.cs			\
	$(srcdir)/System.Windows/TextDecorationKind.cs				\
	$(srcdir)/System.Windows/TextDecorations.cs				\
	$(srcdir)/System.Windows/TextWrapping.cs				\
	$(srcdir)/System.Windows/Thickness.cs					\
	$(srcdir)/System.Windows/TriggerBase.cs					\
	$(srcdir)/System.Windows/TriggerCollection.cs				\
	$(srcdir)/System.Windows/TriggerAction.cs				\
	$(srcdir)/System.Windows/TriggerActionCollection.cs			\
	$(srcdir)/System.Windows/UIElement.cs					\
	$(srcdir)/System.Windows/VerticalAlignment.cs				\
	$(srcdir)/System.Windows/Visibility.cs					\
	$(srcdir)/System.Windows/VisualState.cs					\
	$(srcdir)/System.Windows/XapHackProxyImpl.cs				\
	$(srcdir)/System.Windows.Data/Binding.cs				\
	$(srcdir)/System.Windows.Data/BindingExpression.cs			\
	$(srcdir)/System.Windows.Data/BindingExpressionBase.cs			\
	$(srcdir)/System.Windows.Data/BindingMode.cs				\
	$(srcdir)/System.Windows.Data/IValueConverter.cs			\
	$(srcdir)/System.Windows.Documents/FontSource.cs			\
	$(srcdir)/System.Windows.Documents/Glyphs.cs				\
	$(srcdir)/System.Windows.Documents/Inline.cs				\
	$(srcdir)/System.Windows.Documents/InlineCollection.cs			\
	$(srcdir)/System.Windows.Documents/LineBreak.cs				\
	$(srcdir)/System.Windows.Documents/Run.cs				\
	$(srcdir)/System.Windows.Markup/XamlParseException.cs			\
	$(srcdir)/System.Windows.Markup/XamlReader.cs				\
	$(srcdir)/System.Windows.Markup/XmlLanguage.cs				\
	$(srcdir)/System.Windows.Markup/XmlnsDefinitionAttribute.cs		\
	$(srcdir)/System.Windows.Markup/XmlnsPrefixAttribute.cs		\
	$(srcdir)/System.Windows.Markup/ContentPropertyAttribute.cs		\
	$(srcdir)/System.Windows.Media/AlignmentX.cs				\
	$(srcdir)/System.Windows.Media/AlignmentY.cs				\
	$(srcdir)/System.Windows.Media/ArcSegment.cs				\
	$(srcdir)/System.Windows.Media/BezierSegment.cs				\
	$(srcdir)/System.Windows.Media/Brush.cs					\
	$(srcdir)/System.Windows.Media/BrushMappingMode.cs			\
	$(srcdir)/System.Windows.Media/Color.cs					\
	$(srcdir)/System.Windows.Media/Colors.cs				\
	$(srcdir)/System.Windows.Media/ColorInterpolationMode.cs		\
	$(srcdir)/System.Windows.Media/CompositionTarget.cs			\
	$(srcdir)/System.Windows.Media/DeepZoomImageTileSource.cs		\
	$(srcdir)/System.Windows.Media/DoubleCollection.cs			\
	$(srcdir)/System.Windows.Media/EllipseGeometry.cs			\
	$(srcdir)/System.Windows.Media/FillRule.cs				\
	$(srcdir)/System.Windows.Media/FontFamily.cs				\
	$(srcdir)/System.Windows.Media/GeneralTransform.cs			\
	$(srcdir)/System.Windows.Media/Geometry.cs				\
	$(srcdir)/System.Windows.Media/GeometryCollection.cs			\
	$(srcdir)/System.Windows.Media/GeometryGroup.cs				\
	$(srcdir)/System.Windows.Media/GradientBrush.cs				\
	$(srcdir)/System.Windows.Media/GradientSpreadMethod.cs			\
	$(srcdir)/System.Windows.Media/GradientStop.cs				\
	$(srcdir)/System.Windows.Media/GradientStopCollection.cs		\
	$(srcdir)/System.Windows.Media/ImageBrush.cs				\
	$(srcdir)/System.Windows.Media/ImageSource.cs				\
	$(srcdir)/System.Windows.Media/LicenseAcquirer.cs			\
	$(srcdir)/System.Windows.Media/LinearGradientBrush.cs			\
	$(srcdir)/System.Windows.Media/LineGeometry.cs				\
	$(srcdir)/System.Windows.Media/LineSegment.cs				\
	$(srcdir)/System.Windows.Media/MatrixTransform.cs			\
	$(srcdir)/System.Windows.Media/Matrix.cs				\
	$(srcdir)/System.Windows.Media/MediaElementState.cs			\
	$(srcdir)/System.Windows.Media/MediaSampleAttributeKeys.cs		\
	$(srcdir)/System.Windows.Media/MediaSourceAttributesKeys.cs		\
	$(srcdir)/System.Windows.Media/MediaStreamAttributesKeys.cs		\
	$(srcdir)/System.Windows.Media/MediaStreamDescription.cs		\
	$(srcdir)/System.Windows.Media/MediaStreamSample.cs			\
	$(srcdir)/System.Windows.Media/MediaStreamSource.cs			\
	$(srcdir)/System.Windows.Media/MediaStreamSourceDiagnosticKind.cs	\
	$(srcdir)/System.Windows.Media/MediaStreamType.cs			\
	$(srcdir)/System.Windows.Media/MultiScaleTileSource.cs			\
	$(srcdir)/System.Windows.Media/PathFigure.cs				\
	$(srcdir)/System.Windows.Media/PathFigureCollection.cs			\
	$(srcdir)/System.Windows.Media/PathGeometry.cs				\
	$(srcdir)/System.Windows.Media/PathSegment.cs				\
	$(srcdir)/System.Windows.Media/PathSegmentCollection.cs			\
	$(srcdir)/System.Windows.Media/PenLineCap.cs				\
	$(srcdir)/System.Windows.Media/PenLineJoin.cs				\
	$(srcdir)/System.Windows.Media/PointCollection.cs			\
	$(srcdir)/System.Windows.Media/PolyBezierSegment.cs			\
	$(srcdir)/System.Windows.Media/PolyLineSegment.cs			\
	$(srcdir)/System.Windows.Media/PolyQuadraticBezierSegment.cs		\
	$(srcdir)/System.Windows.Media/QuadraticBezierSegment.cs		\
	$(srcdir)/System.Windows.Media/RadialGradientBrush.cs			\
	$(srcdir)/System.Windows.Media/RectangleGeometry.cs			\
	$(srcdir)/System.Windows.Media/RenderingEventArgs.cs			\
	$(srcdir)/System.Windows.Media/RotateTransform.cs			\
	$(srcdir)/System.Windows.Media/ScaleTransform.cs			\
	$(srcdir)/System.Windows.Media/SkewTransform.cs				\
	$(srcdir)/System.Windows.Media/SolidColorBrush.cs			\
	$(srcdir)/System.Windows.Media/Stretch.cs				\
	$(srcdir)/System.Windows.Media/StyleSimulations.cs			\
	$(srcdir)/System.Windows.Media/SweepDirection.cs			\
	$(srcdir)/System.Windows.Media/TileBrush.cs				\
	$(srcdir)/System.Windows.Media/TimelineMarker.cs			\
	$(srcdir)/System.Windows.Media/TimelineMarkerCollection.cs		\
	$(srcdir)/System.Windows.Media/TimelineMarkerRoutedEventArgs.cs		\
	$(srcdir)/System.Windows.Media/TimelineMarkerRoutedEventHandler.cs	\
	$(srcdir)/System.Windows.Media/TransformCollection.cs			\
	$(srcdir)/System.Windows.Media/Transform.cs				\
	$(srcdir)/System.Windows.Media/TransformGroup.cs			\
	$(srcdir)/System.Windows.Media/TranslateTransform.cs			\
	$(srcdir)/System.Windows.Media/VideoBrush.cs				\
	$(srcdir)/System.Windows.Media/VisualTreeHelper.cs			\
	$(srcdir)/System.Windows.Media.Animation/BeginStoryboard.cs		\
	$(srcdir)/System.Windows.Media.Animation/ColorAnimation.cs		\
	$(srcdir)/System.Windows.Media.Animation/DoubleAnimation.cs		\
	$(srcdir)/System.Windows.Media.Animation/DoubleAnimationUsingKeyFrames.cs\
	$(srcdir)/System.Windows.Media.Animation/FillBehavior.cs		\
	$(srcdir)/System.Windows.Media.Animation/KeySpline.cs			\
	$(srcdir)/System.Windows.Media.Animation/PointAnimation.cs		\
	$(srcdir)/System.Windows.Media.Animation/RepeatBehavior.cs		\
	$(srcdir)/System.Windows.Media.Animation/SplineDoubleKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/Storyboard.cs			\
	$(srcdir)/System.Windows.Media.Animation/Timeline.cs			\
	$(srcdir)/System.Windows.Media.Imaging/BitmapImage.cs			\
	$(srcdir)/System.Windows.Media.Imaging/DownloadProgressEventArgs.cs	\
	$(srcdir)/System.Windows.Media.Imaging/RenderTargetBitmap.cs	\
	$(srcdir)/System.Windows.Resources/StreamResourceInfo.cs		\
	$(srcdir)/System.Windows.Shapes/Ellipse.cs				\
	$(srcdir)/System.Windows.Shapes/Line.cs					\
	$(srcdir)/System.Windows.Shapes/Path.cs					\
	$(srcdir)/System.Windows.Shapes/Polygon.cs				\
	$(srcdir)/System.Windows.Shapes/Polyline.cs				\
	$(srcdir)/System.Windows.Shapes/Rectangle.cs				\
	$(srcdir)/System.Windows.Shapes/Shape.cs				\
	$(srcdir)/System.Windows.Threading/Dispatcher.cs			\
	$(srcdir)/System.Windows.Threading/DispatcherOperation.cs		\
	$(srcdir)/System.Windows.Threading/DispatcherSynchronizationContext.cs	\
	$(srcdir)/System.Windows.Threading/DispatcherTimer.cs			\
	$(WPFTOOLKIT)/VSM/System/Windows/TemplateVisualStateAttribute.cs	\
	$(WPFTOOLKIT)/VSM/System/Windows/VisualStateGroup.cs			\
	$(WPFTOOLKIT)/VSM/System/Windows/VisualStateChangedEventArgs.cs		\
	$(WPFTOOLKIT)/VSM/System/Windows/VisualStateManager.cs			\
	$(WPFTOOLKIT)/VSM/System/Windows/VisualTransition.cs			\
	$(MSCONTROLS)/Src/Resource.Designer.cs					\
	$(MSCONTROLS)/Src/Button/Button.cs					\
	$(MSCONTROLS)/Src/ButtonBase/ButtonBase.cs				\
	$(MSCONTROLS)/Src/ButtonBase/ClickMode.cs				\
	$(MSCONTROLS)/Src/CheckBox/CheckBox.cs					\
	$(MSCONTROLS)/Src/Common/DragCompletedEventArgs.cs			\
	$(MSCONTROLS)/Src/Common/DragDeltaEventArgs.cs				\
	$(MSCONTROLS)/Src/Common/DragStartedEventArgs.cs			\
	$(MSCONTROLS)/Src/Common/FontFamilyConverter.cs				\
	$(MSCONTROLS)/Src/Common/FontStretchConverter.cs			\
	$(MSCONTROLS)/Src/Common/FontStyleConverter.cs				\
	$(MSCONTROLS)/Src/Common/FontWeightConverter.cs				\
	$(MSCONTROLS)/Src/Common/KeyboardNavigation.cs				\
	$(MSCONTROLS)/Src/Common/NullableBoolConverter.cs			\
	$(MSCONTROLS)/Src/Common/RoutedPropertyChangedEventArgs.cs		\
	$(MSCONTROLS)/Src/Common/RoutedPropertyChangedEventHandler.cs		\
	$(MSCONTROLS)/Src/Common/SelectionChangedEventArgs.cs			\
	$(MSCONTROLS)/Src/Common/SelectionChangedEventHandler.cs		\
	$(MSCONTROLS)/Src/Common/TextDecorationCollectionConverter.cs		\
	$(MSCONTROLS)/Src/Common/TypeConverters.cs				\
	$(MSCONTROLS)/Src/Common/UriTypeConverter.cs				\
	$(MSCONTROLS)/Src/ContentControl/ContentControl.cs			\
	$(MSCONTROLS)/Src/ContentControl/ContentPresenter.cs			\
	$(MSCONTROLS)/Src/GlobalSuppressions.cs					\
	$(MSCONTROLS)/Src/ListBox/DisplayMemberValueConverter.cs		\
	$(MSCONTROLS)/Src/ListBox/ListBox.cs					\
	$(MSCONTROLS)/Src/ListBox/ListBoxItem.cs				\
	$(MSCONTROLS)/Src/ListBox/SelectionMode.cs				\
	$(MSCONTROLS)/Src/RadioButton/RadioButton.cs				\
	$(MSCONTROLS)/Src/RangeBase/RangeBase.cs				\
	$(MSCONTROLS)/Src/RepeatButton/RepeatButton.cs				\
	$(MSCONTROLS)/Src/ScrollBar/ScrollBar.cs				\
	$(MSCONTROLS)/Src/ScrollBar/ScrollBarVisibility.cs			\
	$(MSCONTROLS)/Src/ScrollBar/ScrollEventArgs.cs				\
	$(MSCONTROLS)/Src/ScrollBar/ScrollEventType.cs				\
	$(MSCONTROLS)/Src/ScrollViewer/ScrollViewer.cs				\
	$(MSCONTROLS)/Src/ScrollViewer/ScrollContentPresenter.cs		\
	$(MSCONTROLS)/Src/Thumb/Thumb.cs					\
	$(MSCONTROLS)/Src/ToggleButton/ToggleButton.cs				\
	$(MSCONTROLS)/Src/ToolTip/ToolTip.cs					\
	$(MSCONTROLS)/Src/ToolTip/ToolTipService.cs				\
	$(MSCONTROLS)/Extended/Src/Slider/Slider.cs

EXTRA_DIST = $(system_windows_sources) 	\
	scripts/checks.cs 			\
	scripts/README				\
	scripts/ChangeLog			\
	Assembly/ChangeLog			\
	Mono/ChangeLog				\
	System.Windows/ChangeLog		\
	System.Windows.Controls/ChangeLog	\
	System.Windows.Documents/ChangeLog	\
	System.Windows.Ink/ChangeLog		\
	System.Windows.Input/ChangeLog		\
	System.Windows.Media.Animation/ChangeLog\
	System.Windows.Media/ChangeLog		\
	System.Windows.Shapes/ChangeLog

$(assemblysl): $(system_windows_sources) Makefile $(top_builddir)/class/lib/2.1/Mono.Moonlight.dll
	$(SMCS) $(NET_2_1_ASSEMBLY_DEPS) -target:library -out:$@ $(system_windows_sources) -d:NET_2_1 -keyfile:$(srcdir)/../silverlight.pub -delaysign+
	sn -q -R $@ $(srcdir)/../mono.snk

$(assemblydesktop): $(system_windows_sources) Makefile $(top_builddir)/class/lib/3.0/Mono.Moonlight.dll
	$(GMCS) $(NET_3_0_ASSEMBLY_DEPS) -target:library -out:$@ $(system_windows_sources) -keyfile:$(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies) check-static check-dynamic
	-rm -rf *.exe *.mdb tester.cs

if INCLUDE_BROWSER_MANAGED_CODE
install-sl:
	$(GACUTIL) /i $(assemblysl)      /f /package 2.1
else
install-sl:
endif

if INCLUDE_DESKTOP_MANAGED_CODE
install-desktop:
	$(GACUTIL) /i $(assemblydesktop) /f /package 3.0
else
install-desktop:
endif

install-data-local: install-sl install-desktop

uninstall-local:
	$(GACUTIL) /u $(assembly)

check-code: check-static check-dynamic

check-static:
	gmcs $(srcdir)/scripts/checks.cs -r:../lib/3.0/System.Windows.dll -out:check-static.exe
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug check-static.exe
	> check-static

check-dynamic:
	gmcs $(srcdir)/scripts/checks.cs -r:../lib/3.0/System.Windows.dll -out:check-static.exe
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug check-static.exe
	mono --debug check-static.exe generate-test-code > tester.cs
	gmcs tester.cs -r:../lib/3.0/System.Windows.dll -debug:full -nowarn:219
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug tester.exe
