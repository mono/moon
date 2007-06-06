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

	Point (Point *point)
	{
		x = point->x;
		y = point->y;
	}
};

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

	Rect (Rect *rect)
	{
		x = rect->x;
		y = rect->y;
		w = rect->w;
		h = rect->h;
	}
};

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

	Color (Color *color)
	{
		if (color) {
			r = color->r;
			g = color->g;
			b = color->b;
			a = color->a;
		} else {
			r = g = b = 1.0;
			a = 0.0;
		}
	}
};


Color *color_from_str (const char *name);


//
// Collection: provides a collection that we can monitor for
// changes.   We expose this collection in a few classes to
// the managed world, and when a change happens we get a
// chance to reflect the changes
//
class Collection;

typedef void (*collection_item_add)    (Collection *col, void *datum); 
typedef void (*collection_item_remove) (Collection *col, void *datum);

class Collection {
 public:
	collection_item_add     add_fn;
	collection_item_remove  remove_fn;
	
	GSList *list;
	void *closure;

	Collection () { Setup (NULL, NULL, NULL); }
	
	Collection (collection_item_add add, collection_item_remove remove, void *data)
	{
		Setup (add, remove, data);
	}

	void Setup (collection_item_add add, collection_item_remove remove, void *data)
	{
		list = NULL;
		add_fn = add;
		remove_fn = remove;
		closure = data;
	}
};

void collection_add    (Collection *collection, void *data);
void collection_remove (Collection *collection, void *data);

struct Value {
public:
	enum Kind {
		INVALID = 0,
		BOOL = 1,
		DOUBLE = 2,
		UINT64 = 3,
		INT64 = 3,
		INT32 = 4,
		STRING = 5,
		COLOR = 7,
		POINT = 8,
		RECT = 9,

		DEPENDENCY_OBJECT = 1000,

		// These are dependency objects
		UIELEMENT,
		PANEL,
		CANVAS,
		TIMELINE,
		ROTATETRANSFORM,
		SCALETRANSFORM,
		TRANSLATETRANSFORM,
		MATRIXTRANSFORM,
		STORYBOARD,
		DOUBLEANIMATION,
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
		BRUSH,
		SOLIDCOLORBRUSH,
		PATHFIGURE,
		ARCSEGMENT,
		BEZIERSEGMENT,
		LINESEGMENT,
		POLYBEZIERSEGMENT,
		POLYQUADRATICBEZIERSEGMENT,
		QUADRATICBEZIERSEGMENT,
		TRIGGERACTION,
		BEGINSTORYBOARD,
	};

	Kind k;
	union {
		bool z;
		double d;
		guint64 ui64;
		gint64 i64;
		gint32 i32;
		char *s;
		DependencyObject *dependency_object;
		Color *color;
		Point *point;
		Rect *rect;
	} u;

	Value () : k (INVALID) {}
	
	void Init ()
	{
		memset (&u, 0, sizeof (u));
	}

	Value (bool z)
	{
		Init ();
		k = BOOL;
		u.z = z;
	}

	Value (double d)
	{
		Init ();
		k = DOUBLE;
		u.d = d;
	}

	Value (guint64 i)
	{
		Init ();
		k = UINT64;
		u.ui64 = i;
	}

	Value (gint64 i)
	{
		Init ();
		k = INT64;
		u.i64 = i;
	}

	Value (gint32 i)
	{
		Init ();
		k = INT32;
		u.i32 = i;
	}

	Value (Color *c)
	{
		Init ();
		k = COLOR;
		u.color = new Color (c);
	}

	Value (DependencyObject *obj)
	{
		g_assert (obj != NULL);
		
		Init ();
		k = DEPENDENCY_OBJECT;
		u.dependency_object = obj;
	}

	Value (Point *pt)
	{
		g_assert (pt != NULL);

		Init ();
		k = POINT;
		u.point = new Point (pt);
	}

	Value (Rect *rect)
	{
		g_assert (rect != NULL);

		Init ();
		k = RECT;
		u.rect = new Rect (rect);
	}
	
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

	Value (const char* s)
	{
		Init ();
		k = STRING;
		u.s= g_strdup (s);
	}

