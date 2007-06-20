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
#include <config.h>
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
#include "text.h"
#include "media.h"
#include "list.h"

#define READ_BUFFER 1024

GHashTable *namespace_map = NULL;
GHashTable *enum_map = NULL;

class XamlElementInfo;
class XamlElementInstance;
class XamlParserInfo;
class XamlNamespace;
class DefaultNamespace;
class XNamespace;


DefaultNamespace *default_namespace = NULL;
XNamespace *x_namespace = NULL;


typedef DependencyObject *(*create_item_func) (void);
typedef XamlElementInstance *(*create_element_instance_func) (XamlParserInfo *p, XamlElementInfo *i);
typedef void  (*add_child_func) (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child);
typedef void  (*set_property_func) (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);
typedef void  (*set_attributes_func) (XamlParserInfo *p, XamlElementInstance *item, const char **attr);

void dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr);
void parser_error (XamlParserInfo *p, const char *el, const char *attr, const char *message);


XamlElementInstance *create_custom_element (XamlParserInfo *p, XamlElementInfo *i);
void  set_custom_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr);
void  custom_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child);
void  custom_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);



class XamlParserInfo {
 public:
	XML_Parser parser;

	const char *file_name;

	NameScope *namescope;
	XamlElementInstance *top_element;
	XamlNamespace *current_namespace;
	XamlElementInstance *current_element;

	GHashTable *namespace_map;
	GString *char_data_buffer;

	bool implicit_default_namespace;

	ParserErrorEventArgs *error_args;

	xaml_create_custom_element_callback *custom_element_callback;
	xaml_set_custom_attribute_callback *custom_attribute_callback;

	XamlParserInfo (XML_Parser parser, const char *file_name) :
		parser (parser), file_name (file_name), top_element (NULL), current_element (NULL),
		current_namespace (NULL), char_data_buffer (NULL), implicit_default_namespace (false),
		error_args (NULL), namescope (new NameScope()),
		custom_element_callback (NULL), custom_attribute_callback (NULL)
	{
		namespace_map = g_hash_table_new (g_str_hash, g_str_equal);
	}

	~XamlParserInfo ()
	{
		if (char_data_buffer)
			g_string_free (char_data_buffer, TRUE);
	}
};

class XamlElementInstance : public List::Node {
 public:
	const char *element_name;
	const char *instance_name;

	XamlElementInfo *info;
	XamlElementInstance *parent;
	List *children;

	enum ElementType {
		ELEMENT,
		PROPERTY,
		UNKNOWN
	};

	int element_type;
	DependencyObject *item;


	XamlElementInstance (XamlElementInfo *info) : info (info), element_name (NULL), instance_name (NULL),
						      parent (NULL), element_type (UNKNOWN), item (NULL)
	{
		children = new List ();
	}
	
	~XamlElementInstance ()
	{
		children->Clear (true);
		delete children;
	}
};

class XamlElementInfo {
 public:
	const char *name;
	XamlElementInfo *parent;
	Type::Kind dependency_type;
	const char *content_property;

	create_item_func create_item;
	create_element_instance_func create_element;
	add_child_func add_child;
	set_property_func set_property;
	set_attributes_func set_attributes;



	XamlElementInfo (const char *name, XamlElementInfo *parent, Type::Kind dependency_type) :
		name (name), parent (parent), dependency_type (dependency_type), content_property (NULL),
		create_item (NULL), create_element (NULL), add_child (NULL), set_property (NULL), set_attributes (NULL)
	{

	}

 protected:
	XamlElementInfo () { }

};

class XamlNamespace {

 public:
	const char *name;

	XamlNamespace () : name (NULL) { }
	
	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el) = 0;
	virtual void SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value) = 0;
};

class DefaultNamespace : public XamlNamespace {

 public:
	GHashTable *element_map;

	DefaultNamespace (GHashTable *element_map) : element_map (element_map) { }

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el)
	{
		return (XamlElementInfo *) g_hash_table_lookup (element_map, el);
	}

	virtual void SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		// We don't have to do anything, since the default framework covers us
	}
};

class XNamespace : public XamlNamespace {

 public:
	GHashTable *element_map;

	XNamespace () { }

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el)
	{
		return NULL;
	}

	virtual void SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		if (!strcmp ("Name", attr)) {
			p->namescope->RegisterName (value, (DependencyObject *) item->item);
			return;
		}

		if (!strcmp ("Class", attr)) {
			delete item->item;
			item->item = NULL;

			DependencyObject *dob = p->custom_element_callback (value, NULL);
			if (!dob)
				parser_error (p, item->element_name, attr,
						g_strdup_printf ("Unable to resolve c:Class type %s\n", value));

			item->item = dob;
			return;
		}
	}
};


class CustomElementInfo : public XamlElementInfo {

 public:
	DependencyObject *dependency_object;

	CustomElementInfo (const char *name, XamlElementInfo *parent, Type::Kind dependency_type)
		: XamlElementInfo (name, parent, dependency_type)
	{
	}
};


class CustomNamespace : public XamlNamespace {

 public:
	char *xmlns;

	CustomNamespace (char *xmlns) :
		xmlns (xmlns)
	{

	}

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el)
	{
		DependencyObject *dob = p->custom_element_callback (xmlns, el);

		if (!dob) {
			parser_error (p, el, NULL, g_strdup_printf ("Unable to resolve custom type %s\n", el));
			return NULL;
		}

		CustomElementInfo *info = new CustomElementInfo (g_strdup (el), NULL, dob->GetObjectType ());

		info->create_element = create_custom_element;
		info->set_attributes = dependency_object_set_attributes;
		info->add_child = custom_add_child;
		info->set_property = custom_set_property;
		info->dependency_object = dob;

		return info;
	}

	virtual void SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		p->custom_attribute_callback (item->item, attr, value);
	}
};

XamlElementInstance *
create_custom_element (XamlParserInfo *p, XamlElementInfo *i)
{
	CustomElementInfo *c = (CustomElementInfo *) i;
	XamlElementInstance *inst = new XamlElementInstance (i);

	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;
	inst->item = c->dependency_object;

	return inst;
}

void
set_custom_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{

}

void  custom_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{

}

void  custom_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value)
{

}

//
// Called when we encounter an error.  Note that memory ownership is taken for everything
// except the message, this allows you to use g_strdup_printf when creating the error message
//
void
parser_error (XamlParserInfo *p, const char *el, const char *attr, const char *message)
{
	p->error_args = new ParserErrorEventArgs ();

	p->error_args->line_number = XML_GetCurrentLineNumber (p->parser);
	p->error_args->char_position = XML_GetCurrentColumnNumber (p->parser);
	p->error_args->xaml_file = p->file_name ? strdup (p->file_name) : NULL;
	p->error_args->xml_element = el ? strdup (el) : NULL;
	p->error_args->xml_attribute = attr ? strdup (attr) : NULL;
	p->error_args->error_message = message;

	g_warning ("PARSER ERROR, STOPPING PARSING:  %s  line: %d   char: %d\n", message,
			p->error_args->line_number, p->error_args->char_position);

	XML_StopParser (p->parser, FALSE);
}

void
start_element (void *data, const char *el, const char **attr)
{
	XamlParserInfo *p = (XamlParserInfo *) data;
	XamlElementInfo *elem;
	XamlElementInstance *inst;

	elem = p->current_namespace->FindElement (p, el);

	if (elem) {
		inst = elem->create_element (p, elem);
		elem->set_attributes (p, inst, attr);

		if (!p->top_element) {
			p->top_element = inst;
			p->current_element = inst;
			NameScope::SetNameScope (p->top_element->item, p->namescope);
			return;
		}

		if (p->current_element && p->current_element->element_type != XamlElementInstance::UNKNOWN) {

			if (p->current_element->info)
				p->current_element->info->add_child (p, p->current_element, inst);
			else
				g_warning ("attempt to set property of unimplemented type: %s\n", p->current_element->element_name);
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
			inst->info = p->current_element->info; // We copy this from our parent
			inst->element_name = g_strdup (el);
			inst->element_type = XamlElementInstance::PROPERTY;
		} else {
			g_warning ("Attempting to ignore unimplemented type:  %s\n", el);
			inst = new XamlElementInstance (NULL);
			inst->element_name = g_strdup (el);
			inst->element_type = XamlElementInstance::UNKNOWN;
		}
	}

	inst->parent = p->current_element;
	p->current_element->children->Append (inst);
	p->current_element = inst;	
}

