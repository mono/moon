#ifndef __RUNTIME_H__
#define __RUNTIME_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

typedef void (*EventHandler) (gpointer data);

class EventObject {
 public:
	EventObject ();
	~EventObject ();
	
	void AddHandler (char *event_name, EventHandler handler, gpointer data);
	void RemoveHandler (char *event_name, EventHandler handler, gpointer data);
	
	void Emit (char *event_name);
 private:
	GHashTable *event_hash;
};

struct Duration;
struct RepeatBehavior;
class Transform;
class TransformGroup;
class Surface;
class Brush;
class DependencyObject;
class DependencyProperty;

struct Point {
public:
	double x, y;

	Point () : x(0), y(0) {}

	Point (double x, double y)
	{
		this->x = x;
		this->y = y;
	}

	Point (const Point &point)
	{
		x = point.x;
		y = point.y;
	}
};

Point point_from_str (const char *s);

struct PointArray {
 public:
	Point *points;
	int count;

	PointArray () : points (NULL), count (0) {}

	PointArray (Point* points, int count)
	{
		this->points = new Point[count];
		memcpy (this->points, points, sizeof (Point) * count);
		this->count = count;
	}

	~PointArray ()
	{
		delete points;
	}
};

Point* point_array_from_str (const char *s, int* count);

// map to System.Windows.Rect
struct Rect {
 public:
	double x, y, w, h;

	Rect () : x (0), y (0), w (0), h (0) {}
	Rect (double x, double y, double width, double height)
	{
		this->x = x;
		this->y = y;
		w = width;
		h = height;
	}

	Rect (const Rect &rect)
	{
		x = rect.x;
		y = rect.y;
		w = rect.w;
		h = rect.h;
	}
};

Rect rect_from_str (const char *s);

struct Color {
	double r, g, b, a;
 public:
	Color () : a(0.0), r(0.0), g(0.0), b(0.0) {}

	Color (unsigned int argb)
	{
		a = (argb >> 24) / 255.0f;
		r = ((argb >> 16) & 0xFF) / 255.0f;
		g = ((argb >> 8) & 0xFF) / 255.0f;
		b = (argb & 0xFF) / 255.0f;
	}
	
	Color (double r, double g, double b, double a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	Color (const Color &color)
	{
		r = color.r;
		g = color.g;
		b = color.b;
		a = color.a;
	}
};


Color *color_from_str (const char *name);

struct DoubleArray {
 public:
	double *values;
	int count;

	DoubleArray () : values (NULL), count (0) {}

	DoubleArray (double* values, int count)
	{
		this->values = new double[count];
		memcpy (this->values, values, sizeof (double) * count);
		this->count = count;
	}

	~DoubleArray ()
	{
		delete values;
	}
};

double* double_array_from_str (const char *s, int* count);

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
  private:
	// You don't want to be using this ctor.  it's here to help
	// c++ recognize bad unspecified pointer args to Value ctors
	// (it normally converts them to bool, which we handle, so you
	// never see the error of your ways).  So do the world a
	// favor, and don't expose this ctor. :)
	Value (void* v) { }
};

class Type {
public:
	static Type* RegisterType (char *name, Value::Kind type, Value::Kind parent);
	static Type* RegisterType (char *name, Value::Kind type);
	static Type* Find (char *name);
	static Type* Find (Value::Kind type);
	
	bool IsSubclassOf (Value::Kind super);	
	
	Value::Kind parent;
	Value::Kind type;
	char *name;

private:
	Type () {};
	public: static Type* types [Value::LASTTYPE];
	static GHashTable *types_by_name;
};

//
// This guy provide reference counting
//
#define BASE_FLOATS 0x80000000

class Base {
 public:	
	uint32_t refcount;
	Base () : refcount(BASE_FLOATS) {}
};

void base_ref   (Base *base);
void base_unref (Base *base);

//
// DependencyObject
// 

class DependencyObject : public Base {
 public:

