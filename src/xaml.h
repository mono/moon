/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xaml.h: xaml parser
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#ifndef __MOON_XAML_H__
#define __MOON_XAML_H__

#include <glib.h>

#include "enums.h"
#include "uielement.h"
#include "error.h"
#include "value.h"

class XamlLoader;

struct XamlCallbackData {
	void *loader;
	void *parser;
	Value *top_level;

	XamlCallbackData (void *loader, void *parser, Value *top_level)
	{
		this->loader = loader;
		this->parser = parser;
		this->top_level = top_level;
	}
};

typedef bool (*xaml_lookup_object_callback) (XamlCallbackData *data, Value *parent, const char *xmlns, const char *name, bool create, bool is_property, Value *value, MoonError *error);
typedef void (*xaml_create_gchandle_callback) ();
typedef bool (*xaml_set_property_callback) (XamlCallbackData *data, const char* xmlns, Value *target, void *target_data, Value *target_parent, const char *prop_xmlns, const char *name, Value *value, void *value_data, MoonError *error);
typedef bool (*xaml_import_xaml_xmlns_callback) (XamlCallbackData *data, const char* xmlns, MoonError *error);
typedef const char* (*xaml_get_content_property_name_callback) (XamlCallbackData *data, Value *object, MoonError *error);
typedef bool (*xaml_add_child_callback) (XamlCallbackData *data, Value *parent_parent, bool parent_is_property, const char* parent_xmlns, Value *parent, void *parent_data, Value *child, void *child_data, MoonError *error);

struct XamlLoaderCallbacks {

	xaml_lookup_object_callback lookup_object;
	xaml_create_gchandle_callback create_gchandle;
	xaml_set_property_callback set_property;
	xaml_import_xaml_xmlns_callback import_xaml_xmlns;
	xaml_get_content_property_name_callback get_content_property_name;
	xaml_add_child_callback add_child;

	XamlLoaderCallbacks () :
		lookup_object (NULL),
		set_property (NULL),
		import_xaml_xmlns (NULL),
		get_content_property_name (NULL),
		add_child (NULL)
	{
	}
};


//
// Used by the templates
//

class XamlContextInternal;

class XamlContext {

 public:
	XamlContextInternal *internal;

	XamlContext (XamlContextInternal *internal);
	~XamlContext ();

	void SetTemplateBindingSource (DependencyObject *source);

	/* @GenerateCBinding,GeneratePInvoke */
	DependencyObject* GetTemplateBindingSource ();
};


G_BEGIN_DECLS

void        xaml_init (void);

bool        xaml_set_property_from_str (DependencyObject *obj, DependencyProperty *prop, const char *value, MoonError *error);

bool        time_span_from_str (const char *str, TimeSpan *res);
/* @GeneratePInvoke */
bool        value_from_str_with_typename (const char *type_name, const char *prop_name, const char *str, /* @MarshalAs=IntPtr,IsOut */ Value **v);
/* @GeneratePInvoke */
bool        value_from_str (Type::Kind type, const char *prop_name, const char *str, /* @MarshalAs=IntPtr,IsOut */ Value **v);
bool        convert_property_value_to_enum_str (DependencyProperty *prop, Value *v, const char **s);

void	    xaml_parse_xmlns (const char *xmlns, char **type_name, char **ns, char **assembly);

bool        xaml_is_valid_event_name (Type::Kind kind, const char *name, bool allow_desktop_events);

/* @GeneratePInvoke */
XamlLoader *xaml_loader_new (const char *filename, const char *str, Surface *surface);
/* @GeneratePInvoke */
void	    xaml_loader_free (XamlLoader *loader);
/* @GeneratePInvoke */
void        xaml_loader_set_callbacks (XamlLoader *loader, XamlLoaderCallbacks callbacks);

/* @GeneratePInvoke */
char*       xaml_uri_for_prefix (void *parser, char* prefix);


/* @GeneratePInvoke */
Value*      xaml_lookup_named_item (void *parser, void *element_instance, const char* name);
/* @GeneratePInvoke */
void*       xaml_get_template_parent (void *parser, void *element_instance);
/* @GeneratePInvoke */
char*       xaml_get_element_key (void *parser, void *element_instance);
/* @GeneratePInvoke */
char*       xaml_get_element_name (void *parser, void *element_instance);