void
start_element_handler (void *data, const char *el, const char **attr)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (p->error_args)
		return;

	char **name = g_strsplit (el, "|", -1);
	char *element;
	
	if (name [1]) {
		// Find the proper namespace
		p->current_namespace = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, name [0]);
		element = name [1];
	} else if (p->implicit_default_namespace) {
		p->current_namespace = default_namespace;
		element = name [0];
	}

	start_element (data, element, attr);

	g_strfreev (name);
}

void
end_element_handler (void *data, const char *el)
{
	XamlParserInfo *info = (XamlParserInfo *) data;

	if (info->error_args)
		return;

	switch (info->current_element->element_type) {
	case XamlElementInstance::ELEMENT:
		if (info->char_data_buffer && info->char_data_buffer->len) {

			bool has_content = false;
			for (int i = 0; i < info->char_data_buffer->len; i++) {
				if (g_ascii_isspace (info->char_data_buffer->str [i]))
					continue;
				has_content = true;
				break;
			}

			const char *cp = info->current_element->info->content_property;
			if (has_content && cp) {
				DependencyProperty *con = DependencyObject::GetDependencyProperty (info->current_element->info->dependency_type, cp);
				// TODO: There might be other types that can be specified here,
				// but string is all i have found so far.  If you can specify other
				// types, i should pull the property setting out of set_attributes
				// and use that code
				if ((con->value_type) == Type::STRING) {
					info->current_element->item->SetValue (con, Value (info->char_data_buffer->str));
				}
			}
		
			g_string_free (info->char_data_buffer, FALSE);
			info->char_data_buffer = NULL;
		}
		break;
	case XamlElementInstance::PROPERTY:
		List::Node *walk = info->current_element->children->First ();
		while (walk) {
			XamlElementInstance *child = (XamlElementInstance *) walk;
			if (info->current_element->parent->element_type != XamlElementInstance::UNKNOWN)
				info->current_element->parent->info->set_property (info, info->current_element->parent,
						info->current_element, child);
			else
				g_warning ("Attempting to set property on unknown type %s\n",
						info->current_element->parent->element_name);
			walk = walk->Next ();
		}
		break;
	}

	info->current_element = info->current_element->parent;
}

void
char_data_handler (void *data, const char *txt, int len)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (p->error_args)
		return;

	if (!p->char_data_buffer) {
		p->char_data_buffer = g_string_new_len (txt, len);
		return;
	}

	p->char_data_buffer = g_string_append_len (p->char_data_buffer, txt, len);
}

void
start_namespace_handler (void *data, const char *prefix, const char *uri)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (p->error_args)
		return;

	if (!strcmp ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", uri) ||
			!strcmp ("http://schemas.microsoft.com/client/2007", uri)) {

		if (prefix)
			return parser_error (p, (p->current_element ? p->current_element->element_name : NULL), prefix,
					g_strdup_printf  ("It is illegal to add a prefix (xmlns:%s) to the default namespace.\n", prefix));

		g_hash_table_insert (p->namespace_map, g_strdup (uri), default_namespace);
	} else if (!strcmp ("http://schemas.microsoft.com/winfx/2006/xaml", uri)) {

		g_hash_table_insert (p->namespace_map, g_strdup (uri), x_namespace);
	} else {
		if (!p->custom_element_callback)
			return parser_error (p, (p->current_element ? p->current_element->element_name : NULL), prefix,
					g_strdup_printf ("No custom element callback installed to handle %s", uri));

		CustomNamespace *c = new CustomNamespace (g_strdup (uri));
		g_hash_table_insert (p->namespace_map, c->xmlns, c);
	}
}

void
add_default_namespaces (XamlParserInfo *p)
{
	p->implicit_default_namespace = true;
	g_hash_table_insert (p->namespace_map, (char *) "http://schemas.microsoft.com/winfx/2006/xaml/presentation", default_namespace);
	g_hash_table_insert (p->namespace_map, (char *) "http://schemas.microsoft.com/winfx/2006/xaml", x_namespace);
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
	
	for (List::Node *walk = el->children->First (); walk != NULL; walk = walk->Next ()) {
		XamlElementInstance *el = (XamlElementInstance *) walk;
		print_tree (el, depth + 1);
	}
}

UIElement *
xaml_create_from_file (const char *xaml_file, bool create_namescope,
		xaml_create_custom_element_callback *cecb,
		xaml_set_custom_attribute_callback *sca,
		Type::Kind *element_type)
{
	UIElement *res;
	FILE *fp;
	char buffer [READ_BUFFER];
	int len, done;
	XamlParserInfo *parser_info = NULL;
	XML_Parser p = NULL;

#ifdef DEBUG_XAML
	printf ("attemtping to load xaml file: %s\n", xaml_file);
#endif
	
	fp = fopen (xaml_file, "r+");

	if (!fp) {
#ifdef DEBUG_XAML
		printf ("can not open file\n");
#endif
		goto cleanup_and_return;
	}

	p = XML_ParserCreateNS (NULL, '|');

	if (!p) {
#ifdef DEBUG_XAML
		printf ("can not create parser\n");
#endif
		fclose (fp);
		fp = NULL;
		goto cleanup_and_return;
	}

	parser_info = new XamlParserInfo (p, xaml_file);
	parser_info->custom_element_callback = cecb;
	parser_info->custom_attribute_callback = sca;

	// TODO: This is just in here temporarily, to make life less difficult for everyone
	// while we are developing.  
	add_default_namespaces (parser_info);

	XML_SetUserData (p, parser_info);

	XML_SetElementHandler (p, start_element_handler, end_element_handler);
	XML_SetCharacterDataHandler (p, char_data_handler);
	XML_SetNamespaceDeclHandler(p, start_namespace_handler, NULL);

	/*
	XML_SetProcessingInstructionHandler (p, proc_handler);
	*/

	done = 0;
	while (!done) {
		len = fread (buffer, 1, READ_BUFFER, fp);
		done = feof (fp);
		if (!XML_Parse (p, buffer, len, done)) {
			parser_error (parser_info, NULL, NULL, g_strdup_printf (XML_ErrorString (XML_GetErrorCode (p))));
			goto cleanup_and_return;
		}
	}

#ifdef DEBUG_XAML
	print_tree (parser_info->top_element, 0);
#endif

	if (parser_info->error_args) {

	}

	if (parser_info->top_element) {
	
		res = (UIElement *) parser_info->top_element->item;
		if (element_type)
			*element_type = parser_info->top_element->info->dependency_type;
		free_recursive (parser_info->top_element);

		if (parser_info->error_args) {
			*element_type = Type::INVALID;
			goto cleanup_and_return;
		}
	}

 cleanup_and_return:
	if (fp)
		fclose (fp);
	if (p)
		XML_ParserFree (p);
	if (parser_info)
		delete parser_info;

	return res;
}

UIElement *
xaml_create_from_str (const char *xaml, bool create_namescope,
		xaml_create_custom_element_callback *cecb,
		xaml_set_custom_attribute_callback *sca,
		Type::Kind *element_type)
{
	XML_Parser p = XML_ParserCreateNS (NULL, '|');
	XamlParserInfo *parser_info = NULL;
	UIElement *res = NULL;

	if (!p) {
#ifdef DEBUG_XAML
		printf ("can not create parser\n");
#endif
		goto cleanup_and_return;
	}

	parser_info = new XamlParserInfo (p, NULL);

	parser_info->custom_element_callback = cecb;
	parser_info->custom_attribute_callback = sca;

	// from_str gets the default namespaces implictly added
	add_default_namespaces (parser_info);

	XML_SetUserData (p, parser_info);

	XML_SetElementHandler (p, start_element_handler, end_element_handler);
	XML_SetCharacterDataHandler (p, char_data_handler);
	XML_SetNamespaceDeclHandler(p, start_namespace_handler, NULL);

	/*
	XML_SetProcessingInstructionHandler (p, proc_handler);
	*/


	if (!XML_Parse (p, xaml, strlen (xaml), TRUE)) {
		parser_error (parser_info, NULL, NULL, g_strdup_printf (XML_ErrorString (XML_GetErrorCode (p))));
		goto cleanup_and_return;
	}

#ifdef DEBUG_XAML
	print_tree (parser_info->top_element, 0);
#endif

	if (parser_info->error_args) {
		// Emit the error event
	}

	if (parser_info->top_element) {
		res = (UIElement *) parser_info->top_element->item;
		if (element_type)
			*element_type = parser_info->top_element->info->dependency_type;
		free_recursive (parser_info->top_element);

		if (parser_info->error_args) {
			res = NULL;
			*element_type = Type::INVALID;
			goto cleanup_and_return;
		}
	}

 cleanup_and_return:
	if (p)
		XML_ParserFree (p);
	if (parser_info)
		delete parser_info;

	return res;
}