	DependencyObject ();
	~DependencyObject ();
	static DependencyProperty *Register (Value::Kind type, char *name, Value *default_value);
	static DependencyProperty *Register (Value::Kind type, char *name, Value::Kind vtype);
	static DependencyProperty *RegisterFull (Value::Kind type, char *name, Value *default_value, Value::Kind vtype);
	
	static DependencyProperty *GetDependencyProperty (Value::Kind type, char *name);
	void SetValue (DependencyProperty *property, Value value);
	void SetValue (DependencyProperty *property, Value *value);
	Value *GetValue (DependencyProperty *property);
	DependencyProperty *GetDependencyProperty (char *name);
	DependencyObject* FindName (char *name);

	EventObject *events;
	static GHashTable *properties;

	virtual void OnPropertyChanged (DependencyProperty *property) {}
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop) { }
	virtual Value::Kind GetObjectType ()
	{
		g_warning ("This class is missing an override of GetObjectType ()");
		return Value::DEPENDENCY_OBJECT; 
	};


 protected:
	void NotifyAttacheesOfPropertyChange (DependencyProperty *property);

 private:
	GHashTable        *current_values;
	GSList            *attached_list;
};

Value *dependency_object_get_value (DependencyObject *object, DependencyProperty *prop);
void   dependency_object_set_value (DependencyObject *object, DependencyProperty *prop, Value val);

//
// DependencyProperty
//
class DependencyProperty {
 public:
	DependencyProperty () {} ;
	~DependencyProperty ();
	DependencyProperty (Value::Kind type, char *name, Value *default_value, Value::Kind value_type);

	char *name;
	Value *default_value;
	Value::Kind type;
	Value::Kind value_type;
};

DependencyProperty *dependency_property_lookup (Value::Kind type, char *name);

class NameScope : public DependencyObject {
 public:
	NameScope ();
	~NameScope ();

	void RegisterName (const char *name, DependencyObject *object);
	void UnregisterName (const char *name);

	DependencyObject* FindName (const char *name);

	static NameScope* GetNameScope (DependencyObject *obj);
	static void SetNameScope (DependencyObject *obj, NameScope *scope);

	static DependencyProperty *NameScopeProperty;
	
 private:
	GHashTable *names;
};


//
// Collection: provides a collection that we can monitor for
// changes.   We expose this collection in a few classes to
// the managed world, and when a change happens we get a
// chance to reflect the changes
//
class Collection;

class Collection : public DependencyObject {
 public:
	GSList *list;
	void *closure;

	Collection () : list(NULL), closure(NULL) {}
	Value::Kind GetObjectType () { return Value::COLLECTION; };	

	virtual void Add    (void *data);
	virtual void Remove (void *data);
};

void collection_add    (Collection *collection, void *data);
void collection_remove (Collection *collection, void *data);

class VisualCollection : public Collection {
 public:
	VisualCollection () {}
	virtual Value::Kind GetObjectType () { return Value::VISUAL_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
};

class Brush : public DependencyObject {
 public:
	static DependencyProperty* OpacityProperty;
	static DependencyProperty* RelativeTransformProperty;
	static DependencyProperty* TransformProperty;

	Brush ()
	{
	}
	Value::Kind GetObjectType () { return Value::BRUSH; };
	virtual void SetupBrush (cairo_t *cairo) = 0;
};
double		brush_get_opacity		(Brush *brush);
void		brush_set_opacity		(Brush *brush, double opacity);
TransformGroup	*brush_get_relative_transform	(Brush *brush);
void		brush_set_relative_transform	(Brush *brush, TransformGroup* transform_group);
TransformGroup	*brush_get_transform		(Brush *brush);
void		brush_set_transform		(Brush *brush, TransformGroup* transform_group);


class SolidColorBrush : public Brush {

 public:
	static DependencyProperty* ColorProperty;

	SolidColorBrush () { };

	Value::Kind GetObjectType () { return Value::SOLIDCOLORBRUSH; };

