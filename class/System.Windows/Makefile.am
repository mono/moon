assembly = System.Windows
assemblysl = ../lib/2.1/$(assembly).dll
assemblydesktop = ../lib/3.0/$(assembly).dll

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -r:System.Core -r:Mono.Moonlight -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS) -lib:../lib/3.0
SMCS = MONO_PATH="../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(DESTDIR)$(prefix)/lib /root $(DESTDIR)$(prefix)/lib

system_windows_sources =							\
	$(srcdir)/Assembly/AssemblyInfo.cs					\
	$(srcdir)/Assembly/MonoTODOAttribute.cs					\
	$(srcdir)/../Consts.cs							\
	$(srcdir)/Mono/EventHandlerList.cs					\
	$(srcdir)/Mono/Events.cs						\
	$(srcdir)/Mono/Hosting.cs						\
	$(srcdir)/Mono/ManagedXamlLoader.cs					\
	$(srcdir)/Mono/NativeMethods.cs						\
	$(srcdir)/Mono/Report.cs						\
	$(srcdir)/System.Collections.ObjectModel/ObservableCollection_T.cs	\
	$(srcdir)/System.Collections.Specialized/INotifyCollectionChanged.cs	\
	$(srcdir)/System.Collections.Specialized/NotifyCollectionChangedAction.cs \
	$(srcdir)/System.Collections.Specialized/NotifyCollectionChangedEventArgs.cs \
	$(srcdir)/System.Collections.Specialized/NotifyCollectionChangedEventHandler.cs \
	$(srcdir)/System.Windows.Controls/InkPresenter.cs			\
	$(srcdir)/System.Windows.Controls/MediaElement.cs			\
	$(srcdir)/System.Windows.Ink/DrawingAttributes.cs			\
	$(srcdir)/System.Windows.Ink/Stroke.cs					\
	$(srcdir)/System.Windows.Ink/StrokeCollection.cs			\
	$(srcdir)/System.Windows.Input/StylusInfo.cs				\
	$(srcdir)/System.Windows.Input/StylusPoint.cs				\
	$(srcdir)/System.Windows.Input/StylusPointCollection.cs			\
	$(srcdir)/System.Windows.Input/TabletDeviceType.cs			\
	$(srcdir)/System.Windows.Input/TimelineMarkerEventArgs.cs		\
	$(srcdir)/System.Windows.Input/TimelineMarkerEventHandler.cs		\
	$(srcdir)/System.Windows.Media.Animation/ColorAnimationUsingKeyFrames.cs\
	$(srcdir)/System.Windows.Media.Animation/ColorKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/ColorKeyFrameCollection.cs	\
	$(srcdir)/System.Windows.Media.Animation/DiscreteColorKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/DiscreteDoubleKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/DiscretePointKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/DoubleKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/DoubleKeyFrameCollection.cs	\
	$(srcdir)/System.Windows.Media.Animation/KeyFrame.cs			\
	$(srcdir)/System.Windows.Media.Animation/KeyTime.cs			\
	$(srcdir)/System.Windows.Media.Animation/KeyTimeType.cs			\
	$(srcdir)/System.Windows.Media.Animation/LinearColorKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/LinearDoubleKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/LinearPointKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/PointAnimationUsingKeyFrames.cs\
	$(srcdir)/System.Windows.Media.Animation/PointKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/PointKeyFrameCollection.cs	\
	$(srcdir)/System.Windows.Media.Animation/SplineColorKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/SplinePointKeyFrame.cs		\
	$(srcdir)/System.Windows.Media.Animation/TimelineMarker.cs		\
	$(srcdir)/System.Windows.Media.Animation/TimelineMarkerCollection.cs	\
	$(srcdir)/System.Windows/Application.cs					\
	$(srcdir)/System.Windows/ApplicationUnhandledExceptionEventArgs.cs	\
	$(srcdir)/System.Windows/AssemblyPart.cs				\
	$(srcdir)/System.Windows/AssemblyPartCollection.cs			\
	$(srcdir)/System.Windows/CrossDomainAccess.cs					\
	$(srcdir)/System.Windows/DataTemplate.cs				\
	$(srcdir)/System.Windows/Deployment.cs					\
	$(srcdir)/System.Windows/DependencyObject.cs				\
	$(srcdir)/System.Windows/DependencyProperty.cs				\
	$(srcdir)/System.Windows/DependencyPropertyChangedEventArgs.cs		\
	$(srcdir)/System.Windows/Downloader.cs					\
	$(srcdir)/System.Windows/Duration.cs					\
	$(srcdir)/System.Windows/ErrorEventArgs.cs				\
	$(srcdir)/System.Windows/ErrorEventHandler.cs				\
	$(srcdir)/System.Windows/ErrorType.cs					\
	$(srcdir)/System.Windows/EventTrigger.cs				\
	$(srcdir)/System.Windows/FontStretches.cs				\
	$(srcdir)/System.Windows/FontStyles.cs					\
	$(srcdir)/System.Windows/FontWeights.cs					\
	$(srcdir)/System.Windows/FrameworkElement.cs				\
	$(srcdir)/System.Windows/FrameworkTemplate.cs				\
	$(srcdir)/System.Windows/GridLength.cs					\
	$(srcdir)/System.Windows/GridUnitType.cs				\
	$(srcdir)/System.Windows/HorizontalAlignment.cs				\
	$(srcdir)/System.Windows/ParserErrorEventArgs.cs			\
	$(srcdir)/System.Windows/ParserErrorEventHandler.cs			\
	$(srcdir)/System.Windows/Point.cs					\
	$(srcdir)/System.Windows/PresentationFrameworkCollection.cs		\
	$(srcdir)/System.Windows/PropertyChangedCallback.cs			\
	$(srcdir)/System.Windows/ResourceDictionary.cs				\
	$(srcdir)/System.Windows/Rect.cs					\
	$(srcdir)/System.Windows/RoutedEventArgs.cs				\
	$(srcdir)/System.Windows/RoutedEventHandler.cs				\
	$(srcdir)/System.Windows/RuntimeErrorEventArgs.cs			\
	$(srcdir)/System.Windows/RuntimeErrorEventHandler.cs			\
	$(srcdir)/System.Windows/Size.cs					\
	$(srcdir)/System.Windows/StartupEventArgs.cs				\
	$(srcdir)/System.Windows/StartupEventHandler.cs				\
	$(srcdir)/System.Windows/StyleSimulations.cs				\
	$(srcdir)/System.Windows/TextAlignment.cs				\
	$(srcdir)/System.Windows/TextDecorations.cs				\
	$(srcdir)/System.Windows/TextWrapping.cs				\
	$(srcdir)/System.Windows/TriggerCollection.cs				\
	$(srcdir)/System.Windows/TriggerActionCollection.cs			\
	$(srcdir)/System.Windows/UIElement.cs					\
	$(srcdir)/System.Windows/VerticalAlignment.cs				\
	$(srcdir)/System.Windows/Visibility.cs					\
	$(srcdir)/System.Windows/XamlReader.cs					\
	$(srcdir)/System.Windows/XapHackProxyImpl.cs				\
	$(srcdir)/System.Windows.Controls/Canvas.cs				\
	$(srcdir)/System.Windows.Controls/ColumnDefinition.cs			\
	$(srcdir)/System.Windows.Controls/ColumnDefinitionCollection.cs		\
	$(srcdir)/System.Windows.Controls/Control.cs				\
	$(srcdir)/System.Windows.Controls/Grid.cs				\
	$(srcdir)/System.Windows.Controls/Image.cs				\
	$(srcdir)/System.Windows.Controls/MediaBase.cs				\
	$(srcdir)/System.Windows.Controls/Panel.cs				\
	$(srcdir)/System.Windows.Controls/TextBlock.cs				\
	$(srcdir)/System.Windows.Controls/UserControl.cs			\
	$(srcdir)/System.Windows.Documents/Glyphs.cs				\
	$(srcdir)/System.Windows.Documents/Inline.cs				\
	$(srcdir)/System.Windows.Documents/InlineCollection.cs			\
	$(srcdir)/System.Windows.Documents/LineBreak.cs				\
	$(srcdir)/System.Windows.Documents/Run.cs				\
	$(srcdir)/System.Windows.Controls/RowDefinition.cs			\
	$(srcdir)/System.Windows.Controls/RowDefinitionCollection.cs		\
	$(srcdir)/System.Windows.Input/Cursors.cs				\
	$(srcdir)/System.Windows.Input/KeyboardEventArgs.cs			\
	$(srcdir)/System.Windows.Input/KeyboardEventHandler.cs			\
	$(srcdir)/System.Windows.Input/Key.cs					\
	$(srcdir)/System.Windows.Input/KeyEventArgs.cs				\
	$(srcdir)/System.Windows.Input/KeyEventHandler.cs			\
	$(srcdir)/System.Windows.Input/MouseButtonEventArgs.cs			\
	$(srcdir)/System.Windows.Input/MouseButtonEventHandler.cs		\
	$(srcdir)/System.Windows.Input/MouseEventArgs.cs			\
	$(srcdir)/System.Windows.Input/MouseEventHandler.cs			\
	$(srcdir)/System.Windows.Interop/BrowserHost.cs				\
	$(srcdir)/System.Windows.Interop/Content.cs				\
	$(srcdir)/System.Windows.Interop/Settings.cs				\
	$(srcdir)/System.Windows.Interop/SilverlightHost.cs			\
	$(srcdir)/System.Windows.Markup/XamlParseException.cs			\
	$(srcdir)/System.Windows.Markup/XmlnsDefinitionAttribute.cs		\
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
	$(srcdir)/System.Windows.Media/EllipseGeometry.cs			\
	$(srcdir)/System.Windows.Media/FillRule.cs				\
	$(srcdir)/System.Windows.Media/Geometry.cs				\
	$(srcdir)/System.Windows.Media/GeometryCollection.cs			\
	$(srcdir)/System.Windows.Media/GeometryGroup.cs				\
	$(srcdir)/System.Windows.Media/GradientBrush.cs				\
	$(srcdir)/System.Windows.Media/GradientSpreadMethod.cs			\
	$(srcdir)/System.Windows.Media/GradientStop.cs				\
	$(srcdir)/System.Windows.Media/GradientStopCollection.cs		\
	$(srcdir)/System.Windows.Media/ImageBrush.cs				\
	$(srcdir)/System.Windows.Media/LinearGradientBrush.cs			\
	$(srcdir)/System.Windows.Media/LineGeometry.cs				\
	$(srcdir)/System.Windows.Media/LineSegment.cs				\
	$(srcdir)/System.Windows.Media/MatrixTransform.cs			\
	$(srcdir)/System.Windows.Media/Matrix.cs				\
	$(srcdir)/System.Windows.Media/MediaAttribute.cs			\
	$(srcdir)/System.Windows.Media/MediaAttributeCollection.cs		\
	$(srcdir)/System.Windows.Media/PathFigure.cs				\
	$(srcdir)/System.Windows.Media/PathFigureCollection.cs			\
	$(srcdir)/System.Windows.Media/PathGeometry.cs				\
	$(srcdir)/System.Windows.Media/PathSegment.cs				\
	$(srcdir)/System.Windows.Media/PathSegmentCollection.cs			\
	$(srcdir)/System.Windows.Media/PenLineCap.cs				\
	$(srcdir)/System.Windows.Media/PenLineJoin.cs				\
	$(srcdir)/System.Windows.Media/PolyBezierSegment.cs			\
	$(srcdir)/System.Windows.Media/PolyLineSegment.cs			\
	$(srcdir)/System.Windows.Media/PolyQuadraticBezierSegment.cs		\
	$(srcdir)/System.Windows.Media/QuadraticBezierSegment.cs		\
	$(srcdir)/System.Windows.Media/RadialGradientBrush.cs			\
	$(srcdir)/System.Windows.Media/RectangleGeometry.cs			\
	$(srcdir)/System.Windows.Media/RotateTransform.cs			\
	$(srcdir)/System.Windows.Media/ScaleTransform.cs			\
	$(srcdir)/System.Windows.Media/SkewTransform.cs				\
	$(srcdir)/System.Windows.Media/SolidColorBrush.cs			\
	$(srcdir)/System.Windows.Media/Stretch.cs				\
	$(srcdir)/System.Windows.Media/SweepDirection.cs			\
	$(srcdir)/System.Windows.Media/TileBrush.cs				\
	$(srcdir)/System.Windows.Media/TimelineCollection.cs			\
	$(srcdir)/System.Windows.Media/TransformCollection.cs			\
	$(srcdir)/System.Windows.Media/Transform.cs				\
	$(srcdir)/System.Windows.Media/TransformGroup.cs			\
	$(srcdir)/System.Windows.Media/TranslateTransform.cs			\
	$(srcdir)/System.Windows.Media/VideoBrush.cs				\
	$(srcdir)/System.Windows.Media/VisualBrush.cs				\
	$(srcdir)/System.Windows.Media/VisualCollection.cs			\
	$(srcdir)/System.Windows.Media/Visual.cs				\
	$(srcdir)/System.Windows.Media.Animation/Animation.cs			\
	$(srcdir)/System.Windows.Media.Animation/BeginStoryBoard.cs		\
	$(srcdir)/System.Windows.Media.Animation/ColorAnimation.cs		\
	$(srcdir)/System.Windows.Media.Animation/DoubleAnimation.cs		\
	$(srcdir)/System.Windows.Media.Animation/DoubleAnimationUsingKeyFrames.cs\
	$(srcdir)/System.Windows.Media.Animation/FillBehavior.cs		\
	$(srcdir)/System.Windows.Media.Animation/KeySpline.cs			\
	$(srcdir)/System.Windows.Media.Animation/ParallelTimeline.cs		\
	$(srcdir)/System.Windows.Media.Animation/PointAnimation.cs		\
	$(srcdir)/System.Windows.Media.Animation/RepeatBehavior.cs		\
	$(srcdir)/System.Windows.Media.Animation/SplineDoubleKeyFrame.cs	\
	$(srcdir)/System.Windows.Media.Animation/Storyboard.cs			\
	$(srcdir)/System.Windows.Media.Animation/Timeline.cs			\
	$(srcdir)/System.Windows.Media.Animation/TimelineGroup.cs		\
	$(srcdir)/System.Windows.Resources/StreamResourceInfo.cs		\
	$(srcdir)/System.Windows.Shapes/Ellipse.cs				\
	$(srcdir)/System.Windows.Shapes/Line.cs					\
	$(srcdir)/System.Windows.Shapes/Path.cs					\
	$(srcdir)/System.Windows.Shapes/Polygon.cs				\
	$(srcdir)/System.Windows.Shapes/Polyline.cs				\
	$(srcdir)/System.Windows.Shapes/Rectangle.cs				\
	$(srcdir)/System.Windows.Shapes/Shape.cs

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
	$(SMCS) -target:library -out:$@ $(system_windows_sources) -d:NET_2_1 -keyfile:$(srcdir)/../silverlight.pub -delaysign+
	sn -q -R $@ $(srcdir)/../mono.snk

$(assemblydesktop): $(system_windows_sources) Makefile $(top_builddir)/class/lib/3.0/Mono.Moonlight.dll
	$(GMCS) -target:library -out:$@ $(system_windows_sources) -keyfile:$(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies) check-static check-dynamic
	-rm -rf *.exe *.mdb tester.cs

install-data-local:
	$(GACUTIL) /i $(assemblydesktop) /f /package 3.0
	$(GACUTIL) /i $(assemblysl)      /f /package 2.1

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
