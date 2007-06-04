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

#define READ_BUFFER 1024

GHashTable *element_table = NULL;
void xaml_init_element_table ();

int id_count = 0;

class XamlElementInfo {

 public:
	const char *element_name;
	const char *instance_name;

	XamlElementInfo *parent;
	GList *children;

	UIElement *item;

	int id;

	XamlElementInfo () : element_name (NULL), instance_name (NULL), parent (NULL), children (NULL), id (id_count++)
	{
	}

};

class XamlParserInfo {

 public:
	XML_Parser parser;

	XamlElementInfo *top_element;
	XamlElementInfo *current_element;

	XamlParserInfo (XML_Parser parser) : parser (parser), top_element (NULL), current_element (NULL)
	{
	}

};

typedef XamlElementInfo* (*create_element_func) (XamlParserInfo *info, const char *, const char **);


void
start_element_handler (void *data, const char *el, const char **attr)
{
	XamlElementInfo *item;
	XamlParserInfo *info = (XamlParserInfo *) data;
	create_element_func create_element = (create_element_func) g_hash_table_lookup (element_table, el);

	if (create_element) {
		item = create_element (info, el, attr);

 		if (!info->top_element) {
			info->top_element = item;
		} else {
			item->parent = info->current_element;
			info->current_element->children = g_list_append (info->current_element->children, item);

			if (info->current_element->item->flags & UIElement::IS_SURFACE != 0)
				panel_child_add ((Panel *) info->current_element->item, item->item);
		}

		info->current_element = item;
		return;
	}
}

void
end_element_handler (void *data, const char *el)
{
	XamlParserInfo *info = (XamlParserInfo *) data;	

	info->current_element = info->current_element->parent;
}

void
char_data_handler (void *data, const char *txt, int len)
{
	
}

void
free_recursive (XamlElementInfo *el)
{
	
}

void
print_tree (XamlElementInfo *el, int depth)
{
	for (int i = 0; i < depth; i++)
		printf ("\t");
	printf ("%s  (%d)\n", el->element_name, g_list_length (el->children));

	for (GList *walk = el->children; walk != NULL; walk = walk->next) {
		XamlElementInfo *el = (XamlElementInfo *) walk->data;
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

	printf ("parsing xaml file:  %s\n", xaml_file);
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

	if (!element_table)
		xaml_init_element_table ();

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

	print_tree (parser_info->top_element, 0);

	if (parser_info->top_element) {
		UIElement *res = (UIElement *) parser_info->top_element->item;
		free_recursive (parser_info->top_element);
		return res;
	}

	return NULL;
}

int
xaml_attribute_as_int (const char **attr, const char *name, int def)
{
	int i;

	for (i = 0; attr [i]; i += 2) {
		if (!g_strcasecmp (attr [i], name)) {
			return (int) strtoll (attr [i + 1], NULL, 10);
		}
	}
	return def;
}

XamlElementInfo *
create_canvas_from_element (XamlParserInfo *info, const char *el, const char **attr)
{
	Canvas *canvas = new Canvas ();
	XamlElementInfo *res = new XamlElementInfo ();

	res->item = canvas;
	res->element_name = el;
	res->instance_name = NULL;	

	int count = XML_GetSpecifiedAttributeCount (info->parser);
	for (int i = 0; attr [i]; i += 2)
		canvas->set_prop_from_str (attr [i], attr [i + 1]);

	return res;
}

XamlElementInfo *
create_rectangle_from_element (XamlParserInfo *info, const char *el, const char **attr)
{
	Rectangle *rectangle = rectangle_new (0, 0, 0, 0);
	XamlElementInfo *res = new XamlElementInfo ();

	res->item = rectangle;
	res->element_name = el;
	res->instance_name = NULL;

	int count = XML_GetSpecifiedAttributeCount (info->parser);
	for (int i = 0; attr [i]; i += 2)
		rectangle->set_prop_from_str (attr [i], attr [i + 1]);

	return res;
}

XamlElementInfo *
create_line_from_element (XamlParserInfo *info, const char *el, const char **attr)
{
	Line *line = line_new (0, 0, 0, 0);
	XamlElementInfo *res = new XamlElementInfo ();

	res->item = line;
	res->element_name = el;
	res->instance_name = NULL;

	int count = XML_GetSpecifiedAttributeCount (info->parser);
	for (int i = 0; attr [i]; i += 2)
		line->set_prop_from_str (attr [i], attr [i + 1]);

	return res;
}

void
xaml_init_element_table ()
{
	element_table = g_hash_table_new (g_str_hash, g_str_equal);

	g_hash_table_insert (element_table, (char *) "Canvas", GINT_TO_POINTER (create_canvas_from_element));
	g_hash_table_insert (element_table, (char *) "Rectangle", GINT_TO_POINTER (create_rectangle_from_element));
	g_hash_table_insert (element_table, (char *) "Line", GINT_TO_POINTER (create_line_from_element));
	
}
