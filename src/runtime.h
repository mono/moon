#ifndef __RUNTIME_H__
#define __RUNTIME_H__

G_BEGIN_DECLS

#include <stdint.h>
#include <cairo.h>

#include "value.h"
#include "type.h"
#include "list.h"

#define TIMERS 0
#if TIMERS
#define STARTTIMER(id,str) TimeSpan id##_t_start = get_now(); printf ("timing of '%s' started at %lld\n", str, id##_t_start)
#define ENDTIMER(id,str) TimeSpan id##_t_end = get_now(); printf ("timing of '%s' ended at %lld (%f seconds)\n", str, id##_t_end, (double)(id##_t_end - id##_t_start) / 1000000)
#else
#define STARTTIMER(id,str)
#define ENDTIMER(id,str)
#endif

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

	bool IntersectsWith (const Rect& rect)
	{
		return ((x < rect.x + rect.w) && (x + w > rect.x) && (y < rect.y + rect.h) && (y + h > rect.y));
	}

	Rect Intersection (const Rect& rect)
	{
		Rect result = Rect ();
		result.x = x > rect.x ? x : rect.x;
		result.y = y > rect.y ? y : rect.y;
		result.w = ((x + w < rect.x + rect.w) ? (x + w) : (rect.x + rect.w)) - result.x;
		result.h = ((y + h < rect.y + rect.h) ? (y + h) : (rect.y + rect.h)) - result.y;
		return result;
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

//
// This guy provide reference counting
//
#define BASE_FLOATS 0x80000000

class Base {
 public:	
	uint32_t refcount;
	Base () : refcount(BASE_FLOATS) { }
	virtual ~Base () { }
	
	void ref ();
	void unref ();
};

void base_ref (Base *base);
void base_unref (Base *base);



//
// DependencyObject
// 

class DependencyObject : public Base {
 public:

	DependencyObject ();
	virtual ~DependencyObject ();
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value);
	static DependencyProperty *Register (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *Register (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype);
	static DependencyProperty *RegisterNullable (Type::Kind type, const char *name, Type::Kind vtype);
	static DependencyProperty *RegisterFull (Type::Kind type, const char *name, Value *default_value, Type::Kind vtype, bool attached);
	
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name);
	static DependencyProperty *GetDependencyProperty (Type::Kind type, const char *name, bool inherits);
	static DependencyProperty *NameProperty;
	void SetValue (DependencyProperty *property, Value value);
	void SetValue (DependencyProperty *property, Value *value);
	void SetValue (const char *name, Value *value);
	void SetValue (const char *name, Value value);
	Value *GetValue (DependencyProperty *property);
	Value *GetValueNoDefault (DependencyProperty *property);
	Value *GetValue (const char *name);
	bool HasProperty (const char *name, bool inherits);
	DependencyProperty *GetDependencyProperty (const char *name);
	DependencyObject* FindName (const char *name);

	EventObject *events;
	static GHashTable *properties;

	virtual void OnPropertyChanged (DependencyProperty *property) {}
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop) { }

	//
	// OnChildPropertyChanged:
	//    This is raised on objects when a child of this object has had one of its
	//    properties changed.   This is used so that owning objects can monitor if
	//    one of the attached properties in a child must be acted upon
	//
	//    This code will go up in the ownership chain until this is handled, by 
	//    returning TRUE.
	//
	virtual bool OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child) { return FALSE; }
	
	virtual Type::Kind GetObjectType ();
	Type* GetType ()
	{
		return Type::Find (GetObjectType ());
	};

	void SetParent (DependencyObject *parent);
	DependencyObject* GetParent ();

	//
	// Is:
	//    Similar to C#'s is: it checks if this object is of this kind or 
	//    a derived class.
	
	bool Is(Type::Kind k) {
		return GetType ()->IsSubclassOf (k);
	};

	static void Shutdown ();

	void Attach (DependencyProperty *property, DependencyObject *container);
	void Detach (DependencyProperty *property, DependencyObject *container);
	
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

