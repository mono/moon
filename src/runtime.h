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
	};
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
	};
};

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
		INT64 = 3,
		INT32 = 4,
		STRING = 5,
		DEPENDENCY_OBJECT = 6
	};

	Kind k;
	union {
		bool z;
		double d;
		gint64 i64;
		gint32 i32;
		char *s;
		DependencyObject *dependency_object;
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

	Value (DependencyObject *obj)
	{
		g_assert (obj != NULL);
		
		Init ();
		k = DEPENDENCY_OBJECT;
		u.dependency_object = obj;
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
	enum Type {
		INVALID = 0,
		UIELEMENT,
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
		RECTANGLEGEOMETRY
	};
	
	DependencyObject ();
	~DependencyObject ();
	static DependencyProperty* Register (Type type, char *name, Value *default_value);
	static DependencyProperty* GetDependencyProperty (Type type, char *name);
	void SetValue (DependencyProperty *property, Value value);
	Value *GetValue (DependencyProperty *property);

	EventObject *events;

	virtual void OnPropertyChanged (DependencyProperty *property) { }
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop) { }

 protected:
	void NotifyAttacheesOfPropertyChange (DependencyProperty *property);

 private:
	static GHashTable *default_values;
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
	DependencyProperty (char *name, Value *default_value);

	char *name;
	Value *default_value;
	DependencyObject::Type type;
};

class Brush : public Base {
 public:
	Brush () : opacity(0), relative_transform(NULL), transform(NULL) {}
	
	double opacity;
	double *relative_transform;
	double *transform;

	virtual void SetupBrush (cairo_t *cairo) = 0;
};

class SolidColorBrush : public Brush {
	Color color;
 public:
	SolidColorBrush (Color c) { color = c; } 

	virtual void SetupBrush (cairo_t *cairo);
};

class GradientBrush : public Brush {
 public:
	GradientBrush ();
	// ColorInterpolationMode{ScRgbLinearInterpolation, SRgbLinearInterpolation} mode;
	// GradientStopCollection stops
	// BrushMappingMode{Absolute,RelativeToBoundingBox} MappingMode
	// GradientSpreadMethod{Pad, Reflect, Repeat} SpreadMethod
	virtual void SetupBrush (cairo_t *cairo);
};

SolidColorBrush  *solid_brush_from_str (const char *name);


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

//
// Item class
//
class UIElement : public DependencyObject {
 public:
	UIElement () :
		parent(NULL), flags (0),
		absolute_xform (NULL),
		user_xform (NULL),
		user_xform_origin(0,0),
		x1 (0), y1(0), x2(0), y2(0)
		{}
	
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
	double *user_xform;		// If null, identity, otherwise affine xform, user set

	// Absolute affine transform
	double *absolute_xform;

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
	// set_prop_from_str
	//  takes a string value for a XAML property name and a string
	//  representing the value.
	virtual void set_prop_from_str (const char *pname, const char *vname);

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

void       item_set_render_transform (UIElement *item, Transform *transform);
Transform *item_get_render_transform (UIElement *item);

//
// Panel Class
//
class Panel : public UIElement {
 public:
	Collection children;

	Panel ();

	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual void update_xform ();
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

	virtual void set_prop_from_str (const char *pname, const char *vname);

	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();

	static DependencyProperty* TopProperty;
	static DependencyProperty* LeftProperty;
};

//
// FrameworkElement class
//
class FrameworkElement : public UIElement {
 public:
	double x, y;		// Canvas.TopProperty, Canvas.LeftProperty
	double w, h;

	FrameworkElement () : w(0), h(0) {} 

	virtual void set_prop_from_str (const char *prop, const char *value);
};

// A video is an UIElement
class Video : public UIElement {
	enum { VIDEO_OK, VIDEO_ERROR_OPEN, VIDEO_ERROR_STREAM_INFO } VideoError;
 public:
	double x, y;
	char  *filename;
	int    error;

	virtual Point getxformorigin () = 0;
	
	Video (const char *filename, double x, double y);
	~Video ();
};

Video *video_new     (const char *filename, double x, double y);
void  video_destroy  (Video *video);

typedef struct _SurfacePrivate SurfacePrivate;

//
// We probably should make the Surface not derive from Canvas, but for now
// it will do.
//
class Surface : public Canvas {
 public:
	Surface () : width (0), height (0), buffer (0), pixbuf (NULL), cairo_surface (NULL), cairo (NULL), data(NULL) {}
	
	int width, height;

	// The data lives here
	unsigned char *buffer;

	// Representations:

	SurfacePrivate *priv;
	
	// The same buffer, but this one is the pixbuf representation.
	GdkPixbuf *pixbuf;

	// The same buffer, but this one is for cairo.
	cairo_surface_t *cairo_surface;
	cairo_t *cairo;

	GtkWidget *drawing_area;
					
	//
	// Consumer-defined data, we use it to pass the handle to invalidate
	//
	void *data;
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
void transform_init ();
void shape_init ();
void geometry_init ();

G_END_DECLS

#endif
