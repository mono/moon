G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

class Surface;
class Brush;
	
typedef void (*BrushChangedNotify)(Brush *brush, void *data);

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

struct Color {
	double r, g, b, a;
 public:
	Color () {}
	
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

class Brush : public Base {
	GSList *listeners;
 public:
	Brush () : opacity(0), relative_transform(NULL), transform(NULL), listeners(NULL) {}
	
	double opacity;
	double *relative_transform;
	double *transform;

	void *AddListener    (BrushChangedNotify notify, void *data);
	void  RemoveListener (void *key);

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

//
// Item class
//
class UIElement {
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
};

Surface *item_get_surface          (UIElement *item);
void     item_invalidate           (UIElement *item);
void     item_update_bounds        (UIElement *item);
void     item_set_transform        (UIElement *item, double *transform);
void     item_set_transform_origin (UIElement *item, Point p);

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
};

//
// Shape class 
// 
class Shape : public UIElement {
	void DoDraw (Surface *s, bool do_op);
 public: 
	Brush *fill, *stroke;

	Shape () : fill (NULL), stroke (NULL) {}

	//
	// Overrides from UIElement.
	//
	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();

	//
	// new virtual methods for shapes
	//
	
	//
	// Draw: draws the Shape on the surface (affine transforms are set before this
	// is called). 
	//
	// This is called multiple times: one for fills, one for strokes
	// if they are both set.   It will also be called to compute the bounding box.
	//
	virtual void Draw (Surface *s) = 0;

	virtual void set_prop_from_str (const char *prop, const char *value);
};

void shape_set_fill   (Shape *shape, Brush *brush);
void shape_set_stroke (Shape *shape, Brush *brush);

//
// Rectangle class 
// 
class Rectangle : public Shape {
 public:
	double x, y, w, h;

	Rectangle (double ix, double iy, double iw, double ih) : x(ix), y(iy), w(iw), h(ih) {};
	Rectangle () : x(0), y(0), w(0), h(0) {};

	void Draw (Surface *s);

	virtual void set_prop_from_str (const char *prop, const char *value);

	virtual Point getxformorigin ();
};
Rectangle *rectangle_new  (double x, double y, double w, double h);

//
// Line class 
// 
class Line : public Shape {
 public:
	double line_x1, line_y1, line_x2, line_y2;

	Line (double px1, double py1, double px2, double py2) :
		line_x1(px1), line_y1(py1), line_x2(px2), line_y2(py2) {};
	
	void Draw (Surface *s);

	virtual void set_prop_from_str (const char *prop, const char *value);

	virtual Point getxformorigin ();
};
Line *line_new  (double x1, double y1, double x2, double y2);



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



UIElement  *xaml_create_from_file     (const char *filename);


G_END_DECLS
