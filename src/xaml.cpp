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
		printf ("Property: %s\n", el);
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

bool
is_instance_of (XamlElementInstance *item, Value::Kind kind)
{
	for (XamlElementInfo *walk = item->info; walk; walk = walk->parent) {
		if (walk->dependency_type == kind)
			return true;
	}

	return false;
}

guint64
timespan_from_str (const char *str)    
{
	char *next = NULL;
	guint64 res = 0;
	bool negative = false;
	int digit;
	int digits [5] = { 0, 0, 0, 0, 0 };
	int di = 0;

	digit = strtol (str, &next, 10);

	if (!next)
		return digit * 36000000;

	if (next [0] == '.') {
		digits [0] = digit;
		di = 1;
	} else {
		digits [1] = digit;
		di = 2;
	}

	next++;
	while (next && di < 5) {
		int d =  strtol (next, &next, 10);
		digits [di++] = d;
		if (next)
			next++;
	}

// 	printf ("%d.%d:%d:%d.%d\n", digits [0], digits [1], digits [2], digits [3], digits [4]);  

	// Convert to seconds, then to ticks
	// TODO: This could overflow?
	res = ((digits [0] * 86400) + (digits [1] * 3600) + (digits [2] * 60) + digits [3]);
	res *= 10000L;
	res += digits [4];

	return res;
}

RepeatBehavior
repeat_behavior_from_str (const char *str)
{
	if (!g_strcasecmp ("Forever", str))
		return RepeatBehavior::Forever;
	return RepeatBehavior (strtod (str, NULL));
}

Duration
duration_from_str (const char *str)
{
	if (!g_strcasecmp ("Automatic", str))
		return Duration::Automatic;
	if (!g_strcasecmp ("Forever", str))
		return Duration::Forever;
	return Duration (timespan_from_str (str));
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

XamlElementInstance *
create_path_figure_collection_instance (XamlParserInfo *p, XamlElementInfo *i)
{
	XamlElementInstance *inst = new XamlElementInstance (i);

	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;

	// Walk up the tree and find the PathGeometry that we are being added to,
	// attempt to use this instance as our item, if the types are incorrect,
	// create a new item
	XamlElementInstance *prop = p->current_element;

	if (prop) {
		XamlElementInstance *owner = prop->parent;
		if (owner && is_instance_of (owner, Value::PATHGEOMETRY))
			inst->item = ((PathGeometry *) owner->item)->children;
	}

	// This is still legal, because someone could have done:
	// createFromXaml ("<PathFigureCollection ...");
	if (!inst->item)
		inst->item = new PathFigureCollection ();

	return inst;
}

XamlElementInstance *
create_path_segment_collection_instance (XamlParserInfo *p, XamlElementInfo *i)
{
	XamlElementInstance *inst = new XamlElementInstance (i);

	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;

	// Walk up the tree and find the PathFigure that we are being added to,
	// attempt to use this instance as our item, if the types are incorrect,
	// create a new item
	XamlElementInstance *prop = p->current_element;

	if (prop) {
		XamlElementInstance *owner = prop->parent;
		if (owner && is_instance_of (owner, Value::PATHFIGURE))
			inst->item = ((PathFigure *) owner->item)->children;
	}

	// This is still legal, because someone could have done:
	// createFromXaml ("<PathSegmentCollection ...");
	if (!inst->item)
		inst->item = new PathSegmentCollection ();

	return inst;
}

XamlElementInstance *
create_geometry_collection_instance (XamlParserInfo *p, XamlElementInfo *i)
{
	XamlElementInstance *inst = new XamlElementInstance (i);

	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;

	// Walk up the tree and find the GeometryGroup that we are being added to,
	// attempt to use this instance as our item, if the types are incorrect,
	// create a new item
	XamlElementInstance *prop = p->current_element;

	if (prop) {

		if (is_instance_of (prop, Value::GEOMETRYGROUP)) {
			inst->item = ((GeometryGroup *) prop->item)->children;
		} else {
			XamlElementInstance *owner = prop->parent;
			if (owner && is_instance_of (owner, Value::GEOMETRYGROUP)) {
				inst->item = ((GeometryGroup *) owner->item)->children;
			}
		}
	}

	// This is still legal, because someone could have done:
	// createFromXaml ("<GeometryCollection ...");
	if (!inst->item) {
		printf ("creating a new geometry collection\n");
		inst->item = new GeometryCollection ();
	}

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
	if (!is_instance_of (child, Value::STORYBOARD)) {
		g_warning ("error, attempting to add non storyboard type (%d) to BeginStoryboard element\n",
				child->info->dependency_type);
		return;
	}

	BeginStoryboard *bsb = (BeginStoryboard *) parent->item;
	Storyboard *sb = (Storyboard *) child->item;

	bsb->SetStoryboard (sb);
}

void
storyboard_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	if (!is_instance_of (child, Value::TIMELINE)) {
		g_warning ("error, attempting to add non timeline type (%d) to Storyboard element\n",
				child->info->dependency_type);
		return;
	}

	Storyboard *sb = (Storyboard *) parent->item;
	Timeline *t = (Timeline *) child->item;

	sb->AddChild (t);
}

