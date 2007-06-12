#ifndef MOON_VALUE_H
#define MOON_VALUE_H

/* ugly file containing the catchall value type we use.
   implementation still lives in runtime.cpp
*/

struct Color;
struct DoubleArray;
struct Duration;
struct KeyTime;
struct Point;
struct PointArray;
struct Rect;
struct RepeatBehavior;

class Animation;
class AnimationClock;
class ArcSegment;
class BeginStoryboard;
class BezierSegment;
class Brush;
class Canvas;
class Clock;
class ClockGroup;
class ColorAnimation;
class DependencyObject;
class DependencyProperty;
class DiscreteDoubleKeyFrame;
class DiscretePointKeyFrame;
class DoubleAnimation;
class DoubleAnimationUsingKeyFrames;
class DoubleArray;
class DoubleKeyFrame;
class Ellipse;
class EllipseGeometry;
class EventTrigger;
class FrameworkElement;
class Geometry;
class GeometryGroup;
class KeyFrame;
class LinearDoubleKeyFrame;
class LinearPointKeyFrame;
class Line;
class LineGeometry;
class LineSegment;
class MatrixTransform;
class NameScope;
class Panel;
class ParallelTimeline;
class Path;
class PathFigure;
class PathGeometry;
class PathSegment;
class PointAnimation;
class PointAnimationUsingKeyFrames;
class PointArray;
class PointKeyFrame;
class PolyBezierSegment;
class Polygon;
class Polyline;
class PolyLineSegment;
class PolyQuadraticBezierSegment;
class QuadraticBezierSegment;
class Rectangle;
class RectangleGeometry;
class RotateTransform;
class ScaleTransform;
class Shape;
class SolidColorBrush;
class TileBrush;
class ImageBrush;
class VideoBrush;
class LinearGradientBrush;
class GradientBrush;
class GradientStop;
class Storyboard;
class Surface;
class Timeline;
class TimelineGroup;
class Transform;
class TransformGroup;
class TranslateTransform;
class TriggerAction;
class UIElement;

class Collection;
class KeyFrameCollection;
class TimelineCollection;
class VisualCollection;
class TriggerCollection;
class TriggerActionCollection;
class TransformCollection;
class GeometryCollection;
class PathFigureCollection;
class PathSegmentCollection;

struct Value {
public:
	// Keep these values in sync with the Value.cs in olive.
	// Also keep in sync within types_init.
	enum Kind {
// START_MANAGED_MAPPING
		INVALID,
		BOOL,
		DOUBLE,
		UINT64,
		INT32,
		STRING,
		COLOR,
		POINT,
		RECT,
		REPEATBEHAVIOR,
		DURATION,
		INT64,
		DOUBLE_ARRAY,
		POINT_ARRAY,
		KEYTIME,

		DEPENDENCY_OBJECT,

		// These are dependency objects
		UIELEMENT,
		PANEL,
		CANVAS,
		TIMELINE,
		TIMELINEGROUP,
		PARALLELTIMELINE,
		TRANSFORM,
		TRANSFORMGROUP,
		ROTATETRANSFORM,
		SCALETRANSFORM,
		TRANSLATETRANSFORM,
		MATRIXTRANSFORM,
		STORYBOARD,
		ANIMATION,
		DOUBLEANIMATION,
		COLORANIMATION,
		POINTANIMATION,
		SHAPE,
		ELLIPSE,
		LINE,
		PATH,
		POLYGON,
		POLYLINE,
		RECTANGLE,
		GEOMETRY,
		GEOMETRYGROUP,
		ELLIPSEGEOMETRY,
		LINEGEOMETRY,
		PATHGEOMETRY,
		RECTANGLEGEOMETRY,
		FRAMEWORKELEMENT,
		NAMESCOPE,
		CLOCK,
		ANIMATIONCLOCK,
		CLOCKGROUP,
		BRUSH,
		SOLIDCOLORBRUSH,
		TILEBRUSH,
		IMAGEBRUSH,
		VIDEOBRUSH,
		LINEARGRADIENTBRUSH,
		GRADIENTBRUSH,
		GRADIENTSTOP,
		PATHFIGURE,
		PATHSEGMENT,
		ARCSEGMENT,
		BEZIERSEGMENT,
		LINESEGMENT,
		POLYBEZIERSEGMENT,
		POLYLINESEGMENT,
		POLYQUADRATICBEZIERSEGMENT,
		QUADRATICBEZIERSEGMENT,
		TRIGGERACTION,
		BEGINSTORYBOARD,
		EVENTTRIGGER,
		KEYFRAME,
		COLORKEYFRAME,
		DOUBLEKEYFRAME,
		POINTKEYFRAME,
		DISCRETECOLORKEYFRAME,
		DISCRETEDOUBLEKEYFRAME,
		DISCRETEPOINTKEYFRAME,
		LINEARCOLORKEYFRAME,
		LINEARDOUBLEKEYFRAME,
		LINEARPOINTKEYFRAME,
		COLORANIMATIONUSINGKEYFRAMES,
		DOUBLEANIMATIONUSINGKEYFRAMES,
		POINTANIMATIONUSINGKEYFRAMES,

