assembly = System.Windows
assemblysl = ../lib/2.1/$(assembly).dll
assemblydesktop = ../lib/3.0/$(assembly).dll

assemblies = $(assemblysl) $(assemblydesktop)

noinst_DATA = $(assemblies)

CSCFLAGS = /codepage:65001 -d:NET_1_1 -d:NET_2_0 -debug+ -noconfig -r:System -r:Mono.Moonlight -d:AGCLR -unsafe

GMCS = gmcs $(CSCFLAGS) -lib:../lib/3.0
SMCS = MONO_PATH="../lib/2.1:$$MONO_PATH" mono --runtime=moonlight --security=temporary-smcs-hack ../lib/2.1/smcs.exe $(CSCFLAGS)
GACUTIL = gacutil /gacdir $(prefix)/lib /root $(prefix)/lib

system_windows_sources =						\
	Internal.cs							\
	Assembly/AssemblyInfo.cs					\
	../Consts.cs							\
	Mono/EventHandlerList.cs					\
	Mono/Events.cs							\
	Mono/Hosting.cs							\
	Mono/ManagedXamlLoader.cs					\
	System.Windows.Controls/InkPresenter.cs				\
	System.Windows.Controls/MediaElement.cs				\
	System.Windows.Ink/DrawingAttributes.cs				\
	System.Windows.Ink/Stroke.cs					\
	System.Windows.Ink/StrokeCollection.cs				\
	System.Windows.Input/StylusInfo.cs				\
	System.Windows.Input/StylusPoint.cs				\
	System.Windows.Input/StylusPointCollection.cs			\
	System.Windows.Input/TabletDeviceType.cs			\
	System.Windows.Input/TimelineMarkerEventArgs.cs			\
	System.Windows.Input/TimelineMarkerEventHandler.cs		\
	System.Windows.Media.Animation/ColorAnimationUsingKeyFrames.cs	\
	System.Windows.Media.Animation/ColorKeyFrame.cs			\
	System.Windows.Media.Animation/ColorKeyFrameCollection.cs	\
	System.Windows.Media.Animation/DiscreteColorKeyFrame.cs		\
	System.Windows.Media.Animation/DiscreteDoubleKeyFrame.cs	\
	System.Windows.Media.Animation/DiscretePointKeyFrame.cs		\
	System.Windows.Media.Animation/DoubleKeyFrame.cs		\
	System.Windows.Media.Animation/DoubleKeyFrameCollection.cs	\
	System.Windows.Media.Animation/KeyFrame.cs			\
	System.Windows.Media.Animation/KeyTime.cs			\
	System.Windows.Media.Animation/KeyTimeType.cs			\
	System.Windows.Media.Animation/LinearColorKeyFrame.cs		\
	System.Windows.Media.Animation/LinearDoubleKeyFrame.cs		\
	System.Windows.Media.Animation/LinearPointKeyFrame.cs		\
	System.Windows.Media.Animation/PointAnimationUsingKeyFrames.cs	\
	System.Windows.Media.Animation/PointKeyFrame.cs			\
	System.Windows.Media.Animation/PointKeyFrameCollection.cs	\
	System.Windows.Media.Animation/SplineColorKeyFrame.cs		\
	System.Windows.Media.Animation/SplinePointKeyFrame.cs		\
	System.Windows.Media.Animation/TimelineMarker.cs		\
	System.Windows.Media.Animation/TimelineMarkerCollection.cs	\
	System.Windows/Downloader.cs					\
	System.Windows/Duration.cs					\
	System.Windows/ErrorEventArgs.cs				\
	System.Windows/ErrorEventHandler.cs				\
	System.Windows/ErrorType.cs					\
	System.Windows/EventTrigger.cs					\
	System.Windows/FontStretches.cs					\
	System.Windows/FontStyles.cs					\
	System.Windows/FontWeights.cs					\
	System.Windows/FrameworkElement.cs				\
	System.Windows/Inlines.cs					\
	System.Windows/ParserErrorEventArgs.cs				\
	System.Windows/ParserErrorEventHandler.cs			\
	System.Windows/Point.cs						\
	System.Windows/ResourceDictionary.cs				\
	System.Windows/Rect.cs						\
	System.Windows/RuntimeErrorEventArgs.cs				\
	System.Windows/RuntimeErrorEventHandler.cs			\
	System.Windows/Size.cs						\
	System.Windows/StyleSimulations.cs				\
	System.Windows/TextDecorations.cs				\
	System.Windows/TextWrapping.cs					\
	System.Windows/TriggerCollection.cs				\
	System.Windows/TriggerActionCollection.cs			\
	System.Windows/UIElement.cs					\
	System.Windows/Visibility.cs					\
	System.Windows/XamlReader.cs					\
	System.Windows.Controls/Canvas.cs				\
	System.Windows.Controls/Control.cs				\
	System.Windows.Controls/Image.cs				\
	System.Windows.Controls/MediaBase.cs				\
	System.Windows.Controls/Panel.cs				\
	System.Windows.Controls/TextBlock.cs				\
	System.Windows.Documents/Glyphs.cs				\
	System.Windows.Documents/Inline.cs				\
	System.Windows.Documents/LineBreak.cs				\
	System.Windows.Documents/Run.cs					\
	System.Windows/DependencyObject.cs				\
	System.Windows/DependencyProperty.cs				\
	System.Windows.Input/Cursors.cs					\
	System.Windows.Input/KeyboardEventArgs.cs			\
	System.Windows.Input/KeyboardEventHandler.cs			\
	System.Windows.Input/MouseEventArgs.cs				\
	System.Windows.Input/MouseEventHandler.cs			\
	System.Windows.Interop/BrowserHost.cs				\
	System.Windows.Markup/XamlParseException.cs			\
	System.Windows.Media/AlignmentX.cs				\
	System.Windows.Media/AlignmentY.cs				\
	System.Windows.Media/ArcSegment.cs				\
	System.Windows.Media/BezierSegment.cs				\
	System.Windows.Media/Brush.cs					\
	System.Windows.Media/BrushMappingMode.cs			\
	System.Windows.Media/Color.cs					\
	System.Windows.Media/Colors.cs					\
	System.Windows.Media/ColorInterpolationMode.cs			\
	System.Windows.Media/EllipseGeometry.cs				\
	System.Windows.Media/FillRule.cs				\
	System.Windows.Media/Geometry.cs				\
	System.Windows.Media/GeometryCollection.cs			\
	System.Windows.Media/GeometryGroup.cs				\
	System.Windows.Media/GradientBrush.cs				\
	System.Windows.Media/GradientSpreadMethod.cs			\
	System.Windows.Media/GradientStop.cs				\
	System.Windows.Media/GradientStopCollection.cs			\
	System.Windows.Media/ImageBrush.cs				\
	System.Windows.Media/LinearGradientBrush.cs			\
	System.Windows.Media/LineGeometry.cs				\
	System.Windows.Media/LineSegment.cs				\
	System.Windows.Media/MatrixTransform.cs				\
	System.Windows.Media/Matrix.cs					\
	System.Windows.Media/MediaAttribute.cs				\
	System.Windows.Media/MediaAttributeCollection.cs		\
	System.Windows.Media/PathFigure.cs				\
	System.Windows.Media/PathFigureCollection.cs			\
	System.Windows.Media/PathGeometry.cs				\
	System.Windows.Media/PathSegment.cs				\
	System.Windows.Media/PathSegmentCollection.cs			\
	System.Windows.Media/PenLineCap.cs				\
	System.Windows.Media/PenLineJoin.cs				\
	System.Windows.Media/PolyBezierSegment.cs			\
	System.Windows.Media/PolyLineSegment.cs				\
	System.Windows.Media/PolyQuadraticBezierSegment.cs		\
	System.Windows.Media/QuadraticBezierSegment.cs			\
	System.Windows.Media/RadialGradientBrush.cs			\
	System.Windows.Media/RectangleGeometry.cs			\
	System.Windows.Media/RotateTransform.cs				\
	System.Windows.Media/ScaleTransform.cs				\
	System.Windows.Media/SkewTransform.cs				\
	System.Windows.Media/SolidColorBrush.cs				\
	System.Windows.Media/Stretch.cs					\
	System.Windows.Media/SweepDirection.cs				\
	System.Windows.Media/TileBrush.cs				\
	System.Windows.Media/TimelineCollection.cs			\
	System.Windows.Media/TransformCollection.cs			\
	System.Windows.Media/Transform.cs				\
	System.Windows.Media/TransformGroup.cs				\
	System.Windows.Media/TranslateTransform.cs			\
	System.Windows.Media/VideoBrush.cs				\
	System.Windows.Media/VisualBrush.cs				\
	System.Windows.Media/VisualCollection.cs			\
	System.Windows.Media/Visual.cs					\
	System.Windows.Media.Animation/Animation.cs			\
	System.Windows.Media.Animation/BeginStoryBoard.cs		\
	System.Windows.Media.Animation/ColorAnimation.cs		\
	System.Windows.Media.Animation/DoubleAnimation.cs		\
	System.Windows.Media.Animation/DoubleAnimationUsingKeyFrames.cs	\
	System.Windows.Media.Animation/FillBehavior.cs			\
	System.Windows.Media.Animation/KeySpline.cs			\
	System.Windows.Media.Animation/ParallelTimeline.cs		\
	System.Windows.Media.Animation/PointAnimation.cs		\
	System.Windows.Media.Animation/RepeatBehavior.cs		\
	System.Windows.Media.Animation/SplineDoubleKeyFrame.cs		\
	System.Windows.Media.Animation/Storyboard.cs			\
	System.Windows.Media.Animation/Timeline.cs			\
	System.Windows.Media.Animation/TimelineGroup.cs			\
	System.Windows.Shapes/Ellipse.cs				\
	System.Windows.Shapes/Line.cs					\
	System.Windows.Shapes/Path.cs					\
	System.Windows.Shapes/Polygon.cs				\
	System.Windows.Shapes/Polyline.cs				\
	System.Windows.Shapes/Rectangle.cs				\
	System.Windows.Shapes/Shape.cs

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
	$(SMCS) -target:library -out:$@ $(system_windows_sources) -d:NET_2_1
	sn -q -R $@ $(srcdir)/../mono.snk