int
property_name_index (const char *p)
{
	for (int i = 0; p [i]; i++) {
		if (p [i] == '.' && p [i + 1])
			return i + 1;
	}
	return -1;
}

bool
is_instance_of (XamlElementInstance *item, Type::Kind kind)
{
	for (XamlElementInfo *walk = item->info; walk; walk = walk->parent) {
		if (walk->dependency_type == kind)
			return true;
	}

	return false;
}

gint64
timespan_from_str (const char *str)    
{
	char *next = NULL;
	gint64 res = 0;
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
		if (next && *next)
			next++;
	}

// 	printf ("%d.%d:%d:%d.%d\n", digits [0], digits [1], digits [2], digits [3], digits [4]);  

	// Convert to seconds, then to ticks
	// TODO: This could overflow?
	res = ((digits [0] * 86400) + (digits [1] * 3600) + (digits [2] * 60) + digits [3]);
	res *= 10L;
	res += digits [4];
	res *= 100000L;

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

KeySpline *
key_spline_from_str (const char *str)
{
	int count = 0;
	Point* pts = point_array_from_str (str, &count);
	KeySpline *res = new KeySpline (pts [0], pts [1]);

	return res;
}

// sepcial case, we return a Value, to avoid allocating/freeing a Matrix
Value*
matrix_value_from_str (const char *str)
{
	cairo_matrix_t matrix;
	int count = 0;

	double* values = double_array_from_str (str, &count);
	if (count == 6) {
		matrix.xx = values [0];
		matrix.yx = values [1];
		matrix.xy = values [2];
		matrix.yy = values [3];
		matrix.x0 = values [4];
		matrix.y0 = values [5];
	} else
		cairo_matrix_init_identity (&matrix);
	g_free (values);

	return new Value (&matrix);
}


void
advance (char **data)
{
	char *d = *data;

	while (*d && !g_ascii_isalnum (*d) && *d != '.' && *d != '-')
		d++;

	*data = d;
}

void
get_point (Point *p, char **data)
{
	char *d = *data;

	p->x = strtod (d, &d);
	advance (&d);
	p->y = strtod (d, &d);

	*data = d;
}

void
make_relative (const Point *cp, Point *mv)
{
	mv->x += cp->x;
	mv->y += cp->y;
}

bool
more_points_available (char *data)
{
	char *d  = data;
	while (*d) {
		if (g_ascii_isalpha (*d))
			return false;
		if (g_ascii_isdigit (*d) || *d == '.' || *d == '-')
			return true;
		// otherwise we are whitespace
		d++;
	}

	return false;
}

Point *
get_point_array (char *data, GSList *pl, int *count, bool relative, Point *cp, Point *last)
{
	int c = *count;

	while (more_points_available (data)) {
		Point n;

		get_point (&n, &data);
		advance (&data);

		if (relative) make_relative (cp, &n);

		pl = g_slist_append (pl, &n);
		c++;
	}

	Point *t;
	Point *pts = new Point [c];
	for (int i = 0; i < c; i++) {
		t = (Point *) pl->data;
		pts [i].x = t->x;
		pts [i].y = t->y;
	}

	last = t;
	*count = c;

	return pts;
}
		
