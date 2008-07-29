/*
 * Automatically generated, do not edit this file directly
 * To regenerate execute typegen.sh
 */
#include <config.h>
#include <stdlib.h>

#include "geometry.h"
#include "animation.h"
#include "brush.h"
#include "canvas.h"
#include "collection.h"
#include "control.h"
#include "array.h"
#include "downloader.h"
#include "stylus.h"
#include "shape.h"
#include "trigger.h"
#include "frameworkelement.h"
#include "text.h"
#include "media.h"
#include "transform.h"
#include "window-gtk.h"
#include "eventargs.h"
#include "panel.h"
#include "clock.h"
#include "runtime.h"
#include "visual.h"
#include "xaml.h"
#if SL_2_0
#include "deployment.h"
#include "grid.h"
#include "usercontrol.h"
#endif

const int Clock::CompletedEvent = 1;
const int Clock::CurrentGlobalSpeedInvalidatedEvent = 2;
const int Clock::CurrentStateInvalidatedEvent = 3;
const int Clock::CurrentTimeInvalidatedEvent = 4;
const int Downloader::CompletedEvent = 1;
const int Downloader::DownloadFailedEvent = 2;
const int Downloader::DownloadProgressChangedEvent = 3;
const int EventObject::DestroyedEvent = 0;
const int Image::ImageFailedEvent = 13;
const int ImageBrush::DownloadProgressChangedEvent = 1;
const int ImageBrush::ImageFailedEvent = 2;
const int MediaBase::DownloadProgressChangedEvent = 12;
const int MediaElement::BufferingProgressChangedEvent = 13;
const int MediaElement::CurrentStateChangedEvent = 14;
const int MediaElement::MarkerReachedEvent = 15;
const int MediaElement::MediaEndedEvent = 16;
const int MediaElement::MediaFailedEvent = 17;
const int MediaElement::MediaOpenedEvent = 18;
const int Storyboard::CompletedEvent = 1;
const int Surface::ErrorEvent = 1;
const int Surface::FullScreenChangeEvent = 2;
const int Surface::LoadEvent = 3;
const int Surface::ResizeEvent = 4;
const int TimeManager::RenderEvent = 1;
const int TimeManager::UpdateInputEvent = 2;
const int TimeSource::TickEvent = 1;
const int UIElement::GotFocusEvent = 1;
const int UIElement::InvalidatedEvent = 2;
const int UIElement::KeyDownEvent = 3;
const int UIElement::KeyUpEvent = 4;
const int UIElement::LoadedEvent = 5;
const int UIElement::LostFocusEvent = 6;
const int UIElement::MouseEnterEvent = 7;
const int UIElement::MouseLeaveEvent = 8;
const int UIElement::MouseLeftButtonDownEvent = 9;
const int UIElement::MouseLeftButtonUpEvent = 10;
const int UIElement::MouseMoveEvent = 11;

const char *Clock_Events [] = { "Completed", "CurrentGlobalSpeedInvalidated", "CurrentStateInvalidated", "CurrentTimeInvalidated", NULL };
const char *Downloader_Events [] = { "Completed", "DownloadFailed", "DownloadProgressChanged", NULL };
const char *EventObject_Events [] = { "Destroyed", NULL };
const char *Image_Events [] = { "ImageFailed", NULL };
const char *ImageBrush_Events [] = { "DownloadProgressChanged", "ImageFailed", NULL };
const char *MediaBase_Events [] = { "DownloadProgressChanged", NULL };
const char *MediaElement_Events [] = { "BufferingProgressChanged", "CurrentStateChanged", "MarkerReached", "MediaEnded", "MediaFailed", "MediaOpened", NULL };
const char *Storyboard_Events [] = { "Completed", NULL };
const char *Surface_Events [] = { "Error", "FullScreenChange", "Load", "Resize", NULL };
const char *TimeManager_Events [] = { "Render", "UpdateInput", NULL };
const char *TimeSource_Events [] = { "Tick", NULL };
const char *UIElement_Events [] = { "GotFocus", "Invalidated", "KeyDown", "KeyUp", "Loaded", "LostFocus", "MouseEnter", "MouseLeave", "MouseLeftButtonDown", "MouseLeftButtonUp", "MouseMove", NULL };

