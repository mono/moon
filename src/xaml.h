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

namespace Moonlight {

class XamlLoader;
class SL3XamlLoader;

struct XamlCallbackData {
	void *loader;
	void *parser;
	Value *top_level;
	int flags;

	XamlCallbackData (void *loader, void *parser, Value *top_level, int flags = 0)
	{
		this->loader = loader;
		this->parser = parser;
		this->top_level = top_level;
		this->flags = flags;
	}

	enum XamlCallbackFlagsEnum {
		NONE,
		SETTING_DELAYED_PROPERTY = 2
	};
};

typedef bool (*xaml_lookup_object_callback) (XamlCallbackData *data, Value *parent, const char *xmlns, const char *name, bool create, bool is_property, Value *value, MoonError *error);
typedef bool (*xaml_set_property_callback) (XamlCallbackData *data, const char* xmlns, Value *target, void *target_data, Value *target_parent, const char *prop_xmlns, const char *name, Value *value, void *value_data, MoonError *error);
typedef bool (*xaml_import_xaml_xmlns_callback) (XamlCallbackData *data, const char* xmlns, MoonError *error);
typedef bool (*xaml_add_child_callback) (XamlCallbackData *data, Value *parent_parent, bool parent_is_property, const char* parent_xmlns, Value *parent, void *parent_data, Value *child, void *child_data, MoonError *error);

struct XamlLoaderCallbacks {
	GCHandle gchandle;
	xaml_lookup_object_callback lookup_object;
	xaml_set_property_callback set_property;
	xaml_import_xaml_xmlns_callback import_xaml_xmlns;
	xaml_add_child_callback add_child;

	XamlLoaderCallbacks () :
		lookup_object (NULL),
		set_property (NULL),
		import_xaml_xmlns (NULL),
		add_child (NULL)
	{
	}
};

//
// Used by the templates
//

class XamlContextInternal;


class XamlContext : public EventObject {

 private:
	XamlContext *parent;

 protected:
	virtual ~XamlContext ();
	
 public:
	XamlContextInternal *internal;

	/* @SkipFactories */
	XamlContext (XamlContextInternal *internal, XamlContext *parent);

	void SetTemplateBindingSource (DependencyObject *source);

	/* @GeneratePInvoke */
	DependencyObject* GetTemplateBindingSource ();
	/* @GeneratePInvoke */
	FrameworkTemplate* GetSourceTemplate ();
};

/* @CBindingRequisite */
typedef DependencyObject *parse_template_func (Value *data, const Uri *resource_base, Surface *surface, DependencyObject *binding_source, const char *xaml, MoonError *error);

G_BEGIN_DECLS
void        xaml_init (void);
G_END_DECLS

class MOON_API Xaml {
public:
	static bool SetPropertyFromStr (DependencyObject *obj, DependencyProperty *prop, const char *value, MoonError *error);
	static bool TimeSpanFromStr (const char *str, TimeSpan *res);
	/* Managed code must call value_free_value on the result */
	/* @GeneratePInvoke */
	static bool ValueFromStrWithParser (void *p, Type::Kind type, const char *prop_name, const char *str, /* @MarshalAs=IntPtr,IsOut */ Value **v, bool *v_set);
	static bool ConvertPropertyValueToEnumStr (DependencyProperty *prop, Value *v, const char **s);

	static void ParseXmlns (const char *xmlns, char **type_name, char **ns, char **assembly);

	static bool IsValidEventName (Deployment *deployment, Type::Kind kind, const char *name, bool allow_desktop_events);

	static bool BoolFromStr (const char *s, bool *res);

	/* @GeneratePInvoke */
	static XamlLoader *LoaderNew (const Uri *resourceBase, Surface *surface);
	/* @GeneratePInvoke */
	static void LoaderFree (XamlLoader *loader);
	/* @GeneratePInvoke */
	static void LoaderSetCallbacks (SL3XamlLoader *loader, XamlLoaderCallbacks *callbacks);

	/* @GeneratePInvoke */
	static char* UriForPrefix (void *parser, char* prefix);

	/* @GeneratePInvoke */
	static Value*      LookupNamedItem (void *parser, void *element_instance, const char* name);
	/* @GeneratePInvoke */
	static void*       GetTemplateParent (void *parser, void *element_instance);
	/* @GeneratePInvoke */
	static char*       GetElementKey (void *parser, void *element_instance);
	/* @GeneratePInvoke */
	static char*       GetElementName (void *parser, void *element_instance);
	/* @GeneratePInvoke */
	static bool        IsPropertySet (void *parser, void *element_instance, char *name);
	/* @GeneratePInvoke */
	static void        MarkPropertyAsSet (void *parser, void *element_instance, char *name);
	/* @GeneratePInvoke */
	static void        DelaySetProperty (void *parser, void *element_instance, const char *xmlns, const char *name, const Value *value);
};

class XamlLoaderFactory {
 public:
	static XamlLoader *CreateLoader (const Uri* resource_base, Surface *surface);
	static XamlLoader *CreateLoader (const Uri* resource_base, Surface *surface, XamlContext *context);
};

class XamlLoader {