DependencyObject *dependency_object_find_name (DependencyObject *obj, const char *name, Type::Kind *element_type);
const char       *dependency_object_get_name  (DependencyObject *obj);

void dependency_object_add_event_handler (DependencyObject *o, char *event, EventHandler handler, gpointer closure);
void dependency_object_remove_event_handler (DependencyObject *o, char *event, EventHandler handler, gpointer closure);

//
// DependencyProperty
//
class DependencyProperty {
 public:
	DependencyProperty () {} ;
	~DependencyProperty ();
	DependencyProperty (Type::Kind type, const char *name, Value *default_value, Type::Kind value_type, bool attached);

	char *name;
	Value *default_value;
	Type::Kind type;
	bool is_attached_property;
	Type::Kind value_type;
	bool is_nullable;
	bool IsNullable () { return is_nullable; }
};

DependencyProperty *dependency_property_lookup (Type::Kind type, char *name);
char* dependency_property_get_name (DependencyProperty* property);
bool  dependency_property_is_nullable (DependencyProperty* property);
Type::Kind dependency_property_get_value_type (DependencyProperty* property);
DependencyProperty *resolve_property_path (DependencyObject **o, const char *path);

class NameScope : public DependencyObject {
 public:
	NameScope ();
	virtual ~NameScope ();

	virtual Type::Kind GetObjectType () { return Type::NAMESCOPE; }

	void RegisterName (const char *name, DependencyObject *object);
	void UnregisterName (const char *name);

	DependencyObject* FindName (const char *name);

	static NameScope* GetNameScope (DependencyObject *obj);
	static void SetNameScope (DependencyObject *obj, NameScope *scope);

	static DependencyProperty *NameScopeProperty;
	
 private:
	GHashTable *names;
};


class Visual : public DependencyObject {
 public:
	Visual () {};
	virtual Type::Kind GetObjectType () { return Type::VISUAL; };	

	//
	// inside_object:
	//   Returns whether the position x, y is inside the object
	//
	virtual bool inside_object (Surface *s, double x, double y) { return FALSE; }
};

enum ErrorType {
	NoError,
	UnknownError,
	InitializeError,
	ParserError,
	ObjectModelError,
	RuntimeError,
	DownloadError,
	MediaError,
	ImageError
};

struct ErrorEventArgs /* : public EventArgs */ {
 public:
	int error_code;;
	const char *error_message;
	ErrorType error_type;
};

struct ParserErrorEventArgs : public ErrorEventArgs {
 public:

	ParserErrorEventArgs () : char_position (0), line_number (0), xaml_file (NULL),
	xml_element (NULL), xml_attribute (NULL)
	{
		error_type = ParserError;
	}
	
	
	int char_position;
	int line_number;
	const char *xaml_file;
	const char *xml_element;
	const char *xml_attribute;
};

//
// Collection: provides a collection that we can monitor for
// changes.   We expose this collection in a few classes to
// the managed world, and when a change happens we get a
// chance to reflect the changes
//
class Collection : public DependencyObject {
 public:
	class Node : public List::Node {
	public:
		DependencyObject *obj;
		
		Node (DependencyObject *dob, DependencyObject *parent);
		~Node ();
	};
	
	List *list;
	void *closure;
	
	Collection ();
	virtual ~Collection ();
	virtual Type::Kind GetObjectType () { return Type::COLLECTION; };	
	virtual Type::Kind GetElementType () { return Type::DEPENDENCY_OBJECT; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
	virtual void Insert (int index, DependencyObject *data);

 private:
	void SharedAdd (DependencyObject *data);
};

bool CollectionNodeFinder (List::Node *n, void *data);

class CollectionIterator {
 public:
	CollectionIterator (Collection *c){
		collection = c;
		current = c->list->First ();
	}
	
