#ifndef MOON_VALUE_H
#define MOON_VALUE_H

/* ugly file containing the catchall value type we use.
   implementation still lives in runtime.cpp
*/

struct Color;
struct Duration;
struct KeyTime;
struct Point;
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
class DoubleAnimation;
class DoubleArray;
class Ellipse;
class EllipseGeometry;
class EventTrigger;
class FrameworkElement;
class Geometry;
class GeometryGroup;
class KeyFrame;
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
class VisualCollection;

struct Value {
public:
	// Keep these values in sync with the Value.cs in olive.
	// Also keep in sync within types_init.
	enum Kind {
		INVALID = 0,
		BOOL = 1,
		DOUBLE = 2,
		UINT64 = 3,
		INT32 = 4,
		STRING = 5,
		COLOR = 7,
		POINT = 8,
		RECT = 9,
		REPEATBEHAVIOR = 10,
		DURATION = 11,
		INT64 = 12,
		DOUBLE_ARRAY = 13,
		POINT_ARRAY = 14,
		KEYTIME = 15,

		DEPENDENCY_OBJECT,

		// These are dependency objects
		UIELEMENT,
		PANEL,
		CANVAS,
		TIMELINE,
		TIMELINEGROUP,
		PARALLELTIMELINE,
		TRANSFORM,
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
		POINTKEYFRAME,
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
	};

	Kind k;
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
	Color*          AsColor ();
	Point*          AsPoint ();
	Rect*           AsRect  ();
	RepeatBehavior* AsRepeatBehavior ();
	Duration*       AsDuration ();
	KeyTime*        AsKeyTime ();

	DependencyObject*             AsDependencyObject ();
	UIElement*                    AsUIElement ();
	Panel*                        AsPanel ();
	Canvas*                       AsCanvas ();
	Timeline*                     AsTimeline ();
	TimelineGroup*                AsTimelineGroup ();
	ParallelTimeline*             AsParallelTimeline ();
	Transform*                    AsTransform ();
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
	PointKeyFrame*                AsPointKeyFrame ();
	PointAnimationUsingKeyFrames* AsPointAnimationUsingKeyFrames ();

	Collection*                   AsCollection ();
	VisualCollection*             AsVisualCollection ();

  
  private:
	// You don't want to be using this ctor.  it's here to help
	// c++ recognize bad unspecified pointer args to Value ctors
	// (it normally converts them to bool, which we handle, so you
	// never see the error of your ways).  So do the world a
	// favor, and don't expose this ctor. :)
	Value (void* v) { }
};


#endif /* MOON_VALUE_H */