	virtual void SetupBrush (cairo_t *cairo);
};
SolidColorBrush	*solid_color_brush_new ();
Color		*solid_color_brush_get_color (SolidColorBrush *solid_color_brush);
void		solid_color_brush_set_color (SolidColorBrush *solid_color_brush, Color *color);

class GradientBrush : public Brush {
 public:
	GradientBrush ();
	// ColorInterpolationMode{ScRgbLinearInterpolation, SRgbLinearInterpolation} mode;
	// GradientStopCollection stops
	// BrushMappingMode{Absolute,RelativeToBoundingBox} MappingMode
	// GradientSpreadMethod{Pad, Reflect, Repeat} SpreadMethod
	virtual void SetupBrush (cairo_t *cairo);
};



enum Stretch {
	StretchNone,
	StretchFill,
	StretchUniform,
	StretchUniformToFill
};

enum PenLineCap {
	PenLineCapFlat,
	PenLineCapSquare,
	PenLineCapRound,
	PenLineCapTriangle
};

enum PenLineJoin {
	PenLineJoinMiter,
	PenLineJoinBevel,
	PenLineJoinRound
};

enum FillRule {
	FillRuleEvenOdd,
	FileRuleNonzero
};

enum SweepDirection {
	SweepDirectionCounterclockwise,
	SweepDirectionClockwise
};


class TriggerAction : public DependencyObject {

 public:
	TriggerAction () { };

	Value::Kind GetObjectType () { return Value::TRIGGERACTION; };
       virtual void Fire () = 0;
};


class EventTrigger : public DependencyObject {

 public:
	char *routed_event;
	GSList *actions;

	EventTrigger () : routed_event (NULL), actions (NULL)
	{
	}

	Value::Kind GetObjectType () { return Value::EVENTTRIGGER; };

	void AddAction (TriggerAction *action);

	void SetTarget (DependencyObject *target);
};

EventTrigger  *event_trigger_new ();
void          event_trigger_action_add (EventTrigger *trigger, TriggerAction *action);
void          event_trigger_fire_actions (EventTrigger *trigger);


//
// Item class
//
class UIElement : public DependencyObject {
 public:
	UIElement () :
		parent(NULL), flags (0),
		user_xform_origin(0,0),
		x1 (0), y1(0), x2(0), y2(0)
		{
			cairo_matrix_init_identity (&absolute_xform);
		}
	
	Value::Kind GetObjectType () { return Value::UIELEMENT; };

	UIElement *parent;

	enum UIElementFlags {
		IS_CANVAS = 1,
		IS_LOADED = 2
	};
	
	int flags;

	// The computed bounding box
	double x1, y1, x2, y2;

	//
	// Affine transformations:
	// 
	Point   user_xform_origin;	// transformation origin, user set

	// Absolute affine transform, precomputed with all of its data
	cairo_matrix_t absolute_xform;

	//
	// update_xform:
	//   Updates the absolute_xform for this item
	//
	virtual void update_xform ();
	
	//
	// render: 
	//   Renders the given @item on the @surface.  the area that is
	//   exposed is delimited by x, y, width, height
	//
	virtual void render (Surface *surface, int x, int y, int width, int height) = 0;

	// 
	// getbounds:
	//   Updates the bounding box for the given item, this uses the parent
	//   chain to compute the composite affine.
	//
	// Output:
	//   the item->x1,y1,x2,y2 values are updated.
	// 
	virtual void getbounds () = 0;

	//
	// get_xform_for
	//   Obtains the affine transform for the given child, this is
	//   implemented by containers

	virtual void get_xform_for (UIElement *item, cairo_matrix_t *result);
	
	//
	// gencenter:
	//   Returns the transformation origin based on  of the item and the
	//   xform_origin
	virtual Point getxformorigin () {
		return Point (0, 0);
	}

	~UIElement ();

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);

	static DependencyProperty* RenderTransformProperty;
};

Surface *item_get_surface          (UIElement *item);
void     item_invalidate           (UIElement *item);
void     item_update_bounds        (UIElement *item);
void     item_set_transform        (UIElement *item, double *transform);
void     item_set_transform_origin (UIElement *item, Point p);

