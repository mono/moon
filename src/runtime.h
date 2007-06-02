G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

class Surface;
class Brush;
	
typedef void (*BrushChangedNotify)(Brush *brush, void *data);

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

//
// Item class
//
class UIElement {
 public:
	UIElement () : parent(NULL), flags (0), xform (NULL), x1 (0), y1(0), x2(0), y2(0) {}
	
	UIElement *parent;

	enum UIElementFlags {
		IS_SURFACE = 1
	};
	
	int flags;
	
	// The computed bounding box
	double x1, y1, x2, y2;

	// If null, identity, otherwise affine transformation.
	double *xform;

	//
	// render: 
	//   Renders the given @item on the @surface.  The parent affine transformation is in
	//   @affine, and the area that is exposed is delimited by x, y, width, height
	//
	virtual void render (Surface *surface, double *affine, int x, int y, int width, int height) = 0;

	// 
	// getbounds:
	//   Updates the bounding box for the given item, this uses the parent
	//   chain to compute the composite affine.
	//
	// Output:
	//   the item->x1,y1,x2,y2 values are updated.
	// 
	virtual void getbounds () = 0;
};

Surface *item_surface_get   (UIElement *item);
void     item_destroy       (UIElement *item);
void     item_invalidate    (UIElement *item);
void     item_update_bounds (UIElement *item);
void     item_transform_set (UIElement *item, double *transform);
double  *item_get_affine    (double *container, double *affine, double *result);
Surface *item_surface_get   (UIElement *item);
double  *item_affine_get_absolute (UIElement *item, double *result);

//
// Panel Class
//
class Panel : public UIElement {
 public:
	GSList *children;

	Panel () : children (NULL) {}
	
	virtual void render (Surface *s, double *affine, int x, int y, int width, int height);
	virtual void getbounds ();
};

void panel_child_add (Panel *group, UIElement *item);

//
// Canvas Class, the only purpose is to have the Left/Top properties that
// children can use
//
class Canvas : public Panel {
};

//
// Shape class 
// 
class Shape : public UIElement {
	void DoDraw (Surface *s, double *affine, bool do_op);
 public: 
	Brush *fill, *stroke;

	Shape () : fill (NULL), stroke (NULL) {}

	//
	// Overrides from UIElement.
	//
	virtual void render (Surface *s, double *affine, int x, int y, int width, int height);
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
};
Line *line_new  (double x1, double y1, double x2, double y2);



// A video is an UIElement
class Video : public UIElement {
	enum { VIDEO_OK, VIDEO_ERROR_OPEN, VIDEO_ERROR_STREAM_INFO } VideoError;
 public:
	double x, y;
	char  *filename;
	int    error;

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

G_END_DECLS
