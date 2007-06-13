#ifndef __RUNTIME_H__
#define __RUNTIME_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "value.h"

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

	Point operator+ (const Point &point)
	{
		return Point (x + point.x,
			      y + point.y);
	}

	Point operator- (const Point &point)
	{
		return Point (x - point.x,
			      y - point.y);
	}

	Point operator* (double v)
	{
		return Point (x * v, y * v);
	}
};

//
// Arrays derive from this format
//
struct BasicArray {
public:
	uint32_t count;
	uint32_t refcount;	// Double purpose: refcount and pad. 
};

Point point_from_str (const char *s);

struct PointArray {
 public:
	BasicArray basic;
	Point points [0];
};

PointArray *point_array_new (int count, Point *points);
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

	Color operator+ (const Color &color)
	{
		return Color (r + color.r,
			      g + color.g,
			      b + color.b,
			      a + color.a);
	}

	Color operator- (const Color &color)
	{
		return Color (r - color.r,
			      g - color.g,
			      b - color.b,
			      a - color.a);
	}

	Color operator* (double v)
	{
		return Color (r * v,
			      g * v,
			      b * v,
			      a * v);
	}
};

Value  value_color_from_argb (uint32_t value);
Color *color_from_str  (const char *name);

struct DoubleArray {
 public:
	BasicArray basic;
	double values [0];
};

double* double_array_from_str   (const char *s, int* count);
DoubleArray *double_array_new   (int count, double *values);

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
	virtual ~Base () {} 
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
	static DependencyProperty *Register (Value::Kind type, const char *name, Value *default_value);
	static DependencyProperty *Register (Value::Kind type, const char *name, Value::Kind vtype);
	static DependencyProperty *RegisterFull (Value::Kind type, const char *name, Value *default_value, Value::Kind vtype, bool attached);
	
	static DependencyProperty *GetDependencyProperty (Value::Kind type, const char *name);
	static DependencyProperty *GetDependencyProperty (Value::Kind type, const char *name, bool inherits);

	void SetValue (DependencyProperty *property, Value value);
	void SetValue (DependencyProperty *property, Value *value);
	void SetValue (const char *name, Value *value);
	void SetValue (const char *name, Value value);
	Value *GetValue (DependencyProperty *property);
	Value *GetValue (const char *name);
	bool HasProperty (const char *name, bool inherits);
	DependencyProperty *GetDependencyProperty (const char *name);
	DependencyObject* FindName (const char *name);

	EventObject *events;
	static GHashTable *properties;

	virtual void OnPropertyChanged (DependencyProperty *property) {}
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop) { }
	virtual bool OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child) { return FALSE; }
	
	virtual Value::Kind GetObjectType ()
	{
		g_warning ("This class is missing an override of GetObjectType ()");
		return Value::DEPENDENCY_OBJECT; 
	};
	Type* GetType ()
	{
		return Type::Find (GetObjectType ());
	};

	void SetParent (DependencyObject *parent);
	DependencyObject* GetParent ();

 protected:
	void NotifyAttacheesOfPropertyChange (DependencyProperty *property);
	void NotifyParentOfPropertyChange (DependencyProperty *property, bool only_exact_type);

 private:
	GHashTable        *current_values;
	GSList            *attached_list;
	DependencyObject  *parent;
};

Value *dependency_object_get_value (DependencyObject *object, DependencyProperty *prop);
void   dependency_object_set_value (DependencyObject *object, DependencyProperty *prop, Value *val);

//
// DependencyProperty
//
class DependencyProperty {
 public:
	DependencyProperty () {} ;
	~DependencyProperty ();
	DependencyProperty (Value::Kind type, const char *name, Value *default_value, Value::Kind value_type, bool attached);

	char *name;
	Value *default_value;
	Value::Kind type;
	Value::Kind value_type;
	bool is_attached_property;
};

DependencyProperty *dependency_property_lookup (Value::Kind type, char *name);

class NameScope : public DependencyObject {
 public:
	NameScope ();
	~NameScope ();

	Value::Kind GetObjectType () { return Value::NAMESCOPE; }

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