	Collection *collection;
	List::Node *current;
};

void collection_add    (Collection *collection, DependencyObject *data);
void collection_remove (Collection *collection, DependencyObject *data);
void collection_insert (Collection *collection, int index, DependencyObject *data);
void collection_clear  (Collection *collection);

Type::Kind collection_get_element_type (Collection *collection);
CollectionIterator *collection_get_iterator (Collection *collection);

bool   collection_iterator_move_next   (CollectionIterator *iterator);
void   collection_iterator_reset       (CollectionIterator *iterator);
void   collection_iterator_destroy     (CollectionIterator *iterator);
DependencyObject *collection_iterator_get_current (CollectionIterator *iterator);

class VisualCollection : public Collection {
 public:
	VisualCollection () {}
	virtual Type::Kind GetObjectType () { return Type::VISUAL_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::VISUAL; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
	virtual void Insert (int index, DependencyObject *data);

 private:
	void VisualUpdate (DependencyObject *data);
};

class TriggerCollection : public Collection {
 public:
	TriggerCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TRIGGER_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::EVENTTRIGGER; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
	virtual void Insert (int index, DependencyObject *data);
};

class TriggerActionCollection : public Collection {
 public:
	TriggerActionCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TRIGGERACTION_COLLECTION; }
	/* this may seem wrong, but it's what the TriggerActionCollection mandates */
	virtual Type::Kind GetElementType () { return Type::BEGINSTORYBOARD; }
};

class ResourceCollection : public Collection {
 public:
	ResourceCollection () {}
	virtual Type::Kind GetObjectType () { return Type::RESOURCE_COLLECTION; }
	virtual Type::Kind GetElementType () { return Type::DEPENDENCY_OBJECT; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
};

class StrokeCollection : public Collection {
 public:
	StrokeCollection () {}
	virtual Type::Kind GetObjectType () { return Type::STROKE_COLLECTION; }
	//virtual Type::Kind GetElementType () { return Value::STROKE; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
};

class StylusPointCollection : public Collection {
 public:
	StylusPointCollection () {}
	virtual Type::Kind GetObjectType () { return Type::STYLUSPOINT_COLLECTION; }
	//virtual Type::Kind GetElementType () { return Value::STYLUSPOINT; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
};

class TimelineMarkerCollection : public Collection {
 public:
	TimelineMarkerCollection () {}
	virtual Type::Kind GetObjectType () { return Type::TIMELINEMARKER_COLLECTION; }
	//virtual Type::Kind GetElementType () { return Type::TIMELINEMARKER; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
};

class MediaAttributeCollection : public Collection {
 public:
	MediaAttributeCollection () {}
	virtual Type::Kind GetObjectType () { return Type::MEDIAATTRIBUTE_COLLECTION; }
	//virtual Type::Kind GetElementType () { return Type::MEDIAATTRIBUTE; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
};

class Inlines : public Collection {
 public:
	Inlines () {}
	virtual Type::Kind GetObjectType () { return Type::INLINES; }
	//virtual Type::Kind GetElementType () { return Type::INLINE; }

	virtual void Add    (DependencyObject *data);
	virtual void Remove (DependencyObject *data);
};

Collection *collection_new (Type::Kind kind);

VisualCollection *visual_collection_new (void);
TriggerCollection *trigger_collection_new (void);
TriggerActionCollection *trigger_action_collection_new (void);
ResourceCollection *resource_collection_new (void);
StrokeCollection *stroke_collection_new (void);
StylusPointCollection *stylus_point_collection_new (void);
TimelineMarkerCollection *timeline_marker_collection_new (void);
GradientStopCollection *gradient_stop_collection_new (void);
MediaAttributeCollection *media_attribute_collection_new (void);
Inlines *inlines_new (void);


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
	FillRuleNonzero
};

enum SweepDirection {
	SweepDirectionCounterclockwise,
	SweepDirectionClockwise
};


class TriggerAction : public DependencyObject {
 public:
	TriggerAction () { };

	virtual Type::Kind GetObjectType () { return Type::TRIGGERACTION; };
	virtual void Fire () = 0;
};


class EventTrigger : public DependencyObject {
 public:
	EventTrigger ();
	virtual ~EventTrigger ();
	
	virtual Type::Kind GetObjectType () { return Type::EVENTTRIGGER; };

	void SetTarget (DependencyObject *target);
	void RemoveTarget (DependencyObject *target);

	virtual void OnPropertyChanged (DependencyProperty *prop);

