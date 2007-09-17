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

G_BEGIN_DECLS

DependencyObject  *xaml_create_from_file (XamlLoader* loader, const char *xaml, bool create_namescope, Type::Kind *element_type);
DependencyObject  *xaml_create_from_str  (XamlLoader* loader, const char *xaml, bool create_namescope, Type::Kind *element_type);
void        xaml_set_property_from_str (DependencyObject *obj, DependencyProperty *prop, const char *value);

void        xaml_init (void);

gint64		timespan_from_str (const char *str);

G_END_DECLS

class XamlLoader
{
	private:
		Surface* surface;
		char* filename;
		char* str;
		
	protected:
		XamlLoader (const char* filename, const char* str, Surface* surface);
		
	public:		
		virtual ~XamlLoader ();
		virtual DependencyObject* CreateElement (const char* xmlns, const char* name);
		virtual void SetAttribute (void* target, const char* name, const char* value);
		virtual void HookupEvent (void* target, const char* name, const char* value);
		
		char* GetFilename () { return filename; }
		char* GetString () { return str; }
		Surface* GetSurface () { return surface; }
};

#endif /* __MOON_XAML_H__ */