Geometry *
geometry_from_str (char *str)
{
	char *data = str;
	int s; // FOr starting expression markers
	Point cp = Point (0, 0);
	Point cp1, cp2, cp3;

	PathFigure *pf = NULL;
	PathSegment *prev = NULL;
	PathSegmentCollection *psc = NULL;
	

	PathGeometry *pg = new PathGeometry ();
	PathFigureCollection *pfc = new PathFigureCollection ();
	pg->SetValue (PathGeometry::FiguresProperty, pfc);

	geometry_set_fill_rule (pg, FillRuleEvenOdd);

	while (*data) {
		if (g_ascii_isspace (*data))
			data++;

		bool relative = false;

		switch (*data) {
		case 'm':
			relative = true;
		case 'M':
			data++;

			get_point (&cp1, &data);
			if (relative) make_relative (&cp, &cp1);

			if (pf)
				pfc->Add (pf);

			pf = new PathFigure ();
			psc = new PathSegmentCollection ();
			pf->SetValue (PathFigure::SegmentsProperty, psc);

			pf->SetValue (PathFigure::StartPointProperty, Value (cp1));

			prev = NULL;

			cp.x = cp1.x;
			cp.y = cp1.y;

			advance (&data);
			while (more_points_available (data)) {
				get_point (&cp1, &data);
				if (relative) make_relative (&cp, &cp1);

				LineSegment* ls = new LineSegment ();
				ls->SetValue (LineSegment::PointProperty, Value (cp1));

				psc->Add (ls);
				prev = ls;
			}

			cp.x = cp1.x;
			cp.y = cp1.y;
			break;

		case 'l':
			relative = true;
		case 'L':
		{
			data++;

			get_point (&cp1, &data);
			if (relative)
				make_relative (&cp, &cp1);

			advance (&data);
			if (more_points_available (data)) {
				GSList *pl = NULL;

				pl = g_slist_append (pl, &cp1);

				Point last;
				int count = 1;
				Point *pts = get_point_array (data, pl, &count, relative, &cp, &last);
				
				PolyLineSegment *pls = new PolyLineSegment ();
				pls->SetValue (PolyLineSegment::PointsProperty, Value (pts, count));

				psc->Add (pls);
				prev = pls;

				cp.x = last.x;
				cp.y = last.y;

				g_slist_free (pl);
			} else {
				LineSegment* ls = new LineSegment ();
				ls->SetValue (LineSegment::PointProperty, Value (cp1));

				psc->Add (ls);
				prev = ls;

				cp.x = cp1.x;
				cp.y = cp1.y;
			}

			
			break;
		}
		case 'h':
			relative = true;
		case 'H':
		{
			data++;

			double x = strtod (data, &data);

			if (relative)
				x += cp.x;
			cp = Point (x, cp.y);

			LineSegment* ls = new LineSegment ();
			ls->SetValue (LineSegment::PointProperty, Value (cp));

			psc->Add (ls);
			prev = ls;

			break;
		}		
		case 'v':
			relative = true;
		case 'V':
		{
			data++;

			double y = strtod (data, &data);

			if (relative)
				y += cp.y;
			cp = Point (cp.x, y);

			LineSegment* ls = new LineSegment ();
			ls->SetValue (LineSegment::PointProperty, Value (cp));

			psc->Add (ls);
			prev = ls;

			break;
		}
		case 'c':
			relative = true;
		case 'C':
		{
			data++;

			get_point (&cp1, &data);
			if (relative) make_relative (&cp, &cp1);
			
			advance (&data);
			get_point (&cp2, &data);
			if (relative) make_relative (&cp, &cp2);

			advance (&data);
			get_point (&cp3, &data);
			if (relative) make_relative (&cp, &cp3);

			advance (&data);
			if (more_points_available (data)) {
				GSList *pl = NULL;
				int count = 3;

				pl = g_slist_append (pl, &cp1);
				pl = g_slist_append (pl, &cp2);
				pl = g_slist_append (pl, &cp3);

				
				Point last;
				Point *pts = get_point_array (data, pl, &count, relative, &cp, &last);
				PolyBezierSegment *pbs = new PolyBezierSegment ();
				pbs->SetValue (PolyBezierSegment::PointsProperty, Value (pts, count));

				psc->Add (pbs);
				prev = pbs;

				cp.x = last.x;
				cp.y = last.y;

				g_slist_free (pl);
			} else {
				BezierSegment *bs = new BezierSegment ();
				bs->SetValue (BezierSegment::Point1Property, Value (cp1));
				bs->SetValue (BezierSegment::Point2Property, Value (cp2));
				bs->SetValue (BezierSegment::Point3Property, Value (cp3));

				psc->Add (bs);
				prev = bs;

				cp.x = cp3.x;
				cp.y = cp3.y;
			}

			
			break;
		}
		case 's':
			relative = true;
		case 'S':
		{
			data++;

			get_point (&cp2, &data);
			if (relative) make_relative (&cp, &cp2);

			advance (&data);

			get_point (&cp3, &data);
			if (relative) make_relative (&cp, &cp3);

			if (prev->GetObjectType () == Type::BEZIERSEGMENT) {
				Point *p = prev->GetValue (BezierSegment::Point2Property)->AsPoint ();
				cp1.x = 2 * cp.x - p->x;
				cp1.y = 2 * cp.y - p->y;
			} else
				cp2 = cp;
			if (relative) make_relative (&cp, &cp2);

			advance (&data);
			if (more_points_available (data)) {
				GSList *pl = NULL;
				int count = 3;

				pl = g_slist_append (pl, &cp1);
				pl = g_slist_append (pl, &cp2);
				pl = g_slist_append (pl, &cp3);

				Point last;
				Point *pts = get_point_array (data, pl, &count, relative, &cp, &last);
				PolyBezierSegment *pbs = new PolyBezierSegment ();
				pbs->SetValue (PolyBezierSegment::PointsProperty, Value (pts, count));

				psc->Add (pbs);
				prev = pbs;

				cp.x = last.x;
				cp.y = last.y;

				g_slist_free (pl);
			} else {
				BezierSegment *bs = new BezierSegment ();
				bs->SetValue (BezierSegment::Point1Property, Value (cp1));
				bs->SetValue (BezierSegment::Point2Property, Value (cp2));
				bs->SetValue (BezierSegment::Point3Property, Value (cp3));

				psc->Add (bs);
				prev = bs;

				cp.x = cp3.x;
				cp.y = cp3.y;
			}
			break;
		}
		case 'q':
			relative = true;
		case 'Q':
		{
			data++;

			get_point (&cp1, &data);
			if (relative) make_relative (&cp, &cp1);

			advance (&data);
			get_point (&cp2, &data);
			if (relative) make_relative (&cp, &cp2);

			advance (&data);
			if (more_points_available (data)) {
				GSList *pl = NULL;
				int count = 2;

				pl = g_slist_append (pl, &cp1);
				pl = g_slist_append (pl, &cp2);

				Point last;
				Point *pts = get_point_array (data, pl, &count, relative, &cp, &last);
				PolyQuadraticBezierSegment *pqbs = new PolyQuadraticBezierSegment ();
				pqbs->SetValue (PolyQuadraticBezierSegment::PointsProperty, Value (pts, count));

				psc->Add (pqbs);
				prev = pqbs;

				cp.x = last.x;
				cp.y = last.y;

				g_slist_free (pl);
			} else {
				QuadraticBezierSegment *qbs = new QuadraticBezierSegment ();
				qbs->SetValue (QuadraticBezierSegment::Point1Property, Value (cp1));
				qbs->SetValue (QuadraticBezierSegment::Point2Property, Value (cp2));

				psc->Add (qbs);
				prev = qbs;

				cp.x = cp2.x;
				cp.y = cp2.y;
			}
			break;
		}
		case 't':
			relative = true;
		case 'T':
		{
			data++;

			get_point (&cp2, &data);
			if (relative) make_relative (&cp, &cp2);

			if (prev->GetObjectType () == Type::QUADRATICBEZIERSEGMENT) {
				Point *p = prev->GetValue (QuadraticBezierSegment::Point2Property)->AsPoint ();
				cp1.x = 2 * cp.x - p->x;
				cp1.y = 2 * cp.y - p->y;
			} else
				cp1 = cp;

			advance (&data);

			if (more_points_available (data)) {
				GSList *pl = NULL;
				int count = 2;

				pl = g_slist_append (pl, &cp1);
				pl = g_slist_append (pl, &cp2);

				Point last;
				Point *pts = get_point_array (data, pl, &count, relative, &cp, &last);
				PolyQuadraticBezierSegment *pqbs = new PolyQuadraticBezierSegment ();
				pqbs->SetValue (PolyQuadraticBezierSegment::PointsProperty, Value (pts, count));

				psc->Add (pqbs);
				prev = pqbs;

				cp.x = last.x;
				cp.y = last.y;

				g_slist_free (pl);
			} else {
				QuadraticBezierSegment *qbs = new QuadraticBezierSegment ();
				qbs->SetValue (QuadraticBezierSegment::Point1Property, Value (cp1));
				qbs->SetValue (QuadraticBezierSegment::Point2Property, Value (cp2));

				psc->Add (qbs);
				prev = qbs;

				cp.x = cp2.x;
				cp.y = cp2.y;
			}
			break;
		}
		case 'a':
			relative = true;
		case 'A':
		{
			data++;

			while (more_points_available (data)) {
				get_point (&cp1, &data);

				advance (&data);
				double angle = strtod (data, &data);

				advance (&data);
				int is_large = strtol (data, &data, 10);

				advance (&data);
				int sweep = strtol (data, &data, 10);

				advance (&data);
				get_point (&cp2, &data);
				if (relative) make_relative (&cp, &cp2);

				ArcSegment *arc = new ArcSegment ();
				arc->SetValue (ArcSegment::SizeProperty, cp1);
				arc->SetValue (ArcSegment::RotationAngleProperty, angle);
				arc->SetValue (ArcSegment::IsLargeArcProperty, (bool) is_large);
				arc->SetValue (ArcSegment::SweepDirectionProperty, sweep);
				arc->SetValue (ArcSegment::PointProperty, cp2);

				psc->Add (arc);
				prev = arc;
					
				cp.x = cp2.x;
				cp.y = cp2.y;

				advance (&data);
			}
			break;
		}
		case 'z':
		case 'Z':
			data++;

			prev = NULL;
			path_figure_set_is_closed (pf, true);
			break;

		default:
			data++;
			break;
			
		}
	}

	if (pf)
		pfc->Add (pf);
	
	return pg;
}

///
/// ENUMS
///

typedef struct {
	const char *name;
	int value;
} enum_map_t;

enum_map_t alignment_x_map [] = {
	{ "Left", 0 },
	{ "Center", 1 },
	{ "Right", 2 },
	{ NULL, 0 },
};

enum_map_t alignment_y_map [] = {
	{ "Top", 0 },
	{ "Center", 1 },
	{ "Bottom", 2 },
	{ NULL, 0 },
};

enum_map_t brush_mapping_mode_map [] = {
	{ "Absolute", 0 },
	{ "RelativeToBoundingBox", 1},
	{ NULL, 0 },
};


enum_map_t color_interpolation_mode_map [] = {
	{ "ScRgbLinearInterpolation", 0 },
	{ "SRgbLinearInterpolation", 1 },
	{ NULL, 0 },
};

enum_map_t cursors_map [] = {
	{ "Default", 0 },
	{ "Arrow", 1 },
	{ "Hand", 2 },
	{ "Wait", 3 },
	{ "IBeam", 4 },
	{ "Stylus", 5 },
	{ "Eraser", 6 },
	{ "None", 7 },
	{ NULL, 0 },
};

enum_map_t error_type_map [] = {
	{ "NoError", 0 },
	{ "UnknownError", 1 },
	{ "InitializeError", 2 },
	{ "ParserError", 3 },
	{ "ObjectModelError", 4 },
	{ "RuntimeError", 5 },
	{ "DownloadError", 6 },
	{ "MediaError", 7 },
	{ "ImageError", 8 },
	{ NULL, 0 },
};