	static DependencyProperty* RoutedEventProperty;
	static DependencyProperty* ActionsProperty;
};

EventTrigger *event_trigger_new (void);
void          event_trigger_action_add (EventTrigger *trigger, TriggerAction *action);
void          event_trigger_fire_actions (EventTrigger *trigger);

//
// Surface callbacks
//
typedef void (*callback_mouse_event)    (UIElement *target, int state, double x, double y);
typedef void (*callback_plain_event)    (UIElement *target);
typedef bool (*callback_keyboard_event) (UIElement *target, int state, int platformcode, int key);


//
// Item class
//
class UIElement : public Visual {
	Brush *opacityMask;
 public:
	UIElement ();
	~UIElement ();
	virtual Type::Kind GetObjectType () { return Type::UIELEMENT; };

	UIElement *parent;

	int dump_hierarchy (UIElement *obj);

	enum UIElementFlags {
		IS_CANVAS = 1,
		IS_LOADED = 2
	};
	
	int flags;

	// The computed bounding box
	double x1, y1, x2, y2;

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
	virtual void render (Surface *surface, int x, int y, int width, int height);

	// a non virtual method for use when we want to wrap render
	// with debugging and/or timing info
	void dorender (Surface *surface, int x, int y, int width, int height);
	
	//
	// get_size_for_brush:
	//   Gets the size of the area to be painted by a Brush (needed for image/video scaling)
	virtual void get_size_for_brush (cairo_t *cr, double *width, double *height);
	
	// 
	// getbounds:
	//   Updates the bounding box for the given item, this uses the parent
	//   chain to compute the composite affine.
	//
	// Output:
	//   the item->x1,y1,x2,y2 values are updated.
	// 
	virtual void getbounds ();

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
	//   might want it.   Returns true if the event was within this UIElement
	//   boundaries.
	//
	virtual bool handle_motion (Surface *s, int state, double x, double y);

	//
	// handle_button:
	//   handles the button press or button release events and dispatches
	//   it to all the objects that might be interested in it (nested
	//   objects).
	//
	//   Returns true if the button click was handled. 
	//
	virtual bool handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y);
	
	//
	// enter:
	//   Invoked when the mouse first enters this given object
	//
	virtual void enter (Surface *s, int state, double x, double y);
	
	//
	// leave:
	//   Invoke when the mouse leaves this given object
	//
	virtual void leave (Surface *s);

	//
	// GetTotalOpacity
	//   Get the cumulative opacity of this element, including all it's parents
	double GetTotalOpacity ();
	
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

double	 uielement_get_opacity     (UIElement *item);
void	 uielement_set_opacity     (UIElement *item, double opacity);

Brush   *uielement_get_opacity_mask (UIElement *item);

void     uielement_transform_point (UIElement *item, double *x, double *y);
	
//
// FrameworkElement class
//
class FrameworkElement : public UIElement {
 public:
	static DependencyProperty* HeightProperty;
	static DependencyProperty* WidthProperty;

	FrameworkElement ();
	virtual Type::Kind GetObjectType () { return Type::FRAMEWORKELEMENT; }

	void OnPropertyChanged (DependencyProperty *prop);

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
	Brush *background;
 public:
	Panel ();
	virtual ~Panel ();
	virtual Type::Kind GetObjectType () { return Type::PANEL; }

	VisualCollection *GetChildren ();
	void SetChildren (VisualCollection *col);

	static DependencyProperty* ChildrenProperty;
	static DependencyProperty* BackgroundProperty;

	virtual void OnPropertyChanged (DependencyProperty *prop);
};

// For C API usage.
void  panel_child_add      (Panel *panel, UIElement *item);
Panel *panel_new (void);
Brush *panel_get_background (Panel *panel);

//
// Canvas Class, the only purpose is to have the Left/Top properties that
// children can use
//
class Canvas : public Panel {
 public:
	Canvas ();

	//
	// if not-null, this is a toplevel canvas, and this points to the
	// surface
	//
	Surface *surface;

	//
	// Contains the last element where the mouse entered
	//
	UIElement *current_item;
	
