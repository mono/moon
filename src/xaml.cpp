/*
 * xaml.cpp: xaml parser
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <string.h>
#include <gtk/gtk.h>
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <expat.h>

#include "runtime.h"
#include "shape.h"
#include "animation.h"
#include "geometry.h"

#define READ_BUFFER 1024

extern NameScope *global_NameScope;

GHashTable *element_map = NULL;

class XamlElementInfo;
class XamlElementInstance;
class XamlParserInfo;


typedef void* (*create_item_func) ();
typedef XamlElementInstance *(*create_element_instance_func) (XamlParserInfo *p, XamlElementInfo *i);
typedef void  (*add_child_func) (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child);
typedef void  (*set_property_func) (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);
typedef void  (*set_attributes_func) (XamlParserInfo *p, XamlElementInstance *item, const char **attr);


class XamlElementInstance {

 public:
	const char *element_name;
	const char *instance_name;

	XamlElementInfo *info;
	XamlElementInstance *parent;
	GList *children;

	enum ElementType {
		ELEMENT,
		PROPERTY,
		UNKNOWN
	};

	int element_type;
	void *item;


	XamlElementInstance (XamlElementInfo *info) : info (info), element_name (NULL), instance_name (NULL),
			     parent (NULL), children (NULL), element_type (UNKNOWN)
	{
	}

};

class XamlParserInfo {

 public:
	XML_Parser parser;

	XamlElementInstance *top_element;
	Value::Kind          top_kind;
	XamlElementInstance *current_element;

	GString *char_data_buffer;

	int state;
	
	XamlParserInfo (XML_Parser parser) : parser (parser), top_element (NULL),
					     current_element (NULL), char_data_buffer (NULL), top_kind(Value::INVALID)
	{
	}

};

class XamlElementInfo {

 public:
	const char *name;
	XamlElementInfo *parent;
	Value::Kind dependency_type;

	create_item_func create_item;
	create_element_instance_func create_element;
	add_child_func add_child;
	set_property_func set_property;
	set_attributes_func set_attributes;

	XamlElementInfo (const char *name, XamlElementInfo *parent, Value::Kind dependency_type) :
		name (name), parent (parent), dependency_type (dependency_type),
		create_item (NULL), create_element (NULL), add_child (NULL), set_property (NULL), set_attributes (NULL)
	{

	}

};


void
start_element_handler (void *data, const char *el, const char **attr)
{
	XamlParserInfo *p = (XamlParserInfo *) data;
	XamlElementInfo *elem;
	XamlElementInstance *inst;

	elem = (XamlElementInfo *) g_hash_table_lookup (element_map, el);

	if (elem) {

		inst = elem->create_element (p, elem);
		elem->set_attributes (p, inst, attr);

		if (!p->top_element) {
			p->top_element = inst;
			p->top_kind = elem->dependency_type;
			p->current_element = inst;
			return;
		}

		if (p->current_element && p->current_element->element_type == XamlElementInstance::ELEMENT) {
			p->current_element->info->add_child (p, p->current_element, inst);
		}

	} else {

		bool property = false;
		for (int i = 0; el [i]; i++) {
			if (el [i] != '.')
				continue;
			property = true;
			break;
		}

		if (property) {
			inst = new XamlElementInstance (NULL);
			inst->element_name = g_strdup (el);
			inst->element_type = XamlElementInstance::PROPERTY;
		} else {
			inst = new XamlElementInstance (NULL);
			inst->element_name = g_strdup (el);
			inst->element_type = XamlElementInstance::UNKNOWN;
		}

	}

	
	inst->parent = p->current_element;
	p->current_element->children = g_list_append (p->current_element->children, inst);
	p->current_element = inst;	
}


void
end_element_handler (void *data, const char *el)
{
	XamlParserInfo *info = (XamlParserInfo *) data;

	switch (info->current_element->element_type) {
	case XamlElementInstance::ELEMENT:
		if (info->char_data_buffer && info->char_data_buffer->len) {
			/*
			 * TODO: set content property
			 - Make sure we aren't just white space
			 info->current_element->set_content_prop (info->char_data_buffer->str);
			*/
		
			g_string_free (info->char_data_buffer, FALSE);
			info->char_data_buffer = NULL;
		}
		break;
	case XamlElementInstance::PROPERTY:

		GList *walk = info->current_element->children;
		while (walk) {
			XamlElementInstance *child = (XamlElementInstance *) walk->data;
			info->current_element->parent->info->set_property (info, info->current_element->parent, info->current_element, child);
			walk = walk->next;
		}

		break;
	}

	info->current_element = info->current_element->parent;
}