		// The collections
		COLLECTION,
		STROKE_COLLECTION,
		INLINES,
		STYLUSPOINT_COLLECTION,
		KEYFRAME_COLLECTION,
		TIMELINEMARKER_COLLECTION,
		GEOMETRY_COLLECTION,
		GRADIENTSTOP_COLLECTION,
		MEDIAATTRIBUTE_COLLECTION,
		PATHFIGURE_COLLECTION,
		PATHSEGMENT_COLLECTION,
		TIMELINE_COLLECTION,
		TRANSFORM_COLLECTION,
		VISUAL_COLLECTION,
		RESOURCE_COLLECTION,
		TRIGGERACTION_COLLECTION,
		TRIGGER_COLLECTION,

		LASTTYPE
// END_MANAGED_MAPPING
	};

	void Init ();

	Value ();
	Value (const Value& v);
	Value (Kind k);
	Value (bool z);
	Value (double d);
	Value (guint64 i);
	Value (gint64 i);
	Value (gint32 i);
	Value (Color c);
	Value (DependencyObject *obj);
	Value (Point pt);
	Value (Rect rect);
	Value (RepeatBehavior repeat);
	Value (Duration duration);
	Value (KeyTime keytime);
	Value (const char* s);
	Value (Point *points, int count);
	Value (double *values, int count);
	
	~Value ();

	bool operator!= (const Value &v) const
	{
		return !(*this == v);
	}

	bool operator== (const Value &v) const
	{
		if (k != v.k)
			return false;

		if (k == STRING) {
			return !strcmp (u.s, v.u.s);
		}
		else {
			return !memcmp (&u, &v.u, sizeof (u));
		}

		return true;
	}

	bool            AsBool ();
	double          AsDouble ();
	guint64         AsUint64 ();
	gint64          AsInt64 ();
	gint32          AsInt32 ();

	double*         AsNullableDouble ();
	guint64*        AsNullableUint64 ();
	gint64*         AsNullableInt64 ();
	gint32*         AsNullableInt32 ();

	Color*          AsColor ();
	Point*          AsPoint ();
	Rect*           AsRect  ();
	char*           AsString ();
	RepeatBehavior* AsRepeatBehavior ();
	Duration*       AsDuration ();
	KeyTime*        AsKeyTime ();
	PointArray*     AsPointArray ();
	DoubleArray*     AsDoubleArray ();