	virtual Type::Kind GetObjectType () { return Type::CANVAS; }

	virtual Point getxformorigin () { return Point (0, 0); }

	virtual void render (Surface *s, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual void update_xform ();
	virtual void get_xform_for (UIElement *item, cairo_matrix_t *result);
	virtual bool handle_motion (Surface *s, int state, double x, double y);
	virtual bool handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y);
	virtual void leave (Surface *s);
	
	virtual bool OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child);
	
	static DependencyProperty* TopProperty;
	static DependencyProperty* LeftProperty;
};

Canvas *canvas_new (void);

//
// Control Class
//
class Control : public FrameworkElement {
 public:
	FrameworkElement *real_object;
	
	Control () : real_object (NULL) { };
	~Control ();
	
	virtual Type::Kind GetObjectType () { return Type::CONTROL; }
	virtual void update_xform ();
	virtual void render (Surface *surface, int x, int y, int width, int height);
	virtual void getbounds ();
	virtual void get_xform_for (UIElement *item, cairo_matrix_t *result);
	virtual Point getxformorigin ();
	virtual bool inside_object (Surface *s, double x, double y);
	virtual bool handle_motion (Surface *s, int state, double x, double y);
	virtual bool handle_button (Surface *s, callback_mouse_event cb, int state, double x, double y);
	virtual void enter (Surface *s, int state, double x, double y);
	virtual void leave (Surface *s);

	virtual void OnPropertyChanged (DependencyProperty *prop);
	virtual void OnSubPropertyChanged (DependencyProperty *prop, DependencyProperty *subprop);
	virtual bool OnChildPropertyChanged (DependencyProperty *prop, DependencyObject *child);

};

Control *control_new (void);
UIElement* control_initialize_from_xaml (Control *control, const char *xaml, Type::Kind *element_type);

typedef struct _SurfacePrivate SurfacePrivate;

class Surface {
 public:
	Surface () : width (0), height (0), buffer (0), 
		cairo_buffer_surface (NULL), cairo_buffer(NULL),
		xlib_surface(NULL), cairo_xlib(NULL), pixmap(NULL),
		using_cairo_xlib_surface(0), pixbuf(NULL),
		cb_motion(NULL), cb_down(NULL), cb_up(NULL), cb_enter(NULL),
		cb_got_focus(NULL), cb_lost_focus(NULL), cb_loaded(NULL), cb_mouse_leave(NULL), cb_surface_resize(NULL),
		cb_keydown(NULL), cb_keyup(NULL),
		cairo (NULL) {}
	
	~Surface();

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
	callback_plain_event cb_got_focus, cb_lost_focus, cb_loaded, cb_mouse_leave, cb_surface_resize;
	callback_keyboard_event cb_keydown, cb_keyup;

};

Surface *surface_new       (int width, int height);
void     surface_resize    (Surface *s, int width, int height);
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
				  callback_plain_event surface_resized,
				  callback_keyboard_event keydown, callback_keyboard_event keyup);
		      

//
// XAML
//

typedef DependencyObject *xaml_create_custom_element_callback (const char *xmlns, const char *name);
typedef void xaml_set_custom_attribute_callback (void *target, const char *name, const char *value);

UIElement  *xaml_create_from_file (const char *xaml, bool create_namescope, xaml_create_custom_element_callback *cecb,
		xaml_set_custom_attribute_callback *sca, Type::Kind *element_type);
UIElement  *xaml_create_from_str  (const char *xaml, bool create_namescope, xaml_create_custom_element_callback *cecb,
		xaml_set_custom_attribute_callback *sca, Type::Kind *element_type);
void       xaml_set_property_from_str (DependencyObject *obj, const char *prop, const char *value);


void runtime_init (void);
void animation_init (void);
void brush_init (void);
void clock_init (void);
void transform_init (void);
void shape_init (void);
void geometry_init (void);
void xaml_init (void);
void types_init (void);
void dependencyobject_init (void);
void downloader_init (void);
void media_init (void);
void dependency_object_init(void);

void runtime_shutdown (void);

G_END_DECLS

#endif