void
char_data_handler (void *data, const char *txt, int len)
{
	XamlParserInfo *info = (XamlParserInfo *) data;

	if (info->char_data_buffer == NULL) {
		info->char_data_buffer = g_string_new_len (txt, len);
		return;
	}

	info->char_data_buffer = g_string_append_len (info->char_data_buffer, txt, len);
}

void
free_recursive (XamlElementInstance *el)
{
	
}

void
print_tree (XamlElementInstance *el, int depth)
{
	for (int i = 0; i < depth; i++)
		printf ("\t");
	printf ("%s  (%d)\n", el->element_name, el->element_type);

	for (GList *walk = el->children; walk != NULL; walk = walk->next) {
		XamlElementInstance *el = (XamlElementInstance *) walk->data;
		print_tree (el, depth + 1);
	}
}

UIElement *
xaml_create_from_file (const char *xaml_file, Value::Kind *element_type)
{
	FILE *fp;
	char buffer [READ_BUFFER];
	int len, done;
	XamlParserInfo *parser_info;

	fp = fopen (xaml_file, "r+");

	if (!fp) {
#ifdef DEBUG_XAML
		printf ("can not open file\n");
#endif
		return NULL;
	}

	XML_Parser p = XML_ParserCreate (NULL);

	if (!p) {
#ifdef DEBUG_XAML
		printf ("can not create parser\n");
#endif
		fclose (fp);
		return NULL;
	}

	parser_info = new XamlParserInfo (p);
	XML_SetUserData (p, parser_info);

	XML_SetElementHandler (p, start_element_handler, end_element_handler);
	XML_SetCharacterDataHandler (p, char_data_handler);

	/*
	XML_SetProcessingInstructionHandler (p, proc_handler);
	*/

	done = 0;
	while (!done) {
		len = fread (buffer, 1, READ_BUFFER, fp);
		done = feof (fp);
		if (!XML_Parse (p, buffer, len, done)) {
#ifdef DEBUG_XAML
			printf("Parse error at line %d:\n%s\n",
					XML_GetCurrentLineNumber (p),
					XML_ErrorString (XML_GetErrorCode (p)));
			printf ("can not parse xaml\n");
#endif
			fclose (fp);
			return NULL;
		}
	}

#ifdef DEBUG_XAML
	print_tree (parser_info->top_element, 0);
#endif

	if (parser_info->top_element) {
		UIElement *res = (UIElement *) parser_info->top_element->item;
		if (element_type)
			*element_type = parser_info->top_kind;
		free_recursive (parser_info->top_element);
		return res;
	}

	return NULL;
}

UIElement *
xaml_create_from_str (const char *xaml, Value::Kind *element_type)
{
	XML_Parser p = XML_ParserCreate (NULL);
	XamlParserInfo *parser_info;
	
	if (!p) {
#ifdef DEBUG_XAML
		printf ("can not create parser\n");
#endif
		return NULL;
	}

	parser_info = new XamlParserInfo (p);
	XML_SetUserData (p, parser_info);

	XML_SetElementHandler (p, start_element_handler, end_element_handler);
	XML_SetCharacterDataHandler (p, char_data_handler);

	/*
	XML_SetProcessingInstructionHandler (p, proc_handler);
	*/


	if (!XML_Parse (p, xaml, strlen (xaml), TRUE)) {
#ifdef DEBUG_XAML
		printf("Parse error at line %d:\n%s\n",
				XML_GetCurrentLineNumber (p),
				XML_ErrorString (XML_GetErrorCode (p)));
		printf ("can not parse xaml\n");
#endif
		return NULL;
	}

#ifdef DEBUG_XAML
	print_tree (parser_info->top_element, 0);
#endif

	if (parser_info->top_element) {
		UIElement *res = (UIElement *) parser_info->top_element->item;
		if (element_type)
			*element_type = parser_info->top_kind;

		free_recursive (parser_info->top_element);
		return res;
	}

	return NULL;
}