 protected:
	void Add (DependencyObject *data);
	void Remove (DependencyObject *data);
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

class TriggerCollection : public Collection {
 public:
	TriggerCollection () {}
	virtual Value::Kind GetObjectType () { return Value::TRIGGER_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
};

class TriggerActionCollection : public Collection {
 public:
	TriggerActionCollection () {}
	virtual Value::Kind GetObjectType () { return Value::TRIGGERACTION_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
};

class ResourceCollection : public Collection {
 public:
	ResourceCollection () {}
	virtual Value::Kind GetObjectType () { return Value::RESOURCE_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
	
};

class StrokeCollection : public Collection {
 public:
	StrokeCollection () {}
	virtual Value::Kind GetObjectType () { return Value::STROKE_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
	
};

class StylusPointCollection : public Collection {
 public:
	StylusPointCollection () {}
	virtual Value::Kind GetObjectType () { return Value::STYLUSPOINT_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
	
};

class TimelineMarkerCollection : public Collection {
 public:
	TimelineMarkerCollection () {}
	virtual Value::Kind GetObjectType () { return Value::TIMELINEMARKER_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
	
};

class GradientStopCollection : public Collection {
 public:
	GradientStopCollection () {}
	virtual Value::Kind GetObjectType () { return Value::GRADIENTSTOP_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
	
};

class MediaAttributeCollection : public Collection {
 public:
	MediaAttributeCollection () {}
	virtual Value::Kind GetObjectType () { return Value::MEDIAATTRIBUTE_COLLECTION; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
	
};

class Inlines : public Collection {
 public:
	Inlines () {}
	virtual Value::Kind GetObjectType () { return Value::INLINES; }

	virtual void Add    (void *data);
	virtual void Remove (void *data);
	
};


VisualCollection*          visual_collection_new ();
TriggerCollection*         trigger_collection_new ();
TriggerActionCollection*   trigger_action_collection_new ();
/*
ResourceCollection*        resource_collection_new ();
StrokeCollection*          stroke_collection_new ();
StylusPointCollection*     stylus_point_collection_new ();
TimelineMarkerCollection*  time_line_marker_collection_new ();
GradientStopCollection*    gradient_stop_collection_new ();
MediaAttributeCollection*  media_attribute_collection_new ();
*/


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
	TriggerActionCollection *actions;

	EventTrigger ();

	Value::Kind GetObjectType () { return Value::EVENTTRIGGER; };

	void SetTarget (DependencyObject *target);
	void RemoveTarget (DependencyObject *target);

	virtual void OnPropertyChanged (DependencyProperty *prop);

	static DependencyProperty* RoutedEventProperty;
	static DependencyProperty* ActionsProperty;
};

EventTrigger  *event_trigger_new ();
void          event_trigger_action_add (EventTrigger *trigger, TriggerAction *action);
void          event_trigger_fire_actions (EventTrigger *trigger);


//
// Item class
//
class UIElement : public DependencyObject {
 public:
	UIElement ();
		
	
	Value::Kind GetObjectType () { return Value::UIELEMENT; };

	UIElement *parent;

	enum UIElementFlags {
		IS_CANVAS = 1,
		IS_LOADED = 2
	};
	
	int flags;

	// The computed bounding box
	double x1, y1, x2, y2;

	// Absolute affine transform, precomputed with all of its data
	cairo_matrix_t absolute_xform;

	TriggerCollection *triggers;

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
	// Recomputes the bounding box, requests redraws, 
	// the parameter determines if we should also update the transformation
	//
	void FullInvalidate (bool render_xform);
	
	//
	// gencenter:
	//   Returns the transformation origin based on  of the item and the
	//   xform_origin
	virtual Point getxformorigin () {
		return Point (0, 0);
	}

	//
	// inside_object:
	//   Returns whether the position x, y is inside the object
	//
	virtual bool inside_object (Surface *s, double x, double y);
	
	//
	// handle_motion:
	//   handles an mouse motion event, and dispatches it to anyone that
	//   might want it
	//
	virtual void handle_motion (Surface *s, int state, double x, double y);
	
	~UIElement ();

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);

	Point GetRenderTransformOrigin () {
		Value *vu = GetValue (UIElement::RenderTransformOriginProperty);
		if (vu)
			return *vu->AsPoint ();
		return Point (0, 0);
	}

