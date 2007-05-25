#ifdef __cplusplus
extern "C" {
#endif
	
#include <cairo.h>

typedef struct _Item Item;
typedef struct _Surface Surface;

typedef struct {
	void (*nothing) ();
} ObjectVtable;
	
typedef struct {
	void *vtable;
} Object;

void object_init (Object *object);
		
typedef struct {
	double opacity;
	double *relative_transform;
	double *transform;

	GList *listeners;
} Brush;
	
typedef struct {
	ObjectVtable object_vtable;
	
	//
	// render: 
	//   Renders the given @item on the @surface.  The parent affine transformation is in
	//   @affine, and the area that is exposed is delimited by x, y, width, height
	//
	void (*render)    (Item *item, Surface *surface, double *affine, int x, int y, int width, int height);

	// 
	// getbounds:
	//   Updates the bounding box for the given item, this uses the parent
	//   chain to compute the composite affine.
	//
	// Input parameters: 
	//   @item:   the item
	//
	// Output:
	//   the item->x1,y1,x2,y2 values are updated.
	// 
	void (*getbounds) (Item *item);
} ItemVtable;

struct _Item {
	Object object;

        // OpacityProperty;
        // ClipProperty;
        // RenderTransformProperty;
        // TriggersProperty;
        // OpacityMaskProperty;
        // RenderTransformOriginProperty;
        // CursorProperty;
        // IsHitTestVisibleProperty;
        // VisibilityProperty;
        // ResourcesProperty;
        // ZIndexProperty;

	Item *parent;

	// The computed bounding box
	double x1, y1, x2, y2;

	// If null, identity, otherwise affine transformation.
	double *xform;
};

Surface *item_surface_get   (Item *item);
void     item_init          (Item *item);
void     item_destroy       (Item *item);
void     item_invalidate    (Item *item);
void     item_update_bounds (Item *item);
void     item_transform_set (Item *item, double *transform);
double  *item_get_affine    (double *container, double *affine, double *result);
Surface *item_surface_get   (Item *item);
double  *item_affine_get_absolute (Item *item, double *result);

typedef struct {
	Item item;

	GSList *items;
} Group;

void group_init     (Group *group);
void group_item_add (Group *group, Item *item);

typedef struct {
	Item item;

	// Brush: Fill, Stroke
} Shape;

void shape_init (Shape *shape);

typedef struct {
	Shape shape;
	double x, y, w, h;
} Rectangle;

void  rectangle_init (Rectangle *rectangle);
Item *rectangle_new  (double x, double y, double w, double h);

// A video is an Item
typedef struct _Video Video;

void  video_init    (Video *video);
Item *video_new     (const char *filename, double x, double y);
void  video_destroy (Video *video);

typedef struct _SurfacePrivate SurfacePrivate;
	
struct _Surface {
	Group group;

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

// Vtables
extern ItemVtable item_vtable;

// External class init
void video_class_init ();

#ifdef __cplusplus
};
#endif