RepeatBehavior
repeat_behavior_from_str (const char *str)
{
	if (!g_strcasecmp ("Forever", str))
		return RepeatBehavior::Forever;
	// It's late at night and I don't want to parse timespans
	return RepeatBehavior (strtod (str, NULL));
}

Duration
duration_from_str (const char *str)
{
	if (!g_strcasecmp ("Automatic", str))
		return Duration::Automatic;
	if (!g_strcasecmp ("Forever", str))
		return Duration::Forever;
	return Duration::FromSeconds (strtod (str, NULL));
}

XamlElementInstance *
default_create_element_instance (XamlParserInfo *p, XamlElementInfo *i)
{
	XamlElementInstance *inst = new XamlElementInstance (i);

	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;
	inst->item = i->create_item ();

	return inst;
}

XamlElementInstance *
create_event_trigger_instance (XamlParserInfo *p, XamlElementInfo *i)
{
	XamlElementInstance *inst = new XamlElementInstance (i);
	EventTrigger *trigger = (EventTrigger *) i->create_item ();
	
	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;
	inst->item = trigger;

	// this is pretty evil, once i work out some of the other issues
	// in the parser, i will add a more clean way to do this
	// probably a method that is called to notify an element that it
	// was added to a property
	XamlElementInstance *prop = p->current_element;
	XamlElementInstance *owner = prop->parent;

	trigger->SetTarget ((DependencyObject *) owner->item);

	return inst;
}

///
/// Add Child funcs
///

void
nonpanel_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	/// should we raise an error here????

}

void
panel_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	panel_child_add ((Panel *) parent->item, (UIElement *) child->item);
}

void
event_trigger_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	event_trigger_action_add ((EventTrigger *) parent->item, (TriggerAction *) child->item);
}

void
begin_storyboard_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	BeginStoryboard *bsb = (BeginStoryboard *) parent->item;
	Storyboard *sb = (Storyboard *) child->item;

	bsb->SetStoryboard (sb);
}

void
storyboard_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	Storyboard *sb = (Storyboard *) parent->item;
	Timeline *t = (Timeline *) child->item;

	sb->AddChild (t);
}

		
///
/// set property funcs
///

// these are just a bunch of special cases
void
dependency_object_missed_property (XamlElementInstance *item, XamlElementInstance *prop, XamlElementInstance *value, char **prop_name)
{
	if (!strcmp ("Triggers", prop_name [1])) {

		// Ensure that we are dealing with a framework element
		bool is_fwe = false;
		for (XamlElementInfo *walk = item->info; walk; walk = walk->parent) {
			if (walk->dependency_type != Value::FRAMEWORKELEMENT)
				continue;
			is_fwe = true;
			break;
		}

		if (is_fwe)
			framework_element_trigger_add ((FrameworkElement *) item->item, (EventTrigger *) value->item);
	}
}

void
dependency_object_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value)
{
	char **prop_name = g_strsplit (property->element_name, ".", -1);

	DependencyObject *dep = (DependencyObject *) item->item;
	DependencyProperty *prop = NULL;
	XamlElementInfo *walk = item->info;
	while (walk) {
		prop = DependencyObject::GetDependencyProperty (walk->dependency_type, prop_name [1]);
		if (prop)
			break;
		walk = walk->parent;
	}

	if (prop) {
		if (prop->value_type >= Value::DEPENDENCY_OBJECT) {
			dep->SetValue (prop, Value ((DependencyObject *) value->item));
		}
	} else {
		dependency_object_missed_property (item, property, value, prop_name);
	}

	g_strfreev (prop_name);
}

///
/// set attributes funcs
///

