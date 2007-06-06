
#define DEBUG_XAML

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
		PANEL,
		PROPERTY,
		UNKNOWN
	};

	int element_type;
	void *item;


	XamlElementInstance (XamlElementInfo *info) : info (info), element_name (NULL), instance_name (NULL),
			     parent (NULL), children (NULL), element_type (PANEL)
	{
	}

};

class XamlParserInfo {

 public:
	XML_Parser parser;

	XamlElementInstance *top_element;
	XamlElementInstance *current_element;

	GString *char_data_buffer;

	int state;
	
	XamlParserInfo (XML_Parser parser) : parser (parser), top_element (NULL),
					     current_element (NULL), char_data_buffer (NULL)
	{
	}

};

class XamlElementInfo {

 public:
	const char *name;
	XamlElementInfo *parent;
	DependencyObject::Type dependency_type;

	create_item_func create_item;
	create_element_instance_func create_element;
	add_child_func add_child;
	set_property_func set_property;
	set_attributes_func set_attributes;

	XamlElementInfo (const char *name, XamlElementInfo *parent, DependencyObject::Type dependency_type) :
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
	printf ("%s  (%d)\n", el->element_name, g_list_length (el->children));

	for (GList *walk = el->children; walk != NULL; walk = walk->next) {
		XamlElementInstance *el = (XamlElementInstance *) walk->data;
		print_tree (el, depth + 1);
	}
}

UIElement *
xaml_create_from_file (const char *xaml_file)
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
		free_recursive (parser_info->top_element);
		return res;
	}

	return NULL;
}

UIElement *
xaml_create_from_str (const char *xaml)
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
		free_recursive (parser_info->top_element);
		return res;
	}

	return NULL;
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


///
/// set property funcs
///

void
dependency_object_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value)
{
	char **prop_name = g_strsplit (property->element_name, ".", -1);

	DependencyProperty *prop = NULL;
	XamlElementInfo *walk = item->info;
	while (walk) {
		prop = DependencyObject::GetDependencyProperty (walk->dependency_type, prop_name [1]);
		if (prop)
			break;
		walk = walk->parent;
	}

	if (prop) {
		/* need to create Values from void*'s */
	}

	g_strfreev (prop_name);
}

void
solid_color_brush_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value)
{

}

///
/// set attributes funcs
///

void
default_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	
}

void
dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	DependencyObject *dep = (DependencyObject *) item->item;

	for (int i = 0; attr [i]; i++) {
		DependencyProperty *prop = NULL;
		XamlElementInfo *walk = item->info;
		while (walk) {
			prop = DependencyObject::GetDependencyProperty (walk->dependency_type, (char *) attr [i]);
			if (prop)
				break;
			walk = walk->parent;
		}

		if (prop) {
			/// This should be replaced soon, just stuck it in here for debugging
			Value v;
			switch (prop->value_type) {
			case Value::BOOL:
				v = Value ((bool) !g_strcasecmp ("true", attr [i + 1]));
				break;
			case Value::DOUBLE:
				v = Value ((double) strtod (attr [i + 1], NULL));
				break;
			case Value::INT64:
				v = Value ((gint64) strtol (attr [i + 1], NULL, 10));
				break;
			case Value::INT32:
				v = Value ((int) strtol (attr [i + 1], NULL, 10));
				break;
			case Value::STRING:
				v = Value (attr [i + 1]);
				break;
			default:
				continue;
			}
			dep->SetValue (prop, v);
		}
	}
}

// We still use a name for ghost elements to make debugging easier
XamlElementInfo *
register_ghost_element (const char *name, XamlElementInfo *parent, DependencyObject::Type dt)
{
	return new XamlElementInfo (name, parent, dt);
}