$(assemblydesktop): $(system_windows_sources) Makefile $(top_builddir)/class/lib/3.0/Mono.Moonlight.dll
	$(GMCS) -target:library -out:$@ $(system_windows_sources)
	sn -q -R $@ $(srcdir)/../mono.snk

clean-local:
	-rm -rf $(assemblies) check-static check-dynamic
	-rm -rf *.exe *.mdb tester.cs

install-data-local:
	$(GACUTIL) /i $(assemblydesktop) /f /package 3.0
	$(GACUTIL) /i $(assemblysl)      /f /package 2.1

uninstall-local:
	$(GACUTIL) /u $(assembly)

check: check-static check-dynamic

check-static:
	gmcs $(srcdir)/scripts/checks.cs -r:../lib/3.0/System.Windows.dll -out:check-static.exe
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug check-static.exe
	> check-static

check-dynamic:
	gmcs $(srcdir)/scripts/checks.cs -r:../lib/3.0/System.Windows.dll -out:check-static.exe
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug check-static.exe
	mono --debug checks.exe generate-test-code > tester.cs
	gmcs tester.cs -r:../lib/3.0/System.Windows.dll -debug:full -nowarn:219
	MONO_PATH=../lib/3.0:$$MONO_PATH mono --debug tester.exe