void
dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	DependencyObject *dep = (DependencyObject *) item->item;

	for (int i = 0; attr [i]; i += 2) {

		if (!strcmp ("x:Name", attr [i])) {
			global_NameScope->RegisterName (attr [i + 1], dep);
			continue;
		}

		const char *pname = attr [i];
		char *atchname = NULL;
		for (int a = 0; attr [i][a]; a++) {
			if (attr [i][a] != '.')
				continue;
			atchname = g_strndup (attr [i], a);
			pname = attr [i] + a + 1;
			break;
		}

		DependencyProperty *prop = NULL;
		XamlElementInfo *walk = item->info;

		if (atchname) {
			XamlElementInfo *attached = (XamlElementInfo *) g_hash_table_lookup (element_map, atchname);
			walk = attached;
		}

		while (walk) {
			prop = DependencyObject::GetDependencyProperty (walk->dependency_type, (char *) pname);
			if (prop)
				break;
			walk = walk->parent;
		}

		if (prop) {

			switch (prop->value_type) {
			case Value::BOOL:
				dep->SetValue (prop, Value ((bool) !g_strcasecmp ("true", attr [i + 1])));
				break;
			case Value::DOUBLE:
				dep->SetValue (prop, Value ((double) strtod (attr [i + 1], NULL)));
				break;
			case Value::INT64:
				dep->SetValue (prop, Value ((gint64) strtol (attr [i + 1], NULL, 10)));
				break;
			case Value::INT32:
				dep->SetValue (prop, Value ((int) strtol (attr [i + 1], NULL, 10)));
				break;
			case Value::STRING:
				dep->SetValue (prop, Value (attr [i + 1]));
				break;
			case Value::COLOR:
				dep->SetValue (prop, Value (*color_from_str (attr [i + 1])));
				break;
			case Value::REPEATBEHAVIOR:
				dep->SetValue (prop, Value (repeat_behavior_from_str (attr [i + 1])));
				break;
			case Value::DURATION:
				dep->SetValue (prop, Value (duration_from_str (attr [i + 1])));
				break;
			case Value::BRUSH:
			case Value::SOLIDCOLORBRUSH:
			{
				// Only solid color brushes can be specified using attribute syntax
				SolidColorBrush *scb = solid_color_brush_new ();
				solid_color_brush_set_color (scb, color_from_str (attr [i + 1]));
				dep->SetValue (prop, Value (scb));
			}
				break;
			case Value::POINT:
				dep->SetValue (prop, Value (point_from_str (attr [i + 1])));
				break;
			case Value::RECT:
				dep->SetValue (prop, Value (rect_from_str (attr [i + 1])));
				break;
			default:
#ifdef DEBUG_XAML
				printf ("could not find value type for: %s::%s %d\n", pname, attr [i + 1], prop->value_type);
#endif
				continue;
			}

		} else {
#ifdef DEBUG_XAML
			printf ("can not find property:  %s  %s\n", pname, attr [i + 1]);
#endif
		}

		if (atchname)
			g_free (atchname);
	}
}

void
event_trigger_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	EventTrigger *et = (EventTrigger *) item->item;

	for (int i = 0; attr [i]; i += 2) {
		if (!strcmp (attr [i], "RoutedEvent")) {
			et->routed_event = g_strdup (attr [i + 1]);
		}
	}

	dependency_object_set_attributes (p, item, attr);
}

// We still use a name for ghost elements to make debugging easier
XamlElementInfo *
register_ghost_element (const char *name, XamlElementInfo *parent, Value::Kind dt)
{
	return new XamlElementInfo (name, parent, dt);
}

XamlElementInfo *
register_dependency_object_element (const char *name, XamlElementInfo *parent, Value::Kind dt,
		create_item_func create_item)
{
	XamlElementInfo *res = new XamlElementInfo (name, parent, dt);

	res->create_item = create_item;
	res->create_element = default_create_element_instance;
	res->add_child = nonpanel_add_child;
	res->set_property = dependency_object_set_property;
	res->set_attributes = dependency_object_set_attributes;

	g_hash_table_insert (element_map, (char *) name, GINT_TO_POINTER (res));

	return res;
}

XamlElementInfo *
register_element_full (const char *name, XamlElementInfo *parent, Value::Kind dt,
		create_item_func create_item, create_element_instance_func create_element, add_child_func add_child,
		set_property_func set_property, set_attributes_func set_attributes)
{
	XamlElementInfo *res = new XamlElementInfo (name, parent, dt);

	res->create_item = create_item;
	res->create_element = create_element;
	res->add_child = add_child;
	res->set_property = set_property;
	res->set_attributes = set_attributes;

	g_hash_table_insert (element_map, (char *) name, GINT_TO_POINTER (res));

	return res;
}