G_END_DECLS

/*

  Plugin:
    - calls PluginXamlLoader::TryLoad to try to load some xaml.
    -  calls xaml_create_from_*
    -     calls XamlLoader::LookupObject (,) if it encounters xmlns/name
    -      parses the xmlns and name
    -       calls XamlLoader::LoadVM.
    -        PluginXamlLoader::LoadVM will load the vm and create a ManagedXamlLoader (which will set the callbacks in XamlLoader)
    -       calls XamlLoader::CreateObject (,,,) with the parsed xml
    -        calls the create_managed_object callback (if any).
    -          will try to load the assembly, if it fails, it's requested.
    -  if XamlLoader::CreateObject failed, try to download the missing assembly (if any).
    -  if no missing assembly, the xaml load fails.

  Deskop:
    - calls System.Windows.XamlReader::Load
    -  creates a ManagedXamlLoader and a native XamlLoader (setting the callbacks).
    -  calls xaml_create_from_str
    -     calls XamlLoader::CreateObject (,) if it encounters xmlns/name
    -      parses the xmlns and name
    -       calls XamlLoader::LoadVM (which does nothing).
    -       calls XamlLoader::CreateObject (,,,) with the parsed xml
    -        calls the create_managed_object callback (if any).
    -          will try to load the assembly, if it fails, it's requested.
    -    destroy the native/managed XamlLoader. Any requested assemblies are ignored, no retries are done.
*/


class XamlLoader {
	bool expanding_template;
	Surface *surface;
	char *filename;
	char *str;
	XamlContext *context;

 public:

	XamlLoader (const char *filename, const char *str, Surface *surface, XamlContext *context = NULL);
	virtual ~XamlLoader ();
	
	virtual bool LoadVM ();

	virtual bool LookupObject (void *p, Value* top_element, Value* parent, const char* xmlns, const char* name, bool create, bool is_property, Value *value);
	virtual bool SetProperty (void *p, Value *top_level, const char* xmlns, Value *target, void *target_data, Value *target_parent, const char *prop_xmlns, const char *name, Value *value, void *value_data);
	virtual bool AddChild (void *p, Value *top_level, Value *parent_parent, bool parent_is_property, const char* parent_xmlns, Value *parent, void *parent_data, Value *child, void *child_data);

	virtual const char *GetContentPropertyName (void *p, Value *top_level, Value *object);

	char *GetFilename () { return filename; }
	char *GetString () { return str; }
	Surface *GetSurface () { return surface; }
	
	bool GetExpandingTemplate () { return expanding_template; }
	void SetExpandingTemplate (bool value) { expanding_template = value; }

	/* @GenerateCBinding,GeneratePInvoke */
	XamlContext *GetContext () { return context; }

	bool vm_loaded;

	DependencyObject* CreateDependencyObjectFromString (const char *xaml, bool create_namescope, Type::Kind *element_type);
	DependencyObject* CreateDependencyObjectFromFile (const char *xaml, bool create_namescope, Type::Kind *element_type);

	Value* CreateFromFile (const char *xaml, bool create_namescope, Type::Kind *element_type);
	Value* CreateFromString  (const char *xaml, bool create_namescope, bool validate_templates, Type::Kind *element_type);
	Value* HydrateFromString (const char *xaml, DependencyObject *object, bool create_namescope, bool validate_templates, Type::Kind *element_type);

	/* @GenerateCBinding,GeneratePInvoke */
	Value* CreateFromFileWithError (const char *xaml, bool create_namescope, Type::Kind *element_type, MoonError *error);
	/* @GenerateCBinding,GeneratePInvoke */
	Value* CreateFromStringWithError  (const char *xaml, bool create_namescope, bool validate_templates, Type::Kind *element_type, MoonError *error);
	/* @GenerateCBinding,GeneratePInvoke */
	Value* HydrateFromStringWithError (const char *xaml, DependencyObject *obj, bool create_namescope, bool validate_templates, Type::Kind *element_type, MoonError *error);
	
	XamlLoaderCallbacks callbacks;
	ParserErrorEventArgs *error_args;
};


#endif /* __MOON_XAML_H__ */