void
transform_group_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	TransformGroup *tg = (TransformGroup *) parent->item;

	if (is_instance_of (child, Value::TRANSFORM_COLLECTION)) {
		Value v ((TransformCollection *) child->item);
		tg->SetValue (tg->ChildrenProperty, &v);
		return;
	}

	if (!is_instance_of (child, Value::TRANSFORM)) {
		g_warning ("error, attempting to add non Transform type (%d) to TransformCollection element\n",
				child->info->dependency_type);
		return;
	}

	Transform *t = (Transform *) child->item;
	Value v = Value (t);

	tg->children->Add (&v);
}

void
transform_collection_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	if (!is_instance_of (child, Value::TRANSFORM)) {
		g_warning ("error, attempting to add non Transform type (%d) to TransformCollection element\n",
				child->info->dependency_type);
		return;
	}

	TransformCollection *tc = (TransformCollection *) parent->item;
	Transform *t = (Transform *) child->item;
	Value v = Value (t);

	tc->Add (&v);
}

void
geometry_group_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	GeometryGroup *gg = (GeometryGroup *) parent->item;

	// When creating the child collections we attempt to use the PathGeometry's Collection
	// in this case, the pointers will be equal, and we don't need to do anything
	if (gg->children == child->item)
		return;

	if (is_instance_of (child, Value::GEOMETRY_COLLECTION)) {
		Value v ((GeometryCollection *) child->item);
		gg->SetValue (gg->ChildrenProperty, &v);
		return;
	}

	if (!is_instance_of (child, Value::GEOMETRY)) {
		g_warning ("error, attempting to add non Geometry type (%s) to GeometryCollection element\n",
				child->element_name);
		return;
	}

	Geometry *g = (Geometry *) child->item;
	Value v = Value (g);

	gg->children->Add (&v);
}

void
path_geometry_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	PathGeometry *pg = (PathGeometry *) parent->item;

	// When creating the child collections we attempt to use the PathGeometry's Collection
	// in this case, the pointers will be equal, and we don't need to do anything
	if (pg->children == child->item)
		return;

	if (is_instance_of (child, Value::PATHFIGURE_COLLECTION)) {
		Value v ((PathFigureCollection *) child->item);
		pg->SetValue (pg->FiguresProperty, &v);
		return;
	}

	if (!is_instance_of (child, Value::PATHFIGURE)) {
		g_warning ("error, attempting to add non PathFigure type (%d) to PathFigureCollection element\n",
				child->info->dependency_type);
		return;
	}

	PathFigure *pf = (PathFigure *) child->item;
	Value v = Value (pf);

	pg->children->Add (&v);
}

void
path_segment_collection_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	PathSegmentCollection *psc = (PathSegmentCollection *) parent->item;

	if (!is_instance_of (child, Value::PATHSEGMENT)) {
		g_warning ("error, attempting to add non PathSegment type (%d) to PathSegmentCollection element\n",
				child->info->dependency_type);
		return;
	}

	PathSegment *ps = (PathSegment *) child->item;
	Value v = Value (ps);

	psc->Add (&v);
}