void
xaml_init ()
{
	element_map = g_hash_table_new (g_str_hash, g_str_equal);

	//
	// ui element ->
	//
	XamlElementInfo *ui = register_ghost_element ("UIElement", NULL, Value::UIELEMENT);
	XamlElementInfo *fw = register_ghost_element ("FrameworkElement", ui, Value::FRAMEWORKELEMENT);
	XamlElementInfo *shape = register_ghost_element ("Shape", fw, Value::SHAPE);

	
	///
	/// Shapes
	///
	
	register_dependency_object_element ("Ellipse", shape, Value::ELLIPSE, (create_item_func) ellipse_new);
	register_dependency_object_element ("Line", shape, Value::LINE, (create_item_func) line_new);
	register_dependency_object_element ("Path", shape, Value::PATH, (create_item_func) path_new);
	register_dependency_object_element ("Polygon", shape, Value::POLYGON, (create_item_func) polygon_new);
	register_dependency_object_element ("Polyline", shape, Value::POLYLINE, (create_item_func) polyline_new);
	register_dependency_object_element ("Rectangle", shape, Value::RECTANGLE, (create_item_func) rectangle_new);

	///
	/// Geometry
	///

	XamlElementInfo *geo = register_ghost_element ("Geometry", NULL, Value::GEOMETRY);
	register_dependency_object_element ("GeometryGroup", geo, Value::GEOMETRYGROUP, (create_item_func) geometry_group_new);
	register_dependency_object_element ("EllipseGeometry", geo, Value::ELLIPSEGEOMETRY, (create_item_func) ellipse_geometry_new);
//	register_dependency_object_element ("CombinedGeometry", geo, Value::COMBINEDGEOMETRY, (create_item_func) combined_geometry_new);
	register_dependency_object_element ("LineGeometry", geo, Value::LINEGEOMETRY, (create_item_func) line_geometry_new);
	register_dependency_object_element ("PathGeometry", geo, Value::PATHGEOMETRY, (create_item_func) path_geometry_new);
	register_dependency_object_element ("RectangleGeometry", geo, Value::RECTANGLEGEOMETRY, (create_item_func) rectangle_geometry_new);
//	register_dependency_object_element ("StreamGeometry", geo, Value::STREAMGEOMETRY, (create_item_func) stream_geometry_new);


	///
	/// Panels
	///
	
	XamlElementInfo *panel = register_ghost_element ("Panel", fw, Value::PANEL);
	XamlElementInfo *canvas = register_dependency_object_element ("Canvas", panel, Value::CANVAS, (create_item_func) canvas_new);
	canvas->add_child = panel_add_child;


	///
	/// Animation
	///
	
	XamlElementInfo *tl = register_ghost_element ("Timeline", NULL, Value::TIMELINE);
	register_dependency_object_element ("DoubleAnimation", tl, Value::DOUBLEANIMATION, (create_item_func) double_animation_new);
	XamlElementInfo *sb = register_dependency_object_element ("Storyboard", tl, Value::STORYBOARD, (create_item_func) storyboard_new);
	sb->add_child = storyboard_add_child;


	///
	/// Triggers
	///
	XamlElementInfo *trg = register_ghost_element ("Trigger", NULL, Value::TRIGGERACTION);
	XamlElementInfo *bsb = register_dependency_object_element ("BeginStoryboard", trg, Value::BEGINSTORYBOARD,
			(create_item_func) begin_storyboard_new);
	bsb->add_child = begin_storyboard_add_child;

	register_element_full ("EventTrigger", NULL, Value::EVENTTRIGGER, (create_item_func) event_trigger_new,
			create_event_trigger_instance, event_trigger_add_child, dependency_object_set_property,
			event_trigger_set_attributes);

	///
	/// Transforms
	///
	
	register_dependency_object_element ("RotateTransform", NULL, Value::ROTATETRANSFORM, (create_item_func) rotate_transform_new);
	register_dependency_object_element ("ScaleTransform", NULL, Value::SCALETRANSFORM, (create_item_func) scale_transform_new);
	register_dependency_object_element ("TranslateTransform", NULL, Value::TRANSLATETRANSFORM, (create_item_func) translate_transform_new);
	register_dependency_object_element ("MatrixTransform", NULL, Value::MATRIXTRANSFORM, (create_item_func) matrix_transform_new);


	///
	/// Brushes
	///

	XamlElementInfo *brush = register_ghost_element ("Brush", NULL, Value::BRUSH);
	register_dependency_object_element ("SolidColorBrush", brush, Value::SOLIDCOLORBRUSH, (create_item_func) solid_color_brush_new);

}