XamlElementInfo *
register_dependency_object_element (const char *name, XamlElementInfo *parent, DependencyObject::Type dt,
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
register_element_full (const char *name, XamlElementInfo *parent, DependencyObject::Type dt,
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
	XamlElementInfo *ui = register_ghost_element ("UIElement", NULL, DependencyObject::UIELEMENT);
	XamlElementInfo *fw = register_ghost_element ("FrameworkElement", ui, DependencyObject::FRAMEWORKELEMENT);
	XamlElementInfo *shape = register_ghost_element ("Shape", fw, DependencyObject::SHAPE);

	
	///
	/// Shapes
	///
	
	register_dependency_object_element ("Ellipse", shape, DependencyObject::ELLIPSE, (create_item_func) ellipse_new);
	register_dependency_object_element ("Line", shape, DependencyObject::LINE, (create_item_func) line_new);
	register_dependency_object_element ("Path", shape, DependencyObject::PATH, (create_item_func) path_new);
	register_dependency_object_element ("Polygon", shape, DependencyObject::POLYGON, (create_item_func) polygon_new);
	register_dependency_object_element ("Polyline", shape, DependencyObject::POLYLINE, (create_item_func) polyline_new);
	register_dependency_object_element ("Rectangle", shape, DependencyObject::RECTANGLE, (create_item_func) rectangle_new);

	///
	/// Geometry
	///

	XamlElementInfo *geo = register_ghost_element ("Geometry", NULL, DependencyObject::GEOMETRY);
	register_dependency_object_element ("GeometryGroup", geo, DependencyObject::GEOMETRYGROUP, (create_item_func) geometry_group_new);
	register_dependency_object_element ("EllipseGeometry", geo, DependencyObject::ELLIPSEGEOMETRY, (create_item_func) ellipse_geometry_new);
//	register_dependency_object_element ("CombinedGeometry", geo, DependencyObject::COMBINEDGEOMETRY, (create_item_func) combined_geometry_new);
	register_dependency_object_element ("LineGeometry", geo, DependencyObject::LINEGEOMETRY, (create_item_func) line_geometry_new);
	register_dependency_object_element ("PathGeometry", geo, DependencyObject::PATHGEOMETRY, (create_item_func) path_geometry_new);
	register_dependency_object_element ("RectangleGeometry", geo, DependencyObject::RECTANGLEGEOMETRY, (create_item_func) rectangle_geometry_new);
//	register_dependency_object_element ("StreamGeometry", geo, DependencyObject::STREAMGEOMETRY, (create_item_func) stream_geometry_new);


	///
	/// Panels
	///
	
	XamlElementInfo *panel = register_ghost_element ("Panel", fw, DependencyObject::PANEL);
	XamlElementInfo *canvas = register_dependency_object_element ("Canvas", panel, DependencyObject::CANVAS, (create_item_func) canvas_new);
	canvas->add_child = panel_add_child;


	///
	/// Animation
	///
	
	XamlElementInfo *tl = register_ghost_element ("Timeline", NULL, DependencyObject::TIMELINE);
	register_dependency_object_element ("DoubleAnimation", tl, DependencyObject::DOUBLEANIMATION, (create_item_func) double_animation_new);
	register_dependency_object_element ("StoryBoard", tl, DependencyObject::STORYBOARD, (create_item_func) storyboard_new);


	///
	/// Transforms
	///
	
	register_dependency_object_element ("RotateTransform", NULL, DependencyObject::ROTATETRANSFORM, (create_item_func) rotate_transform_new);
	register_dependency_object_element ("ScaleTransform", NULL, DependencyObject::SCALETRANSFORM, (create_item_func) scale_transform_new);
	register_dependency_object_element ("TranslateTransform", NULL, DependencyObject::TRANSLATETRANSFORM, (create_item_func) translate_transform_new);
	register_dependency_object_element ("MatrixTransform", NULL, DependencyObject::MATRIXTRANSFORM, (create_item_func) matrix_transform_new);


	///
	/// Brushes
	///

	XamlElementInfo *brush = register_ghost_element ("Brush", NULL, DependencyObject::BRUSH);
	register_dependency_object_element ("SolidColorBrush", brush, DependencyObject::SOLIDCOLORBRUSH, (create_item_func) solid_color_brush_new);
}