void
path_figure_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	PathFigure *pf = (PathFigure *) parent->item;

	// When creating the child collections we attempt to use the PathGeometry's Collection
	// in this case, the pointers will be equal, and we don't need to do anything
	if (pf->children == child->item) {
		printf ("path figure got an existing children collection\n");
		return;
	}

	if (is_instance_of (child, Value::PATHSEGMENT_COLLECTION)) {
		Value v ((PathSegmentCollection *) child->item);
		pf->SetValue (pf->SegmentsProperty, &v);
		return;
	}

	if (!is_instance_of (child, Value::PATHSEGMENT)) {
		g_warning ("error, attempting to add non PathSegment type (%d) to PathFigureCollection element\n",
				child->info->dependency_type);
		return;
	}

	PathSegment *ps = (PathSegment *) child->item;
	Value v = Value (ps);

	pf->children->Add (&v);
}

void
path_figure_collection_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	if (!is_instance_of (child, Value::PATHFIGURE)) {
		g_warning ("error, attempting to add non PathFigure type (%d) to PathFigureCollection element\n",
				child->info->dependency_type);
		return;
	}

	PathFigureCollection *pfc = (PathFigureCollection *) parent->item;
	PathFigure *pf = (PathFigure *) child->item;
	Value v = Value (pf);

	pfc->Add (&v);
}

void
geometry_collection_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	if (!is_instance_of (child, Value::GEOMETRY)) {
		g_warning ("error, attempting to add non PathFigure type (%d) to PathFigureCollection element\n",
				child->info->dependency_type);
		return;
	}

	GeometryCollection *gc = (GeometryCollection *) parent->item;
	Geometry *geo = (Geometry *) child->item;
	Value v = Value (geo);

	gc->Add (&v);
}

///
/// set property funcs
///

// these are just a bunch of special cases
void
dependency_object_missed_property (XamlElementInstance *item, XamlElementInstance *prop, XamlElementInstance *value, char **prop_name)
{
	if (!strcmp ("Triggers", prop_name [1])) {
		if (is_instance_of (item, Value::TIMELINE) && is_instance_of (value, Value::EVENTTRIGGER)) {
			framework_element_trigger_add ((FrameworkElement *) item->item, (EventTrigger *) value->item);
		}
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
			printf ("setting property:  %s\n", property->element_name);
			// an empty collection can be NULL and valid
			if (value->item)
				dep->SetValue (prop, Value ((DependencyObject *) value->item));
		}
	} else {
		dependency_object_missed_property (item, property, value, prop_name);
	}

	g_strfreev (prop_name);
}

void
path_figure_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value)
{
	if (!strcmp (property->element_name, "PathFigure.Segments")) {
		if (((PathFigure *) item->item)->children == value->item)
			printf ("collections are equal\n");
	} else {
		dependency_object_set_property (p, item, property, value);
	}
}