Type type_infos [] = {
	{ Type::INVALID, Type::INVALID, false, "INVALID", NULL, 0, 0, NULL, NULL, NULL },
	{ Type::ANIMATION, Type::TIMELINE, false, "Animation", "ANIMATION", 0, 1, NULL, NULL, NULL }, 
	{ Type::ANIMATIONCLOCK, Type::CLOCK, false, "AnimationClock", "ANIMATIONCLOCK", 0, 5, NULL, NULL, NULL }, 
#if SL_2_0
	{ Type::APPLICATION, Type::DEPENDENCY_OBJECT, false, "Application", "APPLICATION", 0, 1, NULL, (create_inst_func *) application_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'APPLICATION'", "APPLICATION", 0, 0, NULL, NULL, NULL }, 
#endif
	{ Type::ARCSEGMENT, Type::PATHSEGMENT, false, "ArcSegment", "ARCSEGMENT", 0, 1, NULL, (create_inst_func *) arc_segment_new, NULL }, 
#if SL_2_0
	{ Type::ASSEMBLYPART, Type::DEPENDENCY_OBJECT, false, "AssemblyPart", "ASSEMBLYPART", 0, 1, NULL, (create_inst_func *) assembly_part_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'ASSEMBLYPART'", "ASSEMBLYPART", 0, 0, NULL, NULL, NULL }, 
#endif
#if SL_2_0
	{ Type::ASSEMBLYPART_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "AssemblyPartCollection", "ASSEMBLYPART_COLLECTION", 0, 1, NULL, (create_inst_func *) assembly_part_collection_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'ASSEMBLYPART_COLLECTION'", "ASSEMBLYPART_COLLECTION", 0, 0, NULL, NULL, NULL }, 
#endif
	{ Type::BEGINSTORYBOARD, Type::TRIGGERACTION, false, "BeginStoryboard", "BEGINSTORYBOARD", 0, 1, NULL, (create_inst_func *) begin_storyboard_new, "Storyboard" }, 
	{ Type::BEZIERSEGMENT, Type::PATHSEGMENT, false, "BezierSegment", "BEZIERSEGMENT", 0, 1, NULL, (create_inst_func *) bezier_segment_new, NULL }, 
	{ Type::BOOL, Type::INVALID, false, "bool", "BOOL", 0, 0, NULL, NULL, NULL }, 
	{ Type::BRUSH, Type::DEPENDENCY_OBJECT, false, "Brush", "BRUSH", 0, 1, NULL, (create_inst_func *) brush_new, NULL }, 
	{ Type::CANVAS, Type::PANEL, false, "Canvas", "CANVAS", 0, 12, NULL, (create_inst_func *) canvas_new, NULL }, 
	{ Type::CLOCK, Type::DEPENDENCY_OBJECT, false, "Clock", "CLOCK", 4, 5, Clock_Events, NULL, NULL }, 
	{ Type::CLOCKGROUP, Type::CLOCK, false, "ClockGroup", "CLOCKGROUP", 0, 5, NULL, NULL, NULL }, 
	{ Type::COLLECTION, Type::DEPENDENCY_OBJECT, false, "Collection", "COLLECTION", 0, 1, NULL, (create_inst_func *) collection_new, NULL }, 
	{ Type::COLOR, Type::INVALID, false, "Color", "COLOR", 0, 0, NULL, NULL, NULL }, 
	{ Type::COLORANIMATION, Type::ANIMATION, false, "ColorAnimation", "COLORANIMATION", 0, 1, NULL, (create_inst_func *) color_animation_new, NULL }, 
	{ Type::COLORANIMATIONUSINGKEYFRAMES, Type::COLORANIMATION, false, "ColorAnimationUsingKeyFrames", "COLORANIMATIONUSINGKEYFRAMES", 0, 1, NULL, (create_inst_func *) color_animation_using_key_frames_new, "KeyFrames" }, 
	{ Type::COLORKEYFRAME, Type::KEYFRAME, false, "ColorKeyFrame", "COLORKEYFRAME", 0, 1, NULL, (create_inst_func *) color_key_frame_new, NULL }, 
	{ Type::COLORKEYFRAME_COLLECTION, Type::KEYFRAME_COLLECTION, false, "ColorKeyFrameCollection", "COLORKEYFRAME_COLLECTION", 0, 1, NULL, (create_inst_func *) color_key_frame_collection_new, NULL }, 
#if SL_2_0
	{ Type::COLUMNDEFINITION, Type::DEPENDENCY_OBJECT, false, "ColumnDefinition", "COLUMNDEFINITION", 0, 1, NULL, (create_inst_func *) column_definition_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'COLUMNDEFINITION'", "COLUMNDEFINITION", 0, 0, NULL, NULL, NULL }, 
#endif
#if SL_2_0
	{ Type::COLUMNDEFINITION_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "ColumnDefinitionCollection", "COLUMNDEFINITION_COLLECTION", 0, 1, NULL, (create_inst_func *) column_definition_collection_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'COLUMNDEFINITION_COLLECTION'", "COLUMNDEFINITION_COLLECTION", 0, 0, NULL, NULL, NULL }, 
#endif
	{ Type::CONTROL, Type::FRAMEWORKELEMENT, false, "Control", "CONTROL", 0, 12, NULL, (create_inst_func *) control_new, "Content" }, 
	{ Type::DEPENDENCY_OBJECT, Type::EVENTOBJECT, false, "DependencyObject", "DEPENDENCY_OBJECT", 0, 1, NULL, NULL, NULL }, 
	{ Type::DEPENDENCY_OBJECT_COLLECTION, Type::COLLECTION, false, "DependencyObjectCollection", "DEPENDENCY_OBJECT_COLLECTION", 0, 1, NULL, (create_inst_func *) dependency_object_collection_new, NULL }, 
#if SL_2_0
	{ Type::DEPLOYMENT, Type::DEPENDENCY_OBJECT, false, "Deployment", "DEPLOYMENT", 0, 1, NULL, (create_inst_func *) deployment_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'DEPLOYMENT'", "DEPLOYMENT", 0, 0, NULL, NULL, NULL }, 
#endif
	{ Type::DISCRETECOLORKEYFRAME, Type::COLORKEYFRAME, false, "DiscreteColorKeyFrame", "DISCRETECOLORKEYFRAME", 0, 1, NULL, (create_inst_func *) discrete_color_key_frame_new, NULL }, 
	{ Type::DISCRETEDOUBLEKEYFRAME, Type::DOUBLEKEYFRAME, false, "DiscreteDoubleKeyFrame", "DISCRETEDOUBLEKEYFRAME", 0, 1, NULL, (create_inst_func *) discrete_double_key_frame_new, NULL }, 
	{ Type::DISCRETEPOINTKEYFRAME, Type::POINTKEYFRAME, false, "DiscretePointKeyFrame", "DISCRETEPOINTKEYFRAME", 0, 1, NULL, (create_inst_func *) discrete_point_key_frame_new, NULL }, 
	{ Type::DOUBLE, Type::INVALID, false, "double", "DOUBLE", 0, 0, NULL, NULL, NULL }, 
	{ Type::DOUBLE_ARRAY, Type::INVALID, false, "double*", "DOUBLE_ARRAY", 0, 0, NULL, NULL, NULL }, 
	{ Type::DOUBLE_COLLECTION, Type::COLLECTION, false, "DoubleCollection", "DOUBLE_COLLECTION", 0, 1, NULL, (create_inst_func *) double_collection_new, NULL }, 
	{ Type::DOUBLEANIMATION, Type::ANIMATION, false, "DoubleAnimation", "DOUBLEANIMATION", 0, 1, NULL, (create_inst_func *) double_animation_new, NULL }, 
	{ Type::DOUBLEANIMATIONUSINGKEYFRAMES, Type::DOUBLEANIMATION, false, "DoubleAnimationUsingKeyFrames", "DOUBLEANIMATIONUSINGKEYFRAMES", 0, 1, NULL, (create_inst_func *) double_animation_using_key_frames_new, "KeyFrames" }, 
	{ Type::DOUBLEKEYFRAME, Type::KEYFRAME, false, "DoubleKeyFrame", "DOUBLEKEYFRAME", 0, 1, NULL, (create_inst_func *) double_key_frame_new, NULL }, 
	{ Type::DOUBLEKEYFRAME_COLLECTION, Type::KEYFRAME_COLLECTION, false, "DoubleKeyFrameCollection", "DOUBLEKEYFRAME_COLLECTION", 0, 1, NULL, (create_inst_func *) double_key_frame_collection_new, NULL }, 
	{ Type::DOWNLOADER, Type::DEPENDENCY_OBJECT, false, "Downloader", "DOWNLOADER", 3, 4, Downloader_Events, (create_inst_func *) downloader_new, NULL }, 
	{ Type::DRAWINGATTRIBUTES, Type::DEPENDENCY_OBJECT, false, "DrawingAttributes", "DRAWINGATTRIBUTES", 0, 1, NULL, (create_inst_func *) drawing_attributes_new, NULL }, 
	{ Type::DURATION, Type::INVALID, false, "Duration", "DURATION", 0, 0, NULL, NULL, NULL }, 
	{ Type::ELLIPSE, Type::SHAPE, false, "Ellipse", "ELLIPSE", 0, 12, NULL, (create_inst_func *) ellipse_new, NULL }, 
	{ Type::ELLIPSEGEOMETRY, Type::GEOMETRY, false, "EllipseGeometry", "ELLIPSEGEOMETRY", 0, 1, NULL, (create_inst_func *) ellipse_geometry_new, NULL }, 
	{ Type::ERROREVENTARGS, Type::EVENTARGS, false, "ErrorEventArgs", "ERROREVENTARGS", 0, 1, NULL, NULL, NULL }, 
	{ Type::EVENTARGS, Type::DEPENDENCY_OBJECT, false, "EventArgs", "EVENTARGS", 0, 1, NULL, NULL, NULL }, 
	{ Type::EVENTOBJECT, Type::INVALID, false, "EventObject", "EVENTOBJECT", 1, 1, EventObject_Events, NULL, NULL }, 
	{ Type::EVENTTRIGGER, Type::DEPENDENCY_OBJECT, false, "EventTrigger", "EVENTTRIGGER", 0, 1, NULL, (create_inst_func *) event_trigger_new, "Actions" }, 
	{ Type::FRAMEWORKELEMENT, Type::UIELEMENT, false, "FrameworkElement", "FRAMEWORKELEMENT", 0, 12, NULL, (create_inst_func *) framework_element_new, NULL }, 
	{ Type::GEOMETRY, Type::DEPENDENCY_OBJECT, false, "Geometry", "GEOMETRY", 0, 1, NULL, NULL, NULL }, 
	{ Type::GEOMETRY_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "GeometryCollection", "GEOMETRY_COLLECTION", 0, 1, NULL, (create_inst_func *) geometry_collection_new, NULL }, 
	{ Type::GEOMETRYGROUP, Type::GEOMETRY, false, "GeometryGroup", "GEOMETRYGROUP", 0, 1, NULL, (create_inst_func *) geometry_group_new, "Children" }, 
	{ Type::GLYPHS, Type::FRAMEWORKELEMENT, false, "Glyphs", "GLYPHS", 0, 12, NULL, (create_inst_func *) glyphs_new, NULL }, 
	{ Type::GRADIENTBRUSH, Type::BRUSH, false, "GradientBrush", "GRADIENTBRUSH", 0, 1, NULL, (create_inst_func *) gradient_brush_new, "GradientStops" }, 
	{ Type::GRADIENTSTOP, Type::DEPENDENCY_OBJECT, false, "GradientStop", "GRADIENTSTOP", 0, 1, NULL, (create_inst_func *) gradient_stop_new, NULL }, 
	{ Type::GRADIENTSTOP_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "GradientStopCollection", "GRADIENTSTOP_COLLECTION", 0, 1, NULL, (create_inst_func *) gradient_stop_collection_new, NULL }, 
#if SL_2_0
	{ Type::GRID, Type::PANEL, false, "Grid", "GRID", 0, 12, NULL, (create_inst_func *) grid_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'GRID'", "GRID", 0, 0, NULL, NULL, NULL }, 
#endif
	{ Type::GRIDLENGTH, Type::INVALID, false, "GridLength", "GRIDLENGTH", 0, 0, NULL, NULL, NULL }, 
	{ Type::IMAGE, Type::MEDIABASE, false, "Image", "IMAGE", 1, 14, Image_Events, (create_inst_func *) image_new, NULL }, 
	{ Type::IMAGEBRUSH, Type::TILEBRUSH, false, "ImageBrush", "IMAGEBRUSH", 2, 3, ImageBrush_Events, (create_inst_func *) image_brush_new, NULL }, 
	{ Type::IMAGEERROREVENTARGS, Type::ERROREVENTARGS, false, "ImageErrorEventArgs", "IMAGEERROREVENTARGS", 0, 1, NULL, NULL, NULL }, 
	{ Type::INKPRESENTER, Type::CANVAS, false, "InkPresenter", "INKPRESENTER", 0, 12, NULL, (create_inst_func *) ink_presenter_new, NULL }, 
	{ Type::INLINE, Type::DEPENDENCY_OBJECT, false, "Inline", "INLINE", 0, 1, NULL, (create_inst_func *) inline_new, NULL }, 
	{ Type::INLINES, Type::DEPENDENCY_OBJECT_COLLECTION, false, "Inlines", "INLINES", 0, 1, NULL, (create_inst_func *) inlines_new, NULL }, 
	{ Type::INT32, Type::INVALID, false, "gint32", "INT32", 0, 0, NULL, NULL, NULL }, 
	{ Type::INT64, Type::INVALID, false, "gint64", "INT64", 0, 0, NULL, NULL, NULL }, 
	{ Type::KEYBOARDEVENTARGS, Type::ROUTEDEVENTARGS, false, "KeyboardEventArgs", "KEYBOARDEVENTARGS", 0, 1, NULL, NULL, NULL }, 
	{ Type::KEYFRAME, Type::DEPENDENCY_OBJECT, false, "KeyFrame", "KEYFRAME", 0, 1, NULL, (create_inst_func *) key_frame_new, NULL }, 
	{ Type::KEYFRAME_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "KeyFrameCollection", "KEYFRAME_COLLECTION", 0, 1, NULL, NULL, NULL }, 
	{ Type::KEYSPLINE, Type::DEPENDENCY_OBJECT, false, "KeySpline", "KEYSPLINE", 0, 1, NULL, (create_inst_func *) key_spline_new, NULL }, 
	{ Type::KEYTIME, Type::INVALID, false, "KeyTime", "KEYTIME", 0, 0, NULL, NULL, NULL }, 
	{ Type::LINE, Type::SHAPE, false, "Line", "LINE", 0, 12, NULL, (create_inst_func *) line_new, NULL }, 
	{ Type::LINEARCOLORKEYFRAME, Type::COLORKEYFRAME, false, "LinearColorKeyFrame", "LINEARCOLORKEYFRAME", 0, 1, NULL, (create_inst_func *) linear_color_key_frame_new, NULL }, 
	{ Type::LINEARDOUBLEKEYFRAME, Type::DOUBLEKEYFRAME, false, "LinearDoubleKeyFrame", "LINEARDOUBLEKEYFRAME", 0, 1, NULL, (create_inst_func *) linear_double_key_frame_new, NULL }, 
	{ Type::LINEARGRADIENTBRUSH, Type::GRADIENTBRUSH, false, "LinearGradientBrush", "LINEARGRADIENTBRUSH", 0, 1, NULL, (create_inst_func *) linear_gradient_brush_new, NULL }, 
	{ Type::LINEARPOINTKEYFRAME, Type::POINTKEYFRAME, false, "LinearPointKeyFrame", "LINEARPOINTKEYFRAME", 0, 1, NULL, (create_inst_func *) linear_point_key_frame_new, NULL }, 
	{ Type::LINEBREAK, Type::INLINE, false, "LineBreak", "LINEBREAK", 0, 1, NULL, (create_inst_func *) line_break_new, NULL }, 
	{ Type::LINEGEOMETRY, Type::GEOMETRY, false, "LineGeometry", "LINEGEOMETRY", 0, 1, NULL, (create_inst_func *) line_geometry_new, NULL }, 
	{ Type::LINESEGMENT, Type::PATHSEGMENT, false, "LineSegment", "LINESEGMENT", 0, 1, NULL, (create_inst_func *) line_segment_new, NULL }, 
	{ Type::MANUALTIMESOURCE, Type::TIMESOURCE, false, "ManualTimeSource", "MANUALTIMESOURCE", 0, 2, NULL, NULL, NULL }, 
	{ Type::MARKERREACHEDEVENTARGS, Type::EVENTARGS, false, "MarkerReachedEventArgs", "MARKERREACHEDEVENTARGS", 0, 1, NULL, NULL, NULL }, 
	{ Type::MATRIX, Type::DEPENDENCY_OBJECT, false, "Matrix", "MATRIX", 0, 1, NULL, (create_inst_func *) matrix_new, NULL }, 
	{ Type::MATRIXTRANSFORM, Type::TRANSFORM, false, "MatrixTransform", "MATRIXTRANSFORM", 0, 1, NULL, (create_inst_func *) matrix_transform_new, NULL }, 
	{ Type::MEDIAATTRIBUTE, Type::DEPENDENCY_OBJECT, false, "MediaAttribute", "MEDIAATTRIBUTE", 0, 1, NULL, (create_inst_func *) media_attribute_new, NULL }, 
	{ Type::MEDIAATTRIBUTE_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "MediaAttributeCollection", "MEDIAATTRIBUTE_COLLECTION", 0, 1, NULL, (create_inst_func *) media_attribute_collection_new, NULL }, 
	{ Type::MEDIABASE, Type::FRAMEWORKELEMENT, false, "MediaBase", "MEDIABASE", 1, 13, MediaBase_Events, (create_inst_func *) media_base_new, NULL }, 
	{ Type::MEDIAELEMENT, Type::MEDIABASE, false, "MediaElement", "MEDIAELEMENT", 6, 19, MediaElement_Events, (create_inst_func *) media_element_new, NULL }, 
	{ Type::MEDIAERROREVENTARGS, Type::ERROREVENTARGS, false, "MediaErrorEventArgs", "MEDIAERROREVENTARGS", 0, 1, NULL, NULL, NULL }, 
	{ Type::MOUSEEVENTARGS, Type::ROUTEDEVENTARGS, false, "MouseEventArgs", "MOUSEEVENTARGS", 0, 1, NULL, (create_inst_func *) mouse_event_args_new, NULL }, 
	{ Type::NAMESCOPE, Type::DEPENDENCY_OBJECT, false, "NameScope", "NAMESCOPE", 0, 1, NULL, NULL, NULL }, 
	{ Type::NPOBJ, Type::INVALID, false, "NPObj", "NPOBJ", 0, 0, NULL, NULL, NULL }, 
	{ Type::PANEL, Type::FRAMEWORKELEMENT, false, "Panel", "PANEL", 0, 12, NULL, (create_inst_func *) panel_new, "Children" }, 
	{ Type::PARALLELTIMELINE, Type::TIMELINEGROUP, false, "ParallelTimeline", "PARALLELTIMELINE", 0, 1, NULL, (create_inst_func *) parallel_timeline_new, NULL }, 
	{ Type::PARSERERROREVENTARGS, Type::ERROREVENTARGS, false, "ParserErrorEventArgs", "PARSERERROREVENTARGS", 0, 1, NULL, NULL, NULL }, 
	{ Type::PATH, Type::SHAPE, false, "Path", "PATH", 0, 12, NULL, (create_inst_func *) path_new, NULL }, 
	{ Type::PATHFIGURE, Type::DEPENDENCY_OBJECT, false, "PathFigure", "PATHFIGURE", 0, 1, NULL, (create_inst_func *) path_figure_new, "Segments" }, 
	{ Type::PATHFIGURE_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "PathFigureCollection", "PATHFIGURE_COLLECTION", 0, 1, NULL, (create_inst_func *) path_figure_collection_new, NULL }, 
	{ Type::PATHGEOMETRY, Type::GEOMETRY, false, "PathGeometry", "PATHGEOMETRY", 0, 1, NULL, (create_inst_func *) path_geometry_new, "Figures" }, 
	{ Type::PATHSEGMENT, Type::DEPENDENCY_OBJECT, false, "PathSegment", "PATHSEGMENT", 0, 1, NULL, NULL, NULL }, 
	{ Type::PATHSEGMENT_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "PathSegmentCollection", "PATHSEGMENT_COLLECTION", 0, 1, NULL, (create_inst_func *) path_segment_collection_new, NULL }, 
	{ Type::POINT, Type::INVALID, false, "Point", "POINT", 0, 0, NULL, NULL, NULL }, 
	{ Type::POINT_ARRAY, Type::INVALID, false, "Point*", "POINT_ARRAY", 0, 0, NULL, NULL, NULL }, 
	{ Type::POINT_COLLECTION, Type::COLLECTION, false, "PointCollection", "POINT_COLLECTION", 0, 1, NULL, (create_inst_func *) point_collection_new, NULL }, 
	{ Type::POINTANIMATION, Type::ANIMATION, false, "PointAnimation", "POINTANIMATION", 0, 1, NULL, (create_inst_func *) point_animation_new, NULL }, 
	{ Type::POINTANIMATIONUSINGKEYFRAMES, Type::POINTANIMATION, false, "PointAnimationUsingKeyFrames", "POINTANIMATIONUSINGKEYFRAMES", 0, 1, NULL, (create_inst_func *) point_animation_using_key_frames_new, "KeyFrames" }, 
	{ Type::POINTKEYFRAME, Type::KEYFRAME, false, "PointKeyFrame", "POINTKEYFRAME", 0, 1, NULL, (create_inst_func *) point_key_frame_new, NULL }, 
	{ Type::POINTKEYFRAME_COLLECTION, Type::KEYFRAME_COLLECTION, false, "PointKeyFrameCollection", "POINTKEYFRAME_COLLECTION", 0, 1, NULL, (create_inst_func *) point_key_frame_collection_new, NULL }, 
	{ Type::POLYBEZIERSEGMENT, Type::PATHSEGMENT, false, "PolyBezierSegment", "POLYBEZIERSEGMENT", 0, 1, NULL, (create_inst_func *) poly_bezier_segment_new, NULL }, 
	{ Type::POLYGON, Type::SHAPE, false, "Polygon", "POLYGON", 0, 12, NULL, (create_inst_func *) polygon_new, NULL }, 
	{ Type::POLYLINE, Type::SHAPE, false, "Polyline", "POLYLINE", 0, 12, NULL, (create_inst_func *) polyline_new, NULL }, 
	{ Type::POLYLINESEGMENT, Type::PATHSEGMENT, false, "PolyLineSegment", "POLYLINESEGMENT", 0, 1, NULL, (create_inst_func *) poly_line_segment_new, NULL }, 
	{ Type::POLYQUADRATICBEZIERSEGMENT, Type::PATHSEGMENT, false, "PolyQuadraticBezierSegment", "POLYQUADRATICBEZIERSEGMENT", 0, 1, NULL, (create_inst_func *) poly_quadratic_bezier_segment_new, NULL }, 
	{ Type::QUADRATICBEZIERSEGMENT, Type::PATHSEGMENT, false, "QuadraticBezierSegment", "QUADRATICBEZIERSEGMENT", 0, 1, NULL, (create_inst_func *) quadratic_bezier_segment_new, NULL }, 
	{ Type::RADIALGRADIENTBRUSH, Type::GRADIENTBRUSH, false, "RadialGradientBrush", "RADIALGRADIENTBRUSH", 0, 1, NULL, (create_inst_func *) radial_gradient_brush_new, NULL }, 
	{ Type::RECT, Type::INVALID, false, "Rect", "RECT", 0, 0, NULL, NULL, NULL }, 
	{ Type::RECTANGLE, Type::SHAPE, false, "Rectangle", "RECTANGLE", 0, 12, NULL, (create_inst_func *) rectangle_new, NULL }, 
	{ Type::RECTANGLEGEOMETRY, Type::GEOMETRY, false, "RectangleGeometry", "RECTANGLEGEOMETRY", 0, 1, NULL, (create_inst_func *) rectangle_geometry_new, NULL }, 
	{ Type::REPEATBEHAVIOR, Type::INVALID, false, "RepeatBehavior", "REPEATBEHAVIOR", 0, 0, NULL, NULL, NULL }, 
	{ Type::RESOURCE_DICTIONARY, Type::DEPENDENCY_OBJECT_COLLECTION, false, "ResourceDictionary", "RESOURCE_DICTIONARY", 0, 1, NULL, (create_inst_func *) resource_dictionary_new, NULL }, 
	{ Type::ROTATETRANSFORM, Type::TRANSFORM, false, "RotateTransform", "ROTATETRANSFORM", 0, 1, NULL, (create_inst_func *) rotate_transform_new, NULL }, 
	{ Type::ROUTEDEVENTARGS, Type::EVENTARGS, false, "RoutedEventArgs", "ROUTEDEVENTARGS", 0, 1, NULL, (create_inst_func *) routed_event_args_new, NULL }, 
#if SL_2_0
	{ Type::ROWDEFINITION, Type::DEPENDENCY_OBJECT, false, "RowDefinition", "ROWDEFINITION", 0, 1, NULL, (create_inst_func *) row_definition_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'ROWDEFINITION'", "ROWDEFINITION", 0, 0, NULL, NULL, NULL }, 
#endif
#if SL_2_0
	{ Type::ROWDEFINITION_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "RowDefinitionCollection", "ROWDEFINITION_COLLECTION", 0, 1, NULL, (create_inst_func *) row_definition_collection_new, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'ROWDEFINITION_COLLECTION'", "ROWDEFINITION_COLLECTION", 0, 0, NULL, NULL, NULL }, 
#endif
	{ Type::RUN, Type::INLINE, false, "Run", "RUN", 0, 1, NULL, (create_inst_func *) run_new, "Text" }, 
	{ Type::SCALETRANSFORM, Type::TRANSFORM, false, "ScaleTransform", "SCALETRANSFORM", 0, 1, NULL, (create_inst_func *) scale_transform_new, NULL }, 
	{ Type::SHAPE, Type::FRAMEWORKELEMENT, false, "Shape", "SHAPE", 0, 12, NULL, NULL, NULL }, 
#if SL_2_0
	{ Type::SIZE, Type::INVALID, false, "Size", "SIZE", 0, 0, NULL, NULL, NULL }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'SIZE'", "SIZE", 0, 0, NULL, NULL, NULL }, 
#endif
	{ Type::SKEWTRANSFORM, Type::TRANSFORM, false, "SkewTransform", "SKEWTRANSFORM", 0, 1, NULL, (create_inst_func *) skew_transform_new, NULL }, 
	{ Type::SOLIDCOLORBRUSH, Type::BRUSH, false, "SolidColorBrush", "SOLIDCOLORBRUSH", 0, 1, NULL, (create_inst_func *) solid_color_brush_new, NULL }, 
	{ Type::SPLINECOLORKEYFRAME, Type::COLORKEYFRAME, false, "SplineColorKeyFrame", "SPLINECOLORKEYFRAME", 0, 1, NULL, (create_inst_func *) spline_color_key_frame_new, NULL }, 
	{ Type::SPLINEDOUBLEKEYFRAME, Type::DOUBLEKEYFRAME, false, "SplineDoubleKeyFrame", "SPLINEDOUBLEKEYFRAME", 0, 1, NULL, (create_inst_func *) spline_double_key_frame_new, NULL }, 
	{ Type::SPLINEPOINTKEYFRAME, Type::POINTKEYFRAME, false, "SplinePointKeyFrame", "SPLINEPOINTKEYFRAME", 0, 1, NULL, (create_inst_func *) spline_point_key_frame_new, NULL }, 
	{ Type::STORYBOARD, Type::PARALLELTIMELINE, false, "Storyboard", "STORYBOARD", 1, 2, Storyboard_Events, (create_inst_func *) storyboard_new, "Children" }, 
	{ Type::STRING, Type::INVALID, false, "char*", "STRING", 0, 0, NULL, NULL, NULL }, 
	{ Type::STROKE, Type::DEPENDENCY_OBJECT, false, "Stroke", "STROKE", 0, 1, NULL, (create_inst_func *) stroke_new, NULL }, 
	{ Type::STROKE_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "StrokeCollection", "STROKE_COLLECTION", 0, 1, NULL, (create_inst_func *) stroke_collection_new, NULL }, 
	{ Type::STYLUSINFO, Type::DEPENDENCY_OBJECT, false, "StylusInfo", "STYLUSINFO", 0, 1, NULL, (create_inst_func *) stylus_info_new, NULL }, 
	{ Type::STYLUSPOINT, Type::DEPENDENCY_OBJECT, false, "StylusPoint", "STYLUSPOINT", 0, 1, NULL, (create_inst_func *) stylus_point_new, NULL }, 
	{ Type::STYLUSPOINT_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "StylusPointCollection", "STYLUSPOINT_COLLECTION", 0, 1, NULL, (create_inst_func *) stylus_point_collection_new, NULL }, 
	{ Type::SURFACE, Type::EVENTOBJECT, false, "Surface", "SURFACE", 4, 5, Surface_Events, (create_inst_func *) surface_new, NULL }, 
	{ Type::SYSTEMTIMESOURCE, Type::TIMESOURCE, false, "SystemTimeSource", "SYSTEMTIMESOURCE", 0, 2, NULL, NULL, NULL }, 
	{ Type::TEXTBLOCK, Type::FRAMEWORKELEMENT, false, "TextBlock", "TEXTBLOCK", 0, 12, NULL, (create_inst_func *) text_block_new, "Inlines" }, 
	{ Type::THICKNESS, Type::INVALID, false, "Thickness", "THICKNESS", 0, 0, NULL, NULL, NULL }, 
	{ Type::TILEBRUSH, Type::BRUSH, false, "TileBrush", "TILEBRUSH", 0, 1, NULL, NULL, NULL }, 
	{ Type::TIMELINE, Type::DEPENDENCY_OBJECT, false, "Timeline", "TIMELINE", 0, 1, NULL, NULL, NULL }, 
	{ Type::TIMELINE_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "TimelineCollection", "TIMELINE_COLLECTION", 0, 1, NULL, (create_inst_func *) timeline_collection_new, NULL }, 
	{ Type::TIMELINEGROUP, Type::TIMELINE, false, "TimelineGroup", "TIMELINEGROUP", 0, 1, NULL, (create_inst_func *) timeline_group_new, NULL }, 
	{ Type::TIMELINEMARKER, Type::DEPENDENCY_OBJECT, false, "TimelineMarker", "TIMELINEMARKER", 0, 1, NULL, (create_inst_func *) timeline_marker_new, NULL }, 
	{ Type::TIMELINEMARKER_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "TimelineMarkerCollection", "TIMELINEMARKER_COLLECTION", 0, 1, NULL, (create_inst_func *) timeline_marker_collection_new, NULL }, 
	{ Type::TIMEMANAGER, Type::EVENTOBJECT, false, "TimeManager", "TIMEMANAGER", 2, 3, TimeManager_Events, NULL, NULL }, 
	{ Type::TIMESOURCE, Type::EVENTOBJECT, false, "TimeSource", "TIMESOURCE", 1, 2, TimeSource_Events, NULL, NULL }, 
	{ Type::TIMESPAN, Type::INVALID, false, "TimeSpan", "TIMESPAN", 0, 0, NULL, NULL, NULL }, 
	{ Type::TRANSFORM, Type::DEPENDENCY_OBJECT, false, "Transform", "TRANSFORM", 0, 1, NULL, (create_inst_func *) transform_new, NULL }, 
	{ Type::TRANSFORM_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "TransformCollection", "TRANSFORM_COLLECTION", 0, 1, NULL, (create_inst_func *) transform_collection_new, NULL }, 
	{ Type::TRANSFORMGROUP, Type::TRANSFORM, false, "TransformGroup", "TRANSFORMGROUP", 0, 1, NULL, (create_inst_func *) transform_group_new, "Children" }, 
	{ Type::TRANSLATETRANSFORM, Type::TRANSFORM, false, "TranslateTransform", "TRANSLATETRANSFORM", 0, 1, NULL, (create_inst_func *) translate_transform_new, NULL }, 
	{ Type::TRIGGER_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "TriggerCollection", "TRIGGER_COLLECTION", 0, 1, NULL, (create_inst_func *) trigger_collection_new, NULL }, 
	{ Type::TRIGGERACTION, Type::DEPENDENCY_OBJECT, false, "TriggerAction", "TRIGGERACTION", 0, 1, NULL, NULL, NULL }, 
	{ Type::TRIGGERACTION_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "TriggerActionCollection", "TRIGGERACTION_COLLECTION", 0, 1, NULL, (create_inst_func *) trigger_action_collection_new, NULL }, 
	{ Type::UIELEMENT, Type::VISUAL, false, "UIElement", "UIELEMENT", 11, 12, UIElement_Events, NULL, NULL }, 
	{ Type::UINT32, Type::INVALID, false, "guint32", "UINT32", 0, 0, NULL, NULL, NULL }, 
	{ Type::UINT64, Type::INVALID, false, "guint64", "UINT64", 0, 0, NULL, NULL, NULL }, 
#if SL_2_0
	{ Type::USERCONTROL, Type::CONTROL, false, "UserControl", "USERCONTROL", 0, 12, NULL, (create_inst_func *) user_control_new, "Content" }, 
#else
	{ Type::INVALID, Type::INVALID, false, "2.0 specific type 'USERCONTROL'", "USERCONTROL", 0, 0, NULL, NULL, NULL }, 
#endif
	{ Type::VIDEOBRUSH, Type::TILEBRUSH, false, "VideoBrush", "VIDEOBRUSH", 0, 1, NULL, (create_inst_func *) video_brush_new, NULL }, 
	{ Type::VISUAL, Type::DEPENDENCY_OBJECT, false, "Visual", "VISUAL", 0, 1, NULL, NULL, NULL }, 
	{ Type::VISUAL_COLLECTION, Type::DEPENDENCY_OBJECT_COLLECTION, false, "VisualCollection", "VISUAL_COLLECTION", 0, 1, NULL, (create_inst_func *) visual_collection_new, NULL }, 
	{ Type::VISUALBRUSH, Type::TILEBRUSH, false, "VisualBrush", "VISUALBRUSH", 0, 1, NULL, (create_inst_func *) visual_brush_new, NULL }, 
	{ Type::LASTTYPE, Type::INVALID, false, NULL, NULL, 0, 0, NULL, NULL, NULL }
};