	static DependencyProperty* RenderTransformProperty;
	static DependencyProperty* OpacityProperty;
	static DependencyProperty* ClipProperty;
	static DependencyProperty* OpacityMaskProperty;
	static DependencyProperty* RenderTransformOriginProperty;
	static DependencyProperty* CursorProperty;
	static DependencyProperty* IsHitTestVisibleProperty;
	static DependencyProperty* VisibilityProperty;
	static DependencyProperty* ResourcesProperty;
	static DependencyProperty* TriggersProperty;
};

Surface *item_get_surface          (UIElement *item);
void     item_invalidate           (UIElement *item);
void     item_update_bounds        (UIElement *item);
void     item_set_transform        (UIElement *item, double *transform);
void     item_set_transform_origin (UIElement *item, Point p);

void     item_set_render_transform (UIElement *item, Transform *transform);
void     item_get_render_affine    (UIElement *item, cairo_matrix_t *result);

double	uielement_get_opacity (UIElement *item);
void	uielement_set_opacity (UIElement *item, double opacity);

//
// FrameworkElement class
//
class FrameworkElement : public UIElement {
 public:
	static DependencyProperty* HeightProperty;
	static DependencyProperty* WidthProperty;

	FrameworkElement ();
	Value::Kind GetObjectType () { return Value::FRAMEWORKELEMENT; }

	virtual bool inside_object (Surface *s, double x, double y);
};

double	framework_element_get_height	(FrameworkElement *framework_element);
void	framework_element_set_height	(FrameworkElement *framework_element, double height);
double	framework_element_get_width	(FrameworkElement *framework_element);
void	framework_element_set_width	(FrameworkElement *framework_element, double width);

//
// Panel Class
//
class Panel : public FrameworkElement {
 public:
	VisualCollection *children;

	Panel ();
	Value::Kind GetObjectType () { return Value::PANEL; }

	static DependencyProperty* ChildrenProperty;
	static DependencyProperty* BackgroundProperty;

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
	
	Value::Kind GetObjectType () { return Value::CANVAS; }

	virtual Point getxformorigin () { return Point (0, 0); }

	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual void update_xform ();
	virtual void get_xform_for (UIElement *item, cairo_matrix_t *result);
	virtual void handle_motion (Surface *s, int state, double x, double y);

	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);

	static DependencyProperty* TopProperty;
	static DependencyProperty* LeftProperty;
};

Canvas *canvas_new ();

typedef struct _SurfacePrivate SurfacePrivate;

//
// Surface:
//
typedef void (*callback_mouse_event)    (UIElement *target, int state, double x, double y);
typedef void (*callback_plain_event)    (UIElement *target);
typedef void (*callback_keyboard_event) (UIElement *target, int state, int platformcode, int key);
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

	callback_mouse_event cb_motion, cb_down, cb_up, cb_enter;
	callback_plain_event cb_got_focus, cb_lost_focus, cb_loaded, cb_mouse_leave;
	callback_keyboard_event cb_keydown, cb_keyup;

};

Surface *surface_new       (int width, int height);
void     surface_attach    (Surface *s, UIElement *element);
void     surface_init      (Surface *s, int width, int height);
void     surface_clear     (Surface *s, int x, int y, int width, int height);
void     surface_clear_all (Surface *s);
void     surface_destroy   (Surface *s);
void     surface_repaint   (Surface *s, int x, int y, int width, int height);

void    *surface_get_drawing_area (Surface *s);

void     surface_register_events (Surface *s,
				  callback_mouse_event motion, callback_mouse_event down, callback_mouse_event up,
				  callback_mouse_event enter,
				  callback_plain_event got_focus, callback_plain_event lost_focus,
				  callback_plain_event loaded, callback_plain_event mouse_leave,
				  callback_keyboard_event keydown, callback_keyboard_event keyup);
		      

//
// XAML
//

UIElement  *xaml_create_from_file     (const char *filename, Value::Kind *element_type);
UIElement  *xaml_create_from_str      (const char *xaml, Value::Kind *element_type);


MediaElement *video_new (const char *filename);


void runtime_init ();
void animation_init ();
void brush_init ();
void clock_init ();
void transform_init ();
void shape_init ();
void geometry_init ();
void xaml_init ();
void types_init ();
void dependencyobject_init ();

G_END_DECLS

#endif