 public:
	
	enum XamlLoaderFlags {
		NONE,
		VALIDATE_TEMPLATES = 2,
		IMPORT_DEFAULT_XMLNS = 4
	};

	DependencyObject* CreateDependencyObjectFromString (const char *xaml, bool create_namescope, Type::Kind *element_type);
	DependencyObject* CreateDependencyObjectFromFile (const char *xaml, bool create_namescope, Type::Kind *element_type);

	/* @GeneratePInvoke */
	virtual Value* CreateFromFileWithError (const char *xaml, bool create_namescope, Type::Kind *element_type, MoonError *error) = 0;

        /* @GeneratePInvoke */
	virtual Value* CreateFromStringWithError  (const char *xaml, bool create_namescope, Type::Kind *element_type, int flags, MoonError *error, DependencyObject* owner = NULL) = 0;

	/* @GeneratePInvoke */
	virtual Value* HydrateFromStringWithError (const char *xaml, Value *obj, bool create_namescope, Type::Kind *element_type, int flags, MoonError *error) = 0;

	virtual ~XamlLoader () {}
};

class SL4XamlLoader : public XamlLoader {

 public:
	SL4XamlLoader (Surface *surface);
	virtual ~SL4XamlLoader ();

	virtual Value* CreateFromFileWithError (const char *xaml, bool create_namescope, Type::Kind *element_type, MoonError *error);
	virtual Value* CreateFromStringWithError  (const char *xaml, bool create_namescope, Type::Kind *element_type, int flags, MoonError *error, DependencyObject* owner = NULL);
	virtual Value* HydrateFromStringWithError (const char *xaml, Value *obj, bool create_namescope, Type::Kind *element_type, int flags, MoonError *error);

 private:
	Surface *surface;
	Deployment *deployment;
};

class SL3XamlLoader : public XamlLoader {
	bool expanding_template;
	DependencyObject *template_owner;
	Surface *surface;
	Uri *resource_base;
	XamlContext *context;
	bool import_default_xmlns;

	void Initialize (const Uri *resourceBase, Surface *surface, XamlContext *context);

 public:

	SL3XamlLoader (Surface *surface, XamlContext *context = NULL);
	SL3XamlLoader (const Uri *resourceBase, Surface *surface, XamlContext *context = NULL);
	
	virtual ~SL3XamlLoader ();
	
	virtual bool LoadVM ();

	virtual bool LookupObject (void *p, Value* top_element, Value* parent, const char* xmlns, const char* name, bool create, bool is_property, Value *value);
	virtual bool SetProperty (void *p, Value *top_level, const char* xmlns, Value *target, void *target_data, Value *target_parent, const char *prop_xmlns, const char *name, Value *value, void *value_data, int flags = 0);
	virtual bool AddChild (void *p, Value *top_level, Value *parent_parent, bool parent_is_property, const char* parent_xmlns, Value *parent, void *parent_data, Value *child, void *child_data);

	// Loaders can override to have the default namespace added, this is for things like plugin.CreateFromXaml
	virtual bool ImportDefaultXmlns () { return import_default_xmlns; }
	void SetImportDefaultXmlns (bool v) { import_default_xmlns = v; }

	Surface *GetSurface () { return surface; }
	const Uri *GetResourceBase () { return resource_base; }

	bool GetExpandingTemplate () { return expanding_template; }
	void SetExpandingTemplate (bool value) { expanding_template = value; }
	
	DependencyObject *GetTemplateOwner () { return template_owner; }
	void SetTemplateOwner (DependencyObject *value) { template_owner = value; }

	/* @GeneratePInvoke */
	XamlContext *GetContext () { return context; }

	bool vm_loaded;

	Value* CreateFromFile (const char *xaml, bool create_namescope, Type::Kind *element_type);
	Value* CreateFromString  (const char *xaml, bool create_namescope, Type::Kind *element_type, int flags);
	Value* HydrateFromString (const char *xaml, Value *object, bool create_namescope, Type::Kind *element_type, int flags);

	/* @GeneratePInvoke */
	Value* CreateFromFileWithError (const char *xaml, bool create_namescope, Type::Kind *element_type, MoonError *error);
	/* @GeneratePInvoke */
	Value* CreateFromStringWithError  (const char *xaml, bool create_namescope, Type::Kind *element_type, int flags, MoonError *error, DependencyObject* owner = NULL);
	/* @GeneratePInvoke */
	Value* HydrateFromStringWithError (const char *xaml, Value *obj, bool create_namescope, Type::Kind *element_type, int flags, MoonError *error);
	
	XamlLoaderCallbacks GetCallbacks ();
	void SetCallbacks (XamlLoaderCallbacks callbacks);

	ParserErrorEventArgs *error_args;
};

};
#endif /* __MOON_XAML_H__ */