enum_map_t fill_behavior_map [] = {
	{ "HoldEnd", 0 },
	{ "Stop", 1 },
	{ NULL, 0 },
};

enum_map_t fill_rule_map [] = {
	{ "EvenOdd", 0 },
	{ "Nonzero", 1},
	{ NULL, 0 },
};

enum_map_t font_stretches_map [] = {
	{ "UltraCondensed", 1 },
	{ "ExtraCondensed", 2 },
	{ "Condensed",      3 },
	{ "SemiCondensed",  4 },
	{ "Normal",         5 },
	{ "Medium",         5 },
	{ "SemiExpanded",   6 },
	{ "Expanded",       7 },
	{ "ExtraExpanded",  8 },
	{ "UltraExpanded",  9 },
	{ NULL, 0 },
};

enum_map_t font_styles_map [] = {
	{ "Normal",  0 },
	{ "Oblique", 1 },
	{ "Italic",  2 },
	{ NULL, 0 },
};

enum_map_t font_weights_map [] = {
	{ "Thin",       100 },
	{ "ExtraLight", 200 },
	{ "UltraLight", 200 },
	{ "Light",      300 },
	{ "Normal",     400 },
	{ "Regular",    400 },
	{ "Medium",     500 },
	{ "SemiBold",   600 },
	{ "DemiBold",   600 },
	{ "Bold",       700 },
	{ "ExtraBold",  800 },
	{ "UltraBold",  800 },
 	{ "Black",      900 },
	{ "Heavy",      900 },
	{ "ExtraBlack", 950 },
	{ "UltraBlack", 950 },
	{ NULL, 0 },
};

enum_map_t gradient_spread_method_map [] = {
	{ "Pad", 0 },
	{ "Reflect", 1 },
	{ "Repeat", 2 },
	{ NULL, 0 },
};

enum_map_t pen_line_cap_map [] = {
	{ "Flat", 0 },
	{ "Square", 1 },
	{ "Round", 2 },
	{ "Triangle", 3 },
	{ NULL, 0 },
};

enum_map_t pen_line_join_map [] = {
	{ "Miter", 0 },
	{ "Bevel", 1 },
	{ "Round", 2 },
	{ NULL, 0 },
};

enum_map_t stretch_map [] = {
	{ "None", 0 },
	{ "Fill", 1 },
	{ "Uniform", 2 },
	{ "UniformToFill", 3 },
	{ NULL, 0 },
};

enum_map_t sweep_direction_map [] = {
	{ "Counterclockwise", 0 },
	{ "Clockwise", 1 },
	{ NULL, 0 },
};

enum_map_t tablet_device_type_map [] = {
	{ "Mouse", 0 },
	{ "Stylus", 1 },
	{ "Touch", 2 },
	{ NULL, 0 },
};

enum_map_t text_decorations_map [] = {
	{ "None", 0 },
	{ "Underline", 1 },
	{ NULL, 0 },
};

enum_map_t text_wrapping_map [] = {
	{ "Wrap", 0 },
	{ "NoWrap", 1 },
	{ "WrapWithOverflow", 2 },
	{ NULL, 0 },
};

enum_map_t visibility_map [] = {
	{ "Visible", 0 },
	{ "Collapsed", 1 },
	{ "Hidden", 2 },
	{ NULL, 0 },
};

int enum_from_str (const enum_map_t *emu, const char *str)
{
	for (int i = 0; emu [i].name; i++) {
		if (!strcmp (emu [i].name, str))
			return emu [i].value;
	}

	// This mind wind up being a legal value, so maybe we should blow up here?
	return -1;
}

XamlElementInstance *
default_create_element_instance (XamlParserInfo *p, XamlElementInfo *i)
{
	XamlElementInstance *inst = new XamlElementInstance (i);

	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;

	if (is_instance_of (inst, Type::COLLECTION)) {
		// If we are creating a collection, try walking up the element tree,
		// to find the parent that we belong to and using that instance for
		// our collection, instead of creating a new one

		XamlElementInstance *walk = p->current_element;
		DependencyProperty *dep = NULL;

		// We attempt to advance past the property setter, because we might be dealing with a
		// content element
		if (walk && walk->element_type == XamlElementInstance::PROPERTY) {
			char **prop_name = g_strsplit (walk->element_name, ".", -1);
			
			walk = walk->parent;
			dep = DependencyObject::GetDependencyProperty (walk->info->dependency_type, prop_name [1]);

			g_strfreev (prop_name);
		} else if (walk && walk->info->content_property) {
			dep = DependencyObject::GetDependencyProperty (walk->info->dependency_type,
					(char *) walk->info->content_property);			
		}

		if (dep && dep->value_type == i->dependency_type) {
			Value *v = ((DependencyObject * ) walk->item)->GetValue (dep);
			if (v)
				inst->item = v->AsCollection ();
		}
	}

	if (!inst->item)
		inst->item = i->create_item ();

	return inst;
}

///
/// Add Child funcs
///

void
dependency_object_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	if (parent->element_type == XamlElementInstance::PROPERTY) {
		char **prop_name = g_strsplit (parent->element_name, ".", -1);
		XamlElementInfo *powner = (XamlElementInfo *) g_hash_table_lookup (default_namespace->element_map, prop_name [0]);
		DependencyProperty *dep = DependencyObject::GetDependencyProperty (powner->dependency_type, prop_name [1]);

		g_strfreev (prop_name);

		// Don't add the child element, if it is the entire collection
		if (!dep || dep->value_type == child->info->dependency_type)
			return;

		Type *col_type = Type::Find (dep->value_type);
		if (!col_type->IsSubclassOf (Type::COLLECTION))
			return;

		// Most common case, we will have a parent that we can snag the collection from
		DependencyObject *obj = (DependencyObject *) parent->parent->item;
		Value *col_v = obj->GetValue (dep);
		Collection *col = (Collection *) col_v->AsCollection ();
		col->Add ((DependencyObject*)child->item);
		return;
	}

	if (is_instance_of (parent, Type::COLLECTION)) {
		Collection *col = (Collection *) parent->item;

		col->Add ((DependencyObject *) child->item);
		return;
	}

	
	if (parent->info->content_property) {
		DependencyProperty *dep = DependencyObject::GetDependencyProperty (parent->info->dependency_type,
				(char *) parent->info->content_property);

		if (!dep)
			return;

		Type *prop_type = Type::Find (dep->value_type);
		bool is_collection = prop_type->IsSubclassOf (Type::COLLECTION);

		if (!is_collection && prop_type->IsSubclassOf (child->info->dependency_type)) {
			DependencyObject *obj = (DependencyObject *) parent->item;
			obj->SetValue (dep, (DependencyObject *) child->item);
			return;

		}

		// We only want to enter this if statement if we are NOT dealing with the content property element,
		// otherwise, attempting to use explicit property setting, would add the content property element
		// to the content property element collection
		if (is_collection && dep->value_type != child->info->dependency_type) {
			DependencyObject *obj = (DependencyObject *) parent->item;
			Value *col_v = obj->GetValue (dep);
			Collection *col;
			
			if (!col_v) {
				col = collection_new (dep->value_type);
				obj->SetValue (dep, Value (col));
			} else {
				col = (Collection *) col_v->AsCollection ();
			}
			
			col->Add ((DependencyObject *) child->item);
			return;
		}
	}

	// Do nothing if we aren't adding to a collection, or a content property collection
}

void
panel_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	if (parent->element_type != XamlElementInstance::PROPERTY)
		panel_child_add ((Panel *) parent->item, (UIElement *) child->item);

	dependency_object_add_child (p, parent, child);
}

///
/// set property funcs
///