	DependencyObject*             AsDependencyObject ();
	UIElement*                    AsUIElement ();
	Panel*                        AsPanel ();
	Canvas*                       AsCanvas ();
	Timeline*                     AsTimeline ();
	TimelineGroup*                AsTimelineGroup ();
	ParallelTimeline*             AsParallelTimeline ();
	Transform*                    AsTransform ();
	TransformGroup*               AsTransformGroup ();
	RotateTransform*              AsRotateTransform ();
	ScaleTransform*               AsScaleTransform ();
	TranslateTransform*           AsTranslateTransform ();
	MatrixTransform*              AsMatrixTransform ();
	Storyboard*                   AsStoryboard ();
	Animation*                    AsAnimation ();
	DoubleAnimation*              AsDoubleAnimation ();
	ColorAnimation*               AsColorAnimation ();
	PointAnimation*               AsPointAnimation ();
	Shape*                        AsShape ();
	Ellipse*                      AsEllipse ();
	Line*                         AsLine ();
	Path*                         AsPath ();
	Polygon*                      AsPolygon ();
	Polyline*                     AsPolyline ();
	Rectangle*                    AsRectangle ();
	Geometry*                     AsGeometry ();
	GeometryGroup*                AsGeometryGroup ();
	EllipseGeometry*              AsEllipseGeometry ();
	LineGeometry*                 AsLineGeometry ();
	PathGeometry*                 AsPathGeometry ();
	RectangleGeometry*            AsRectangleGeometry();
	FrameworkElement*             AsFrameworkElement ();
	NameScope*                    AsNameScope ();
	Clock*                        AsClock ();
	AnimationClock*               AsAnimationClock ();
	ClockGroup*                   AsClockGroup ();
	Brush*                        AsBrush ();
	SolidColorBrush*              AsSolidColorBrush ();
	TileBrush*                    AsTileBrush ();
	ImageBrush*                   AsImageBrush ();
	VideoBrush*                   AsVideoBrush ();
	LinearGradientBrush*          AsLinearGradientBrush ();
	GradientBrush*                AsGradientBrush ();
	GradientStop*                 AsGradientStop ();
	PathFigure*                   AsPathFigure ();
	PathSegment*                  AsPathSegment ();
	ArcSegment*                   AsArcSegment ();
	BezierSegment*                AsBezierSegment ();
	LineSegment*                  AsLineSegment ();
	PolyBezierSegment*            AsPolyBezierSegment ();
	PolyLineSegment*              AsPolyLineSegment ();
	PolyQuadraticBezierSegment*   AsPolyQuadraticBezierSegment ();
	QuadraticBezierSegment*       AsQuadraticBezierSegment ();
	TriggerAction*                AsTriggerAction ();
	BeginStoryboard*              AsBeginStoryboard ();
	EventTrigger*                 AsEventTrigger ();
	KeyFrame*                     AsKeyFrame ();
	DoubleKeyFrame*               AsDoubleKeyFrame();
	PointKeyFrame*                AsPointKeyFrame ();
	DiscreteDoubleKeyFrame*       AsDiscreteDoubleKeyFrame ();
	DiscretePointKeyFrame*        AsDiscretePointKeyFrame ();
	LinearDoubleKeyFrame*         AsLinearDoubleKeyFrame ();
	LinearPointKeyFrame*          AsLinearPointKeyFrame ();
	DoubleAnimationUsingKeyFrames* AsDoubleAnimationUsingKeyFrames ();
	PointAnimationUsingKeyFrames* AsPointAnimationUsingKeyFrames ();

	Collection*                   AsCollection ();
	KeyFrameCollection*           AsKeyFrameCollection ();
	TimelineCollection*           AsTimelineCollection ();
	VisualCollection*             AsVisualCollection ();
	TriggerCollection*            AsTriggerCollection ();
	TriggerActionCollection*      AsTriggerActionCollection ();
	TransformCollection*          AsTransformCollection ();
	GeometryCollection*           AsGeometryCollection ();
	PathFigureCollection*         AsPathFigureCollection ();
	PathSegmentCollection*        AsPathSegmentCollection ();
  
	Kind k;
  private:
	union {
		double d;
		guint64 ui64;
		gint64 i64;
		gint32 i32;
		char *s;
		DependencyObject *dependency_object;
		Color *color;
		Point *point;
		Rect *rect;
		RepeatBehavior *repeat;
		Duration *duration;
		KeyTime *keytime;
		PointArray *point_array;
		DoubleArray *double_array;
	} u;


	// You don't want to be using this ctor.  it's here to help
	// c++ recognize bad unspecified pointer args to Value ctors
	// (it normally converts them to bool, which we handle, so you
	// never see the error of your ways).  So do the world a
	// favor, and don't expose this ctor. :)
	Value (void* v) { }
};


#endif /* MOON_VALUE_H */
