/*
 * xaml.h: xaml parser
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef __MOON_XAML_H__
#define __MOON_XAML_H__

#include "enums.h"
#include "uielement.h"

class XamlLoader;

typedef DependencyObject *plugin_create_custom_element_callback (const char *xmlns, const char *name);
typedef void plugin_set_custom_attribute_callback (void *target, const char *name, const char *value);
typedef void plugin_hookup_event_callback (void *target, const char *ename, const char *evalue);

G_BEGIN_DECLS

DependencyObject  *xaml_create_from_file (XamlLoader* loader, const char *xaml, bool create_namescope, Type::Kind *element_type);
DependencyObject  *xaml_create_from_str  (XamlLoader* loader, const char *xaml, bool create_namescope, Type::Kind *element_type);
void        xaml_set_property_from_str (DependencyObject *obj, DependencyProperty *prop, const char *value);

void xaml_set_parser_callbacks (XamlLoader* loader, plugin_create_custom_element_callback *cecb, 
	plugin_set_custom_attribute_callback *sca, plugin_hookup_event_callback *hue);

void        xaml_init (void);

gint64		timespan_from_str (const char *str);
XamlLoader* xaml_loader_new (const char* filename, const char* str, Surface* surface);
void		xaml_loader_free (XamlLoader* loader);

G_END_DECLS

class XamlLoader
{
	private:
		Surface* surface;
		char* filename;
		char* str;
		
	public:
		XamlLoader (const char* filename, const char* str, Surface* surface);
		virtual ~XamlLoader ();
		virtual DependencyObject* CreateElement (const char* xmlns, const char* name);
		virtual void SetAttribute (void* target, const char* name, const char* value);
		virtual void HookupEvent (void* target, const char* name, const char* value);
		
		char* GetFilename () { return filename; }
		char* GetString () { return str; }
		Surface* GetSurface () { return surface; }
		
	public:
		plugin_create_custom_element_callback *create_element_callback;
		plugin_set_custom_attribute_callback *set_attribute_callback;
		plugin_hookup_event_callback *hookup_event_callback;
};

#endif /* __MOON_XAML_H__ */