void
path_geometry_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value)
{
	if (!strcmp (property->element_name, "PathGeometry.Figures")) {
		if (((PathFigure *) item->item)->children == value->item)
			printf ("path geometry collections are equal\n");
	} else {
		dependency_object_set_property (p, item, property, value);
	}
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
			case Value::DOUBLE_ARRAY:
			{
				int count = 0;
				double *doubles = double_array_from_str (attr [i + 1], &count);
				dep->SetValue (prop, Value (doubles, count));
			}
				break;
			case Value::POINT_ARRAY:
			{
				int count = 0;
				Point *points = point_array_from_str (attr [i + 1], &count);
				dep->SetValue (prop, Value (points, count));
			}
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

	XamlElementInfo *col = register_ghost_element ("Collection", NULL, Value::COLLECTION);

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
	register_dependency_object_element ("EllipseGeometry", geo, Value::ELLIPSEGEOMETRY, (create_item_func) ellipse_geometry_new);
	register_dependency_object_element ("LineGeometry", geo, Value::LINEGEOMETRY, (create_item_func) line_geometry_new);
	register_dependency_object_element ("RectangleGeometry", geo, Value::RECTANGLEGEOMETRY, (create_item_func) rectangle_geometry_new);

	XamlElementInfo *gg = register_dependency_object_element ("GeometryGroup", geo, Value::GEOMETRYGROUP, (create_item_func) geometry_group_new);
	gg->add_child = geometry_group_add_child;
	XamlElementInfo *gc = register_dependency_object_element ("GeometryCollection", col, Value::GEOMETRYGROUP, (create_item_func) geometry_group_new);
	gc->create_element = create_geometry_collection_instance;
	gc->add_child = geometry_collection_add_child;

	XamlElementInfo *pg = register_dependency_object_element ("PathGeometry", geo, Value::PATHGEOMETRY, (create_item_func) path_geometry_new);
	pg->add_child = path_geometry_add_child;
	pg->set_property = path_geometry_set_property;

	XamlElementInfo *pfc = register_dependency_object_element ("PathFigureCollection", col, Value::PATHFIGURE_COLLECTION, (create_item_func) NULL);
	pfc->add_child = path_figure_collection_add_child;
	pfc->create_element = create_path_figure_collection_instance;

	XamlElementInfo *pf = register_dependency_object_element ("PathFigure", geo, Value::PATHFIGURE, (create_item_func) path_figure_new);
	pf->add_child = path_figure_add_child;
	pf->set_property = path_figure_set_property;

	XamlElementInfo *psc = register_dependency_object_element ("PathSegmentCollection", col, Value::PATHFIGURE, (create_item_func) path_figure_new);
	psc->create_element = create_path_segment_collection_instance;
	psc->add_child = path_segment_collection_add_child;

	XamlElementInfo *ps = register_ghost_element ("PathSegment", geo, Value::PATHSEGMENT);
	register_dependency_object_element ("ArcSegment", ps, Value::ARCSEGMENT, (create_item_func) arc_segment_new);
	register_dependency_object_element ("BezierSegment", ps, Value::BEZIERSEGMENT, (create_item_func) bezier_segment_new);
	register_dependency_object_element ("LineSegment", ps, Value::LINESEGMENT, (create_item_func) line_segment_new);
	register_dependency_object_element ("PolyBezierSegment", ps, Value::POLYBEZIERSEGMENT, (create_item_func) poly_bezier_segment_new);
	register_dependency_object_element ("PolyLineSegment", ps, Value::POLYLINESEGMENT, (create_item_func) poly_line_segment_new);
	register_dependency_object_element ("PolyQuadraticBezierSegment", ps, Value::POLYQUADRATICBEZIERSEGMENT, (create_item_func) poly_quadratic_bezier_segment_new);
	register_dependency_object_element ("QuadraticBezierSegment", ps, Value::QUADRATICBEZIERSEGMENT, (create_item_func) quadratic_bezier_segment_new);

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

	XamlElementInfo *tf = register_ghost_element ("Transform", NULL, Value::TRANSFORM);
	register_dependency_object_element ("RotateTransform", tf, Value::ROTATETRANSFORM, (create_item_func) rotate_transform_new);
	register_dependency_object_element ("ScaleTransform", tf, Value::SCALETRANSFORM, (create_item_func) scale_transform_new);
	register_dependency_object_element ("TranslateTransform", tf, Value::TRANSLATETRANSFORM, (create_item_func) translate_transform_new);
	register_dependency_object_element ("MatrixTransform", tf, Value::MATRIXTRANSFORM, (create_item_func) matrix_transform_new);
	XamlElementInfo *tg = register_dependency_object_element ("TransformGroup", tf, Value::TRANSFORMGROUP, (create_item_func) transform_group_new);
	tg->add_child = transform_group_add_child;
	
	XamlElementInfo *tfc = register_dependency_object_element ("TransformCollection", col, Value::TRANSFORMGROUP, (create_item_func) transform_group_new);
	tfc->add_child = transform_collection_add_child;


	///
	/// Brushes
	///

	XamlElementInfo *brush = register_ghost_element ("Brush", NULL, Value::BRUSH);
	register_dependency_object_element ("SolidColorBrush", brush, Value::SOLIDCOLORBRUSH, (create_item_func) solid_color_brush_new);

}