	~Value ()
	{
		if (k == STRING)
			g_free (u.s);
	}
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
	Value *GetValue (DependencyProperty *property);
	DependencyProperty *GetDependencyProperty (char *name);
	Value::Kind GetObjectType ();

	DependencyObject* FindName (char *name);

	EventObject *events;

	virtual void OnPropertyChanged (DependencyProperty *property) {}
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop) { }

 protected:
	void NotifyAttacheesOfPropertyChange (DependencyProperty *property);
	void SetObjectType (Value::Kind objectType);

 private:
	Value::Kind objectType;
	static GHashTable *properties;
	GHashTable        *current_values;
	GSList            *attached_list;
};

//
// DependencyProperty
//
class DependencyProperty {
 public:
	DependencyProperty () {} ;
	~DependencyProperty ();
	DependencyProperty (char *name, Value *default_value, Value::Kind kind);

	char *name;
	Value *default_value;
	Value::Kind type;
	Value::Kind value_type;
};

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


class Brush : public DependencyObject {
 public:
	static DependencyProperty* OpacityProperty;
	static DependencyProperty* RelativeTransformProperty;
	static DependencyProperty* TransformProperty;

	Brush ()
	{
		SetObjectType (Value::BRUSH);
	}
	
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

	SolidColorBrush ()
	{
		SetObjectType (Value::SOLIDCOLORBRUSH);
	}

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
       TriggerAction ()
       {
               SetObjectType (Value::TRIGGERACTION);
       }
};



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
			SetObjectType (Value::UIELEMENT);
			cairo_matrix_init_identity (&absolute_xform);
		}
	
	UIElement *parent;

	enum UIElementFlags {
		IS_SURFACE = 1
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
// Panel Class
//
class Panel : public UIElement {
 public:
	Collection children;

	Panel ();
};

// panel_get_collection, exposed to the managed world so it can manipulate the collection
void *panel_get_collection (Panel *panel);

// For C API usage.
void  panel_child_add      (Panel *panel, UIElement *item);

//
// Canvas Class, the only purpose is to have the Left/Top properties that
// children can use
//
class Canvas : public Panel {
 public:
	virtual Point getxformorigin () { return Point (0, 0); }

	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual void update_xform ();
	virtual void get_xform_for (UIElement *item, cairo_matrix_t *result);
	
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);

	static DependencyProperty* TopProperty;
	static DependencyProperty* LeftProperty;
};

Canvas * canvas_new ();


//
// FrameworkElement class
//
class FrameworkElement : public UIElement {
 public:
	static DependencyProperty* HeightProperty;
	static DependencyProperty* WidthProperty;

	FrameworkElement () {} 
};

double	framework_element_get_height	(FrameworkElement *framework_element);
void	framework_element_set_height	(FrameworkElement *framework_element, double height);
double	framework_element_get_width	(FrameworkElement *framework_element);
void	framework_element_set_width	(FrameworkElement *framework_element, double width);

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
// We probably should make the Surface not derive from Canvas, but for now
// it will do.
//
class Surface : public Canvas {
 public:
	Surface () : width (0), height (0), buffer (0), 
		cairo_buffer_surface (NULL), cairo_buffer(NULL),
		xlib_surface(NULL), cairo_xlib(NULL), pixmap(NULL),
		cairo (NULL) {}
	
	int width, height;

	// The data lives here
	unsigned char *buffer;

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
					
	//
	// Consumer-defined data, we use it to pass the handle to invalidate
	//
//	void *data;
};

Surface *surface_new       (int width, int height);
void     surface_init      (Surface *s, int width, int height);
void     surface_clear     (Surface *s, int x, int y, int width, int height);
void     surface_clear_all (Surface *s);
void     surface_destroy   (Surface *s);
void     surface_repaint   (Surface *s, int x, int y, int width, int height);

void    *surface_get_drawing_area (Surface *s);

//
// XAML
//

UIElement  *xaml_create_from_file     (const char *filename);
UIElement  *xaml_create_from_str      (const char *xaml);


void runtime_init ();
void animation_init ();
void brush_init ();
void transform_init ();
void shape_init ();
void geometry_init ();
void xaml_init ();

G_END_DECLS

#endif
