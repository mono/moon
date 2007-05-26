#ifdef __cplusplus
extern "C" {
#endif
	
#include <cairo.h>

class Surface;
	
typedef struct {
	double opacity;
	double *relative_transform;
	double *transform;

	GList *listeners;
} Brush;

class Item {
 public:
	Item () : parent(NULL), flags (0), xform (NULL), x1 (0), y1(0), x2(0), y2(0) {}
	
	Item *parent;

	enum ItemFlags {
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

Surface *item_surface_get   (Item *item);
void     item_destroy       (Item *item);
void     item_invalidate    (Item *item);
void     item_update_bounds (Item *item);
void     item_transform_set (Item *item, double *transform);
double  *item_get_affine    (double *container, double *affine, double *result);
Surface *item_surface_get   (Item *item);
double  *item_affine_get_absolute (Item *item, double *result);

class Group : public Item {
 public:
	GSList *items;

	Group () : items (NULL) {}
	
	virtual void render (Surface *s, double *affine, int x, int y, int width, int height);
	virtual void getbounds ();
};

void group_item_add (Group *group, Item *item);

class Shape : public Item {
	// Brush: Fill, Stroke
};

class Rectangle : public Shape {
 public:
	double x, y, w, h;

	Rectangle (double ix, double iy, double iw, double ih) : x(ix), y(iy), w(iw), h(ih) {};
	Rectangle () : x(0), y(0), w(0), h(0) {};
	
	virtual void render (Surface *s, double *affine, int x, int y, int width, int height);
	virtual void getbounds ();
};

Rectangle *rectangle_new  (double x, double y, double w, double h);

typedef struct _VideoPrivate VideoPrivate;
	
// A video is an Item
class Video : public Item {
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
	
class Surface : public Group {
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

#ifdef __cplusplus
};
#endif