// these are just a bunch of special cases
void
dependency_object_missed_property (XamlElementInstance *item, XamlElementInstance *prop, XamlElementInstance *value, char **prop_name)
{
			
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
		if (is_instance_of (value, prop->value_type)) {
			// an empty collection can be NULL and valid
			if (value->item)
				dep->SetValue (prop, Value ((DependencyObject *) value->item));
		} else {
			// TODO: Do some error checking in here, this is a valid place to be
			// if we are adding a non collection to a collection, so the non collection
			// item will have already been added to the collection in add_child
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
xaml_set_property_from_str (DependencyObject *obj, const char *pname, const char *value)
{
	DependencyProperty *prop = DependencyObject::GetDependencyProperty (obj->GetObjectType (), (char *) pname);

	if (!prop) {
		printf ("can not find property:  %s\n", prop);
		return;
	}

	switch (prop->value_type) {
	case Type::BOOL:
		obj->SetValue (prop, Value ((bool) !g_strcasecmp ("true", value)));
		break;
	case Type::DOUBLE:
		obj->SetValue (prop, Value ((double) strtod (value, NULL)));
		break;
	case Type::INT64:
		obj->SetValue (prop, Value ((gint64) strtol (value, NULL, 10)));
		break;
	case Type::INT32:
	{
		// Maybe we should try an [0] != '-' && !isdigit before looking up the enum?
		int val;
		enum_map_t *emu = (enum_map_t *) g_hash_table_lookup (enum_map, value);

		if (emu)
			val = enum_from_str (emu, value);
		else
			val = (int) strtol (value, NULL, 10);
		obj->SetValue (prop, Value (val));
		break;
	}
	case Type::STRING:
		obj->SetValue (prop, Value (value));
		break;
	case Type::COLOR:
		obj->SetValue (prop, Value (*color_from_str (value)));
		break;
	case Type::REPEATBEHAVIOR:
		obj->SetValue (prop, Value (repeat_behavior_from_str (value)));
		break;
	case Type::DURATION:
		obj->SetValue (prop, Value (duration_from_str (value)));
		break;
	case Type::KEYTIME:
		obj->SetValue (prop, Value (KeyTime (timespan_from_str (value))));
		break;
	case Type::KEYSPLINE:
		obj->SetValue (prop, Value (key_spline_from_str (value)));
		break;
	case Type::BRUSH:
	case Type::SOLIDCOLORBRUSH:
	{
		// Only solid color brushes can be specified using attribute syntax
		SolidColorBrush *scb = solid_color_brush_new ();
		Color *c = color_from_str (value);
		solid_color_brush_set_color (scb, c); // copies c
		delete c;
		obj->SetValue (prop, Value (scb));
		break;
	}
	case Type::POINT:
		obj->SetValue (prop, Value (point_from_str (value)));
		break;
	case Type::RECT:
		obj->SetValue (prop, Value (rect_from_str (value)));
		break;
	case Type::DOUBLE_ARRAY:
	{
		int count = 0;
		double *doubles = double_array_from_str (value, &count);
		obj->SetValue (prop, Value (doubles, count));
		break;
	}
	case Type::POINT_ARRAY:
	{
		int count = 0;
		Point *points = point_array_from_str (value, &count);
		obj->SetValue (prop, Value (points, count));
		break;
	}
	
	case Type::MATRIX:
		obj->SetValue (prop, matrix_value_from_str (value));
		break;
	case Type::GEOMETRY:
	{
		char *data = g_strdup (value);
		obj->SetValue (prop, geometry_from_str (data));
		g_free (data);
		break;
	}
	default:
		printf ("could not find value type for: %s to '%s' %d\n", prop, value, prop->value_type);
		break;
	}
}

// TODO: Merge more of this code with the above function
void
dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	for (int i = 0; attr [i]; i += 2) {
		// Setting attributes like x:Class can change item->item, so we
		// need to make sure we have an up to date pointer
		DependencyObject *dep = (DependencyObject *) item->item;
		char **attr_name = g_strsplit (attr [i], "|", -1);

		if (attr_name [1]) {
			XamlNamespace *ns = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, attr_name [0]);

			if (!ns)
				return parser_error (p, item->element_name, attr [i],
						g_strdup_printf ("Could not find namespace %s", attr_name [0]));

			ns->SetAttribute (p, item, attr_name [1], attr [i + 1]);

			g_strfreev (attr_name);

			// Setting custom attributes can cause errors galore
			if (p->error_args)
				return;

			continue;
		}

		g_strfreev (attr_name);

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
			XamlElementInfo *attached = (XamlElementInfo *) g_hash_table_lookup (default_namespace->element_map, atchname);
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
			case Type::BOOL:
				dep->SetValue (prop, Value ((bool) !g_strcasecmp ("true", attr [i + 1])));
				break;
			case Type::DOUBLE:
				dep->SetValue (prop, Value ((double) strtod (attr [i + 1], NULL)));
				break;
			case Type::INT64:
				dep->SetValue (prop, Value ((gint64) strtol (attr [i + 1], NULL, 10)));
				break;
			case Type::INT32:
			{
				// Maybe we should try an [0] != '-' && !isdigit before looking up the enum?
				int val;
				enum_map_t *emu = (enum_map_t *) g_hash_table_lookup (enum_map, attr [i]);

				if (emu)
					val = enum_from_str (emu, attr [i + 1]);
				else
					val = (int) strtol (attr [i + 1], NULL, 10);
				dep->SetValue (prop, Value (val));
			}
				break;
			case Type::STRING:
				dep->SetValue (prop, Value (attr [i + 1]));
				break;
			case Type::COLOR:
				dep->SetValue (prop, Value (*color_from_str (attr [i + 1])));
				break;
			case Type::REPEATBEHAVIOR:
				dep->SetValue (prop, Value (repeat_behavior_from_str (attr [i + 1])));
				break;
			case Type::DURATION:
				dep->SetValue (prop, Value (duration_from_str (attr [i + 1])));
				break;
			case Type::KEYTIME:
				dep->SetValue (prop, Value (KeyTime (timespan_from_str (attr [i + 1]))));
				break;
			case Type::KEYSPLINE:
				dep->SetValue (prop, Value (key_spline_from_str (attr [i + 1])));
				break;
			case Type::BRUSH:
			case Type::SOLIDCOLORBRUSH:
			{
				// Only solid color brushes can be specified using attribute syntax
				SolidColorBrush *scb = solid_color_brush_new ();
				Color *c = color_from_str (attr [i + 1]);
				solid_color_brush_set_color (scb, c); // copies c
				delete c;
				dep->SetValue (prop, Value (scb));
			}
				break;
			case Type::POINT:
				dep->SetValue (prop, Value (point_from_str (attr [i + 1])));
				break;
			case Type::RECT:
				dep->SetValue (prop, Value (rect_from_str (attr [i + 1])));
				break;
			case Type::DOUBLE_ARRAY:
			{
				int count = 0;
				double *doubles = double_array_from_str (attr [i + 1], &count);
				dep->SetValue (prop, Value (doubles, count));
			}
				break;
			case Type::POINT_ARRAY:
			{
				int count = 0;
				Point *points = point_array_from_str (attr [i + 1], &count);
				dep->SetValue (prop, Value (points, count));
			}
				break;
			case Type::MATRIX:
				dep->SetValue (prop, matrix_value_from_str (attr [i + 1]));
				break;
			case Type::GEOMETRY:
			{
				char *data = g_strdup (attr [i + 1]);
				dep->SetValue (prop, geometry_from_str (data));
				g_free (data);
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

// We still use a name for ghost elements to make debugging easier
XamlElementInfo *
register_ghost_element (const char *name, XamlElementInfo *parent, Type::Kind dt)
{
	return new XamlElementInfo (name, parent, dt);
}

XamlElementInfo *
register_dependency_object_element (GHashTable *table, const char *name, XamlElementInfo *parent, Type::Kind dt,
		create_item_func create_item)
{
	XamlElementInfo *res = new XamlElementInfo (name, parent, dt);

	res->content_property = NULL;
	res->create_item = create_item;
	res->create_element = default_create_element_instance;
	res->add_child = dependency_object_add_child;
	res->set_property = dependency_object_set_property;
	res->set_attributes = dependency_object_set_attributes;

	g_hash_table_insert (table, (char *) name, GINT_TO_POINTER (res));

	return res;
}




void
xaml_init (void)
{
	GHashTable *dem = g_hash_table_new (g_str_hash, g_str_equal); // default element map
	enum_map = g_hash_table_new (g_str_hash, g_str_equal);

	XamlElementInfo *col = register_ghost_element ("Collection", NULL, Type::COLLECTION);

#define rdoe register_dependency_object_element

	//
	// ui element ->
	//
	XamlElementInfo *ui = register_ghost_element ("UIElement", NULL, Type::UIELEMENT);
	XamlElementInfo *fw = register_ghost_element ("FrameworkElement", ui, Type::FRAMEWORKELEMENT);
	XamlElementInfo *shape = register_ghost_element ("Shape", fw, Type::SHAPE);

	rdoe (dem, "ResourceCollection", col, Type::RESOURCE_COLLECTION, (create_item_func) resource_collection_new);

	///
	/// Shapes
	///
	
	rdoe (dem, "Ellipse", shape, Type::ELLIPSE, (create_item_func) ellipse_new);
	rdoe (dem, "Line", shape, Type::LINE, (create_item_func) line_new);
	rdoe (dem, "Path", shape, Type::PATH, (create_item_func) path_new);
	rdoe (dem, "Polygon", shape, Type::POLYGON, (create_item_func) polygon_new);
	rdoe (dem, "Polyline", shape, Type::POLYLINE, (create_item_func) polyline_new);
	rdoe (dem, "Rectangle", shape, Type::RECTANGLE, (create_item_func) rectangle_new);
	
	///
	/// Geometry
	///

	XamlElementInfo *geo = register_ghost_element ("Geometry", NULL, Type::GEOMETRY);
	rdoe (dem, "EllipseGeometry", geo, Type::ELLIPSEGEOMETRY, (create_item_func) ellipse_geometry_new);
	rdoe (dem, "LineGeometry", geo, Type::LINEGEOMETRY, (create_item_func) line_geometry_new);
	rdoe (dem, "RectangleGeometry", geo, Type::RECTANGLEGEOMETRY, (create_item_func) rectangle_geometry_new);

	XamlElementInfo *gg = rdoe (dem, "GeometryGroup", geo, Type::GEOMETRYGROUP, (create_item_func) geometry_group_new);
	gg->content_property = "Children";


	XamlElementInfo *gc = rdoe (dem, "GeometryCollection", col, Type::GEOMETRY_COLLECTION, (create_item_func) geometry_group_new);
	XamlElementInfo *pg = rdoe (dem, "PathGeometry", geo, Type::PATHGEOMETRY, (create_item_func) path_geometry_new);
	pg->content_property = "Figures";

	XamlElementInfo *pfc = rdoe (dem, "PathFigureCollection", col, Type::PATHFIGURE_COLLECTION, (create_item_func) NULL);

	XamlElementInfo *pf = rdoe (dem, "PathFigure", geo, Type::PATHFIGURE, (create_item_func) path_figure_new);
	pf->content_property = "Segments";

	XamlElementInfo *psc = rdoe (dem, "PathSegmentCollection", col, Type::PATHSEGMENT_COLLECTION, (create_item_func) path_figure_new);

	XamlElementInfo *ps = register_ghost_element ("PathSegment", NULL, Type::PATHSEGMENT);
	rdoe (dem, "ArcSegment", ps, Type::ARCSEGMENT, (create_item_func) arc_segment_new);
	rdoe (dem, "BezierSegment", ps, Type::BEZIERSEGMENT, (create_item_func) bezier_segment_new);
	rdoe (dem, "LineSegment", ps, Type::LINESEGMENT, (create_item_func) line_segment_new);
	rdoe (dem, "PolyBezierSegment", ps, Type::POLYBEZIERSEGMENT, (create_item_func) poly_bezier_segment_new);
	rdoe (dem, "PolyLineSegment", ps, Type::POLYLINESEGMENT, (create_item_func) poly_line_segment_new);
	rdoe (dem, "PolyQuadraticBezierSegment", ps, Type::POLYQUADRATICBEZIERSEGMENT, (create_item_func) poly_quadratic_bezier_segment_new);
	rdoe (dem, "QuadraticBezierSegment", ps, Type::QUADRATICBEZIERSEGMENT, (create_item_func) quadratic_bezier_segment_new);

	///
	/// Panels
	///
	
	XamlElementInfo *panel = register_ghost_element ("Panel", fw, Type::PANEL);
	XamlElementInfo *canvas = rdoe (dem, "Canvas", panel, Type::CANVAS, (create_item_func) canvas_new);
	panel->add_child = panel_add_child;
	canvas->add_child = panel_add_child;

	///
	/// Animation
	///
	
	XamlElementInfo *tl = register_ghost_element ("Timeline", NULL, Type::TIMELINE);
	XamlElementInfo *anim = register_ghost_element ("Animation", tl, Type::ANIMATION);
	
	
	XamlElementInfo * da = rdoe (dem, "DoubleAnimation", anim, Type::DOUBLEANIMATION, (create_item_func) double_animation_new);
	XamlElementInfo *ca = rdoe (dem, "ColorAnimation", anim, Type::COLORANIMATION, (create_item_func) color_animation_new);
	XamlElementInfo *pa = rdoe (dem, "PointAnimation", anim, Type::POINTANIMATION, (create_item_func) point_animation_new);

	XamlElementInfo *daukf = rdoe (dem, "DoubleAnimationUsingKeyFrames", da, Type::DOUBLEANIMATIONUSINGKEYFRAMES, (create_item_func) double_animation_using_key_frames_new);
	daukf->content_property = "KeyFrames";

	XamlElementInfo *caukf = rdoe (dem, "ColorAnimationUsingKeyFrames", ca, Type::COLORANIMATIONUSINGKEYFRAMES, (create_item_func) color_animation_using_key_frames_new);
	caukf->content_property = "KeyFrames";

	XamlElementInfo *paukf = rdoe (dem, "PointAnimationUsingKeyFrames", pa, Type::POINTANIMATIONUSINGKEYFRAMES, (create_item_func) point_animation_using_key_frames_new);
	paukf->content_property = "KeyFrames";
	
	rdoe (dem, "KeyFrameCollection", col, Type::KEYFRAME_COLLECTION, (create_item_func) key_frame_collection_new);

	XamlElementInfo *keyfrm = register_ghost_element ("KeyFrame", NULL, Type::KEYFRAME);

	XamlElementInfo *ckf = register_ghost_element ("ColorKeyFrame", keyfrm, Type::COLORKEYFRAME);
	rdoe (dem, "DiscreteColorKeyFrame", ckf, Type::DISCRETECOLORKEYFRAME, (create_item_func) discrete_color_key_frame_new);
	rdoe (dem, "LinearColorKeyFrame", ckf, Type::LINEARCOLORKEYFRAME, (create_item_func) linear_color_key_frame_new);
//	rdoe (dem, "SplineColorKeyFrame", ckf, Value::SPLINECOLORKEYFRAME, (create_item_func) spline_color_key_frame_new);

	XamlElementInfo *dkf = register_ghost_element ("DoubleKeyFrame", keyfrm, Type::DOUBLEKEYFRAME);
	rdoe (dem, "DiscreteDoubleKeyFrame", dkf, Type::DISCRETEDOUBLEKEYFRAME, (create_item_func) discrete_double_key_frame_new);
	rdoe (dem, "LinearDoubleKeyFrame", dkf, Type::LINEARDOUBLEKEYFRAME, (create_item_func) linear_double_key_frame_new);
	rdoe (dem, "SplineDoubleKeyFrame", dkf, Type::SPLINEDOUBLEKEYFRAME, (create_item_func) spline_double_key_frame_new);

	XamlElementInfo *pkf = register_ghost_element ("PointKeyFrame", keyfrm, Type::POINTKEYFRAME);
	rdoe (dem, "DiscretePointKeyFrame", pkf, Type::DISCRETEPOINTKEYFRAME, (create_item_func) discrete_point_key_frame_new);
	rdoe (dem, "LinearPointKeyFrame", pkf, Type::LINEARPOINTKEYFRAME, (create_item_func) linear_point_key_frame_new);
//	rdoe (dem, "SplinePointKeyFrame", pkf, Value::SPLINEPOINTKEYFRAME, (create_item_func) spline_point_key_frame_new);

	rdoe (dem, "KeySpline", NULL, Type::KEYSPLINE, (create_item_func) key_spline_new);

	rdoe (dem, "TimelineCollection", col, Type::TIMELINE_COLLECTION, (create_item_func) timeline_collection_new);

	XamlElementInfo *timel = register_ghost_element ("Timeline", NULL, Type::TIMELINE);
	XamlElementInfo *tlg = rdoe (dem, "TimelineGroup", timel, Type::TIMELINEGROUP, (create_item_func) timeline_group_new);
	XamlElementInfo *prltl = register_ghost_element ("ParallelTimeline", tlg, Type::PARALLELTIMELINE);

	XamlElementInfo *sb = rdoe (dem, "Storyboard", prltl, Type::STORYBOARD, (create_item_func) storyboard_new);
	sb->content_property = "Children";

	///
	/// Triggers
	///
	XamlElementInfo *trg = register_ghost_element ("Trigger", NULL, Type::TRIGGERACTION);
	XamlElementInfo *bsb = rdoe (dem, "BeginStoryboard", trg, Type::BEGINSTORYBOARD,
			(create_item_func) begin_storyboard_new);
	bsb->content_property = "Storyboard";

	XamlElementInfo *evt = rdoe (dem, "EventTrigger", NULL, Type::EVENTTRIGGER, (create_item_func) event_trigger_new);
	evt->content_property = "Actions";

	rdoe (dem, "TriggerCollection", col, Type::TRIGGER_COLLECTION, (create_item_func) trigger_collection_new);
	rdoe (dem, "TriggerActionCollection", col, Type::TRIGGERACTION_COLLECTION, (create_item_func) trigger_action_collection_new);

	///
	/// Transforms
	///

	XamlElementInfo *tf = register_ghost_element ("Transform", NULL, Type::TRANSFORM);
	rdoe (dem, "RotateTransform", tf, Type::ROTATETRANSFORM, (create_item_func) rotate_transform_new);
	rdoe (dem, "ScaleTransform", tf, Type::SCALETRANSFORM, (create_item_func) scale_transform_new);
	rdoe (dem, "SkewTransform", tf, Type::SKEWTRANSFORM, (create_item_func) skew_transform_new);
	rdoe (dem, "TranslateTransform", tf, Type::TRANSLATETRANSFORM, (create_item_func) translate_transform_new);
	rdoe (dem, "MatrixTransform", tf, Type::MATRIXTRANSFORM, (create_item_func) matrix_transform_new);
	XamlElementInfo *tg = rdoe (dem, "TransformGroup", tf, Type::TRANSFORMGROUP, (create_item_func) transform_group_new);
	tg->content_property = "Children";
	
	rdoe (dem, "TransformCollection", col, Type::TRANSFORM_COLLECTION, (create_item_func) transform_group_new);


	///
	/// Brushes
	///

	XamlElementInfo *brush = register_ghost_element ("Brush", NULL, Type::BRUSH);
	rdoe (dem, "SolidColorBrush", brush, Type::SOLIDCOLORBRUSH, (create_item_func) solid_color_brush_new);

	XamlElementInfo *gb = register_ghost_element ("GradientBrush", brush, Type::GRADIENTBRUSH);
	gb->content_property = "GradientStops";
	rdoe (dem, "LinearGradientBrush", gb, Type::LINEARGRADIENTBRUSH, (create_item_func) linear_gradient_brush_new);
	rdoe (dem, "RadialGradientBrush", gb, Type::RADIALGRADIENTBRUSH, (create_item_func) radial_gradient_brush_new);

	rdoe (dem, "GradientStopCollection", col, Type::GRADIENTSTOP_COLLECTION, (create_item_func) gradient_stop_collection_new);

	rdoe (dem, "GradientStop", NULL, Type::GRADIENTSTOP, (create_item_func) gradient_stop_new);

	rdoe (dem, "TileBrush", brush, Type::TILEBRUSH, (create_item_func) tile_brush_new);
	rdoe (dem, "ImageBrush", brush, Type::IMAGEBRUSH, (create_item_func) image_brush_new);
	rdoe (dem, "VideoBrush", brush, Type::VIDEOBRUSH, (create_item_func) video_brush_new);

	///
	/// Media
	///

	XamlElementInfo *mb = register_ghost_element ("MediaBase", NULL, Type::MEDIABASE);

	rdoe (dem, "Image", mb, Type::IMAGE, (create_item_func) image_new);
	rdoe (dem, "MediaElement", mb, Type::MEDIAELEMENT, (create_item_func) media_element_new);
	rdoe (dem, "MediaAttribute", NULL, Type::MEDIAATTRIBUTE, (create_item_func) media_attribute_new);
	
	///
	/// Text
	///
	
	XamlElementInfo *in = register_ghost_element ("Inline", NULL, Type::INLINE);
	XamlElementInfo *txtblk = rdoe (dem, "TextBlock", fw, Type::TEXTBLOCK, (create_item_func) text_block_new);
	txtblk->content_property = "Inlines";

	rdoe (dem, "Inlines", col, Type::INLINES, (create_item_func) inlines_new);

	XamlElementInfo *run = rdoe (dem, "Run", in, Type::RUN, (create_item_func) run_new);
	run->content_property = "Text";
	rdoe (dem, "LineBreak", in, Type::LINEBREAK, (create_item_func) line_break_new);
	rdoe (dem, "Glyphs", fw, Type::GLYPHS, (create_item_func) glyphs_new);


#undef rdoe
	
	default_namespace = new DefaultNamespace (dem);
	x_namespace = new XNamespace ();

	
	///
	/// ENUMS
	///

	g_hash_table_insert (enum_map, (char *) "AlignmentX", GINT_TO_POINTER (alignment_x_map));
	g_hash_table_insert (enum_map, (char *) "AlignmentY", GINT_TO_POINTER (&alignment_y_map));
	g_hash_table_insert (enum_map, (char *) "MappingMode", GINT_TO_POINTER (brush_mapping_mode_map));
	g_hash_table_insert (enum_map, (char *) "ColorInterpolationMode", GINT_TO_POINTER (color_interpolation_mode_map));
	g_hash_table_insert (enum_map, (char *) "Cursors", GINT_TO_POINTER (cursors_map));
	g_hash_table_insert (enum_map, (char *) "ErrorType", GINT_TO_POINTER (error_type_map));
	g_hash_table_insert (enum_map, (char *) "FillBehavior", GINT_TO_POINTER (fill_behavior_map));
	g_hash_table_insert (enum_map, (char *) "FillRule", GINT_TO_POINTER (fill_rule_map));
	g_hash_table_insert (enum_map, (char *) "FontStretch", GINT_TO_POINTER (font_stretches_map));
	g_hash_table_insert (enum_map, (char *) "FontStyle", GINT_TO_POINTER (font_styles_map));
	g_hash_table_insert (enum_map, (char *) "FontWeight", GINT_TO_POINTER (font_weights_map));
	g_hash_table_insert (enum_map, (char *) "SpreadMethod", GINT_TO_POINTER (gradient_spread_method_map));

	g_hash_table_insert (enum_map, (char *) "StrokeDashCap", GINT_TO_POINTER (pen_line_cap_map));
	g_hash_table_insert (enum_map, (char *) "StrokeStartLineCap", GINT_TO_POINTER (pen_line_cap_map));
	g_hash_table_insert (enum_map, (char *) "StrokeEndLineCap", GINT_TO_POINTER (pen_line_cap_map));
	
	g_hash_table_insert (enum_map, (char *) "StrokeLineJoin", GINT_TO_POINTER (pen_line_join_map));
	g_hash_table_insert (enum_map, (char *) "Stretch", GINT_TO_POINTER (stretch_map));
	g_hash_table_insert (enum_map, (char *) "SweepDirection", GINT_TO_POINTER (sweep_direction_map));
	g_hash_table_insert (enum_map, (char *) "DeviceType", GINT_TO_POINTER (tablet_device_type_map));
	g_hash_table_insert (enum_map, (char *) "TextDecorations", GINT_TO_POINTER (text_decorations_map));
	g_hash_table_insert (enum_map, (char *) "TextWrapping", GINT_TO_POINTER (text_wrapping_map));
	g_hash_table_insert (enum_map, (char *) "Visibility", GINT_TO_POINTER (visibility_map));
}