void     item_set_render_transform (UIElement *item, Transform *transform);
void     item_get_render_affine    (UIElement *item, cairo_matrix_t *result);

//
// FrameworkElement class
//
class FrameworkElement : public UIElement {
 public:
	static DependencyProperty* HeightProperty;
	static DependencyProperty* WidthProperty;

	Collection triggers;

	FrameworkElement () {} 
};

double	framework_element_get_height	(FrameworkElement *framework_element);
void	framework_element_set_height	(FrameworkElement *framework_element, double height);
double	framework_element_get_width	(FrameworkElement *framework_element);
void	framework_element_set_width	(FrameworkElement *framework_element, double width);
void	framework_element_trigger_add   (FrameworkElement *framework_element, EventTrigger *trigger);

//
// Panel Class
//
class Panel : public FrameworkElement {
 public:
	VisualCollection *children;

	Panel ();

	static DependencyProperty* ChildrenProperty;

	virtual void OnPropertyChanged (DependencyProperty *prop);
};

// For C API usage.
void  panel_child_add      (Panel *panel, UIElement *item);

//
// Canvas Class, the only purpose is to have the Left/Top properties that
// children can use
//
class Canvas : public Panel {
 public:
	Canvas ();
	Surface *surface;
	
	virtual Point getxformorigin () { return Point (0, 0); }

	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual void update_xform ();
	virtual void get_xform_for (UIElement *item, cairo_matrix_t *result);
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);

	static DependencyProperty* TopProperty;
	static DependencyProperty* LeftProperty;
};

Canvas *canvas_new ();


// A video is an UIElement
class Video : public UIElement {
	enum { VIDEO_OK, VIDEO_ERROR_OPEN, VIDEO_ERROR_STREAM_INFO } VideoError;
 public:
	char  *filename;
	int    error;

	virtual Point getxformorigin () = 0;
	
	Video (const char *filename);
	~Video ();
};

Video *video_new     (const char *filename);
void   video_destroy (Video *video);

typedef struct _SurfacePrivate SurfacePrivate;

//
// Surface:
//
class Surface {
 public:
	Surface () : width (0), height (0), buffer (0), 
		cairo_buffer_surface (NULL), cairo_buffer(NULL),
		xlib_surface(NULL), cairo_xlib(NULL), pixmap(NULL),
		using_cairo_xlib_surface(0), pixbuf(NULL),
		cairo (NULL) {}
	
	int width, height;

	// The data lives here
	unsigned char *buffer;

	// The above buffer, as a pixbuf, for the software mode
	GdkPixbuf *pixbuf;
	
	bool using_cairo_xlib_surface;
	
	cairo_surface_t *cairo_buffer_surface;
	cairo_t         *cairo_buffer;
	cairo_surface_t *xlib_surface;
	cairo_t         *cairo_xlib;
	
	//
	// This is what code uses, and its equal to either:
	//    cairo_buffer: when the widget has not been realized
	//    cairo_xlib:   when the widget has been realized
	//
	cairo_t *cairo;		

	// The pixmap used for the backing storage for xlib_surface
	GdkPixmap *pixmap;

	// The widget where we draw.
	GtkWidget *drawing_area;

	// This currently can only be a canvas.
	UIElement *toplevel;

	int frames;
};

Surface *surface_new       (int width, int height);
void     surface_attach    (Surface *s, UIElement *element);
void     surface_init      (Surface *s, int width, int height);
void     surface_clear     (Surface *s, int x, int y, int width, int height);
void     surface_clear_all (Surface *s);
void     surface_destroy   (Surface *s);
void     surface_repaint   (Surface *s, int x, int y, int width, int height);

void    *surface_get_drawing_area (Surface *s);

//
// XAML
//

UIElement  *xaml_create_from_file     (const char *filename, Value::Kind *element_type);
UIElement  *xaml_create_from_str      (const char *xaml, Value::Kind *element_type);


void runtime_init ();
void animation_init ();
void brush_init ();
void transform_init ();
void shape_init ();
void geometry_init ();
void xaml_init ();
void types_init ();

G_END_DECLS

#endif
