/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * xaml.cpp: xaml parser
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include <glib.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include <expat.h>

#include "xaml.h"
#include "error.h"
#include "shape.h"
#include "animation.h"
#include "bitmapimage.h"
#include "geometry.h"
#include "projection.h"
#include "textblock.h"
#include "glyphs.h"
#include "media.h"
#include "list.h"
#include "rect.h"
#include "point.h"
#include "canvas.h"
#include "color.h"
#include "namescope.h"
#include "stylus.h"
#include "runtime.h"
#include "utils.h"
#include "control.h"
#include "template.h"
#include "style.h"
#include "application.h"
#include "thickness.h"
#include "cornerradius.h"
#include "deployment.h"
#include "grid.h"
#include "deepzoomimagetilesource.h"
#include "managedtypeinfo.h"
#include "bitmapcache.h"

class XamlElementInfo;
class XamlElementInstance;
class XamlParserInfo;
class XamlNamespace;
class DefaultNamespace;
class XNamespace;
class XmlNamespace;
class PrimitiveNamespace;
class MCIgnorableNamespace;
class XamlElementInfoNative;
class XamlElementInstanceNative;
class XamlElementInstanceValueType;
class XamlElementInfoEnum;
class XamlElementInstanceEnum;
class XamlElementInfoManaged;
class XamlElementInstanceManaged;
class XamlElementInfoImportedManaged;
class XamlElementInstanceTemplate;

#define INTERNAL_IGNORABLE_ELEMENT "MoonlightInternalIgnorableElement"

#define IS_NULL_OR_EMPTY(str)	(!str || (*str == 0))

static DefaultNamespace *default_namespace = NULL;
static XNamespace *x_namespace = NULL;
static XmlNamespace *xml_namespace = NULL;

static const char* default_namespace_names [] = {
	"http://schemas.microsoft.com/winfx/2006/xaml/presentation",
	"http://schemas.microsoft.com/client/2007",
	"http://schemas.microsoft.com/xps/2005/06",
	"http://schemas.microsoft.com/client/2007/deployment",
	NULL
};

#define X_NAMESPACE_URI "http://schemas.microsoft.com/winfx/2006/xaml"
#define XML_NAMESPACE_URI "http://www.w3.org/XML/1998/namespace"
#define PRIMITIVE_NAMESPACE_URI "clr-namespace:System;assembly=mscorlib"
#define MC_IGNORABLE_NAMESPACE_URI "http://schemas.openxmlformats.org/markup-compatibility/2006"


static bool value_from_str_with_parser (XamlParserInfo *p, Type::Kind type, const char *prop_name, const char *str, Value **v, bool *v_set);
static bool dependency_object_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value, bool raise_errors);
static bool set_managed_attached_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);
static void dependency_object_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child, bool fail_if_no_prop);
static void dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr);
static void value_type_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr);
static bool element_begins_buffering (Type::Kind kind);
static bool is_managed_kind (Type::Kind kind);
static bool kind_requires_managed_load (Type::Kind kind);
static bool is_legal_top_level_kind (Type::Kind kind);
static Value *lookup_resource_dictionary (ResourceDictionary *rd, const char *name, bool *exists);
static void parser_error (XamlParserInfo *p, const char *el, const char *attr, int error_code, const char *format, ...);
static gboolean namespace_for_prefix (gpointer key, gpointer value, gpointer user_data);

static XamlElementInfo *create_element_info_from_imported_managed_type (XamlParserInfo *p, const char *name, const char **attr, bool create);
static void destroy_created_namespace (gpointer data, gpointer user_data);

enum BufferMode {
	BUFFER_MODE_TEMPLATE,
	BUFFER_MODE_IGNORE
};


class XamlNamespace {
 public:
	const char *name;
	bool is_ignored;
	GSList *prefixes;

	XamlNamespace ()
	{
		name = NULL;
		prefixes = NULL;
		is_ignored = false;
	}

	~XamlNamespace ()
	{
		if (prefixes) {
			GSList *w = prefixes;

			while (w) {
				char *p = (char *) w->data;

				g_free (p);
				w = w->next;
			}

			g_slist_free (prefixes);
			prefixes = NULL;
		}
	}

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el, const char **attr, bool create) = 0;
	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value) = 0;

	
	virtual const char* GetUri () = 0;

	void AddPrefix (const char *prefix)
	{
		prefixes = g_slist_append (prefixes, g_strdup (prefix));
	}

	bool HasPrefix (const char *prefix)
	{
		return g_slist_find_custom (prefixes, prefix, (GCompareFunc) strcmp) != NULL;
	}

	GSList* GetPrefixes ()
	{
		return prefixes;
	}
};

void
add_namespace_data (gpointer key, gpointer value, gpointer user_data)
{
	XamlNamespace *ns = (XamlNamespace *) value;
	GHashTable *table = (GHashTable *) user_data;

	if ((void *)ns != (void *)default_namespace) {
		GSList *p = ns->GetPrefixes ();

		while (p) {
			g_hash_table_insert (table, g_strdup ((char *)p->data), g_strdup (ns->GetUri ()));

			p = p->next;
		}
	}
}

void
add_namespace_to_ignorable (gpointer key, gpointer value, gpointer user_data)
{
	char *prefix = (char *) key;
	char *uri = (char *) value;
	GString *str = (GString *) user_data;

	g_string_append_printf (str, "xmlns:%s=\"%s\" ", prefix, uri);
}

class XamlContextInternal {

 public:
	Value *top_element;
	FrameworkTemplate *template_parent;
	GHashTable *imported_namespaces;
	Surface *surface;
	XamlLoaderCallbacks callbacks;
	GSList *resources;
	XamlContextInternal *parent_context;

	DependencyObject *source;

	XamlContextInternal (XamlLoaderCallbacks callbacks, Value *top_element, FrameworkTemplate *template_parent, GHashTable *namespaces, GSList *resources, XamlContextInternal *parent_context)
	{
		this->callbacks = callbacks;
		this->top_element = new Value (*top_element);
		this->template_parent = template_parent;
		this->surface = template_parent->GetSurface ();
		this->resources = resources;
		this->parent_context = parent_context;

		if (this->callbacks.create_gchandle) 
			this->callbacks.create_gchandle ();
		imported_namespaces = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
		g_hash_table_foreach (namespaces, add_namespace_data, imported_namespaces);
	}

	
	~XamlContextInternal ()
	{
		if (imported_namespaces)
			g_hash_table_destroy (imported_namespaces);
		if (resources)
			g_slist_free (resources);
		delete top_element;
	}

	char *CreateIgnorableTagOpen ()
	{
		GString *str = g_string_new ("<" INTERNAL_IGNORABLE_ELEMENT " ");
		g_hash_table_foreach (imported_namespaces, add_namespace_to_ignorable, str);

		str = g_string_append (str, ">");

		char *res = str->str;
		g_string_free (str, false);

		return res;
	}

	char *CreateIgnorableTagClose ()
	{
		return g_strdup ("</" INTERNAL_IGNORABLE_ELEMENT ">");
	}

	bool LookupNamedItem (const char* name, Value **v)
	{
		if (!resources)
			return NULL;

		bool exists = false;
		GSList *walk = resources;
		while (walk) {
			DependencyObject *dob = (DependencyObject*)walk->data;
			if (dob->Is (Type::RESOURCE_DICTIONARY))
				*v = lookup_resource_dictionary ((ResourceDictionary *) walk->data, name, &exists);
			else /* dob->Is (Type::FRAMEWORKELEMENT) */ {
				ResourceDictionary *rd = dob->GetValue (UIElement::ResourcesProperty)->AsResourceDictionary();
				*v = lookup_resource_dictionary (rd, name, &exists);
			}

			if (exists)
				break;
			walk = walk->next;
		}

		if (exists)
			return exists;
		else if (!parent_context)
			return false;

		return parent_context->LookupNamedItem (name, v);
	}

	void SetTemplateBindingSource (DependencyObject *source)
	{
		this->source = source;
	}

	DependencyObject* GetTemplateBindingSource ()
	{
		return source;
	}
};


XamlContext::XamlContext (XamlContextInternal *internal)
{
	this->internal = internal;
}

XamlContext::~XamlContext ()
{
	delete internal;
}

void
XamlContext::SetTemplateBindingSource (DependencyObject *source)
{
	internal->SetTemplateBindingSource (source);
}

DependencyObject*
XamlContext::GetTemplateBindingSource ()
{
	return internal->GetTemplateBindingSource ();
}

class XamlElementInfo {
 protected:
	Type::Kind kind;
	Type::Kind property_owner_kind;
	bool cdata_verbatim;

 public:
	XamlElementInfo *parent;
	const char *name;
	const char *xmlns;

	XamlElementInfo (const char *xmlns, const char *name, Type::Kind kind)
	{
		this->parent = NULL;
		this->kind = kind;
		this->name = name;
		this->xmlns = xmlns;
		this->cdata_verbatim = false;

		this->property_owner_kind = Type::INVALID;
	}

	~XamlElementInfo ()
	{
	}

	virtual Type::Kind GetKind () { return kind; }

	virtual void SetPropertyOwnerKind (Type::Kind value) { property_owner_kind = value; }
	virtual Type::Kind GetPropertyOwnerKind () { return property_owner_kind; }

	virtual const char *GetContentProperty (XamlParserInfo *p)
	{
		Type *t = Type::Find (Deployment::GetCurrent (), kind);
		if (t)
			return t->GetContentPropertyName ();
		return NULL;
	}

	void SetIsCDataVerbatim (bool flag)
	{
		cdata_verbatim = flag;
	}

	bool IsCDataVerbatim ()
	{
		return cdata_verbatim;
	}

	virtual bool RequiresManagedSet () { return false; }

	virtual XamlElementInstance *CreateElementInstance (XamlParserInfo *p) = 0;
	virtual XamlElementInstance *CreateWrappedElementInstance (XamlParserInfo *p, Value *o) = 0;
	virtual XamlElementInstance *CreatePropertyElementInstance (XamlParserInfo *p, const char *name) = 0;
};


struct DelayedProperty {
	char *xmlns;
	char *name;
	Value *value;

	DelayedProperty (const char *xmlns, const char *name, const Value *value)
	{
		this->xmlns = g_strdup (xmlns);
		this->name = g_strdup (name);
		this->value = new Value (*value);
	}

	~DelayedProperty ()
	{
		g_free (xmlns);
		g_free (name);
		delete value;
	}
};

static void
free_property_list (GSList *list)
{
	GSList *walk = list;

	while (walk) {
		DelayedProperty *prop = (DelayedProperty *) walk->data;

		delete prop;
		walk = walk->next;
	}

	g_slist_free (list);
}

class XamlElementInstance : public List::Node {

 protected:
	DependencyObject *item;
	Value *value;
	bool cleanup_value;
	GSList *delayed_properties;

 public:
	const char *element_name;
	XamlElementInfo *info;
	
	XamlElementInstance *parent;
	List *children;

	enum ElementType {
		ELEMENT,
		PROPERTY,
		INVALID
	};

	int element_type;
	bool requires_managed;
	char *x_key;
	char *x_name;

	GHashTable *set_properties;

	XamlElementInstance (XamlElementInfo *info, const char* element_name, ElementType type, bool requires_managed = false)
	{
		this->element_name = element_name;
		this->set_properties = NULL;
		this->element_type = type;
		this->parent = NULL;
		this->info = info;
		this->item = NULL;
		this->value = NULL;
		this->x_key = NULL;
		this->x_name = NULL;
		this->cleanup_value = true;
		this->requires_managed = requires_managed;
		this->delayed_properties = NULL;
		
		children = new List ();
	}
	
	virtual ~XamlElementInstance ()
	{
		children->Clear (true);
		delete children;
		delete info;

		g_free (x_key);
		g_free (x_name);

		if (cleanup_value)
			delete value;

		if (set_properties)
			g_hash_table_destroy (set_properties);

		if (element_name && element_type == PROPERTY)
			g_free ((void*) element_name);

		free_property_list (delayed_properties);
	}

	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value) = 0;
	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, const char* value) = 0;
	virtual void AddChild (XamlParserInfo *p, XamlElementInstance *child) = 0;
	virtual void SetAttributes (XamlParserInfo *p, const char **attr) = 0;

	virtual bool TrySetContentProperty (XamlParserInfo *p, XamlElementInstance *value);
	virtual bool TrySetContentProperty (XamlParserInfo *p, const char *value);
	
	
	char *GetKey () { return x_key; }
	char *GetName () { return x_name; }

	void SetName (XamlParserInfo *p, const char *name)
	{
		this->x_name = g_strdup (name);
	}

	void SetKey (XamlParserInfo *p, const char *key)
	{
		this->x_key = g_strdup (key);
	}

	virtual bool IsDependencyObject ()
	{
		return true;
	}

	virtual bool SetUnknownAttribute (XamlParserInfo *p, const char* name, const char* value);

	void SetValue (Value *v)
	{
		if (value && cleanup_value)
			delete value;
		value = v;
	}

	virtual Value *GetAsValue ()
	{
		if (!value) {
			value = new Value (item);
		}
		return value;
	}

	virtual DependencyObject *GetAsDependencyObject ()
	{
		return item;
	}

	virtual void SetDependencyObject (DependencyObject *value)
	{
		item = value;
	}
	
	virtual void* GetManagedPointer ()
	{
		return item;
	}

	virtual Value* GetParentPointer ()
	{
		XamlElementInstance *walk = parent;
		while (walk && walk->element_type != XamlElementInstance::ELEMENT)
			walk = walk->parent;

		if (!walk)
			return NULL;

		return walk->GetAsValue ();
	}

	virtual bool IsTemplate ()
	{
		return false;
	}

	virtual XamlElementInfo* FindPropertyElement (XamlParserInfo *p, const char *el, const char *dot);

	void SetDelayedProperties (XamlParserInfo *p);

	void DelaySetProperty (const char *xmlns, const char *name, const Value *value)
	{
		DelayedProperty *prop = new DelayedProperty (xmlns, name, value);

		delayed_properties = g_slist_append (delayed_properties, prop);
	}
	
	bool IsPropertySet (const char *name)
	{
		if (!set_properties)
			return false;

		return g_hash_table_lookup (set_properties, name) != NULL;
	}

	void MarkPropertyAsSet (const char *name)
	{
		if (!set_properties)
			set_properties = g_hash_table_new (g_str_hash, g_str_equal);

		g_hash_table_insert (set_properties, (void *) name, GINT_TO_POINTER (TRUE));
	}
};

void 
unref_xaml_element (gpointer data, gpointer user_data)
{
	DependencyObject* dob = (DependencyObject*) data;
	//printf ("unref_xaml_element: %i\n", dob->id);
	if (dob)
		dob->unref ();
}

class XamlParserInfo {
 public:
	XML_Parser parser;

	const char *file_name;

	NameScope *namescope;
	XamlElementInstance *top_element;
	XamlNamespace *current_namespace;
	XamlElementInstance *current_element;
	const char *next_element;
	Deployment *deployment;

	GHashTable *namespace_map;
	bool cdata_content;
	GString *cdata;
	
	bool implicit_default_namespace;

	ParserErrorEventArgs *error_args;

	XamlLoader *loader;

	
        //
	// If set, this is used to hydrate an existing object, not to create a new toplevel one
	//
	Value *hydrate_expecting;
	bool hydrating;

	char* buffer_until_element;
	int buffer_depth;
	BufferMode buffer_mode;
	GString *buffer;
	bool validate_templates;

 private:
	GList *created_elements;
	GList *created_namespaces;
	const char* xml_buffer;
	int multi_buffer_offset;
	int xml_buffer_start_index;

 public:
	XamlParserInfo (XML_Parser parser, const char *file_name)
	{
		this->deployment = Deployment::GetCurrent ();
		this->parser = parser;
		this->file_name = file_name;
		this->namescope = new NameScope ();

		top_element = NULL;
		current_namespace = NULL;
		current_element = NULL;
		cdata_content = false;
		cdata = NULL;
		implicit_default_namespace = false;
		error_args = NULL;
		loader = NULL;
		created_elements = NULL;
		created_namespaces = NULL;
		hydrate_expecting = NULL;
		hydrating = false;

		buffer_until_element = NULL;
		buffer_depth = -1;
		buffer = NULL;
		xml_buffer = NULL;
		multi_buffer_offset = 0;
		validate_templates = false;

		namespace_map = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	}

	void AddCreatedElement (DependencyObject* element)
	{
		// if we have a loader, set the surface and base resource location
		if (loader) {
			element->SetSurface (loader->GetSurface());
			element->SetResourceBase (loader->GetResourceBase());
		}

		// When instantiating a template, some elements are created which are not explicitly
		// mentioned in the xaml. Therefore we need to keep walking up the tree until we find
		// the last element which we set a value for Control::IsTemplateItem and propagate
		// it from there.
		XamlElementInstance *instance = current_element;
		while (instance) {
			if (!instance->IsDependencyObject () || !instance->GetAsDependencyObject ()) {
				instance = instance->parent;
				continue;
			}
			if (!instance->GetAsDependencyObject ()->ReadLocalValue (Control::IsTemplateItemProperty)) {
				instance = instance->parent;
				continue;
			}
			Control::SetIsTemplateItem (element, Control::GetIsTemplateItem (instance->GetAsDependencyObject ()));
			if (DependencyObject *e = instance->GetAsDependencyObject ()->GetTemplateOwner ())
				element->SetTemplateOwner (e);
			break;
		}
		
		if (instance == NULL) {
			Control::SetIsTemplateItem (element, loader->GetExpandingTemplate ());
			element->SetTemplateOwner (loader->GetTemplateOwner ());
		}

		if (Control::GetIsTemplateItem (element))
			NameScope::SetNameScope (element, namescope);
		created_elements = g_list_prepend (created_elements, element);
	}

	void AddCreatedNamespace (XamlNamespace* ns)
	{
		created_namespaces = g_list_prepend (created_namespaces, ns);
	}

	void QueueBeginBuffering (char* buffer_until, BufferMode mode)
	{
		buffer_until_element = buffer_until;
		buffer_depth = 1;
		buffer_mode = mode;

		xml_buffer_start_index = -1;
	}

	void BeginBuffering ()
	{
		xml_buffer_start_index = XML_GetCurrentByteIndex (parser) - multi_buffer_offset;
		buffer = g_string_new (NULL);
	}

	bool ShouldBeginBuffering ()
	{
		return InBufferingMode () && xml_buffer_start_index == -1;
	}

	bool InBufferingMode ()
	{
		return buffer_until_element != NULL;
	}

	void AppendCurrentXml ()
	{
		if (!buffer)
			return;
		int pos = XML_GetCurrentByteIndex (parser) - multi_buffer_offset;
		g_string_append_len (buffer, xml_buffer + xml_buffer_start_index, pos - xml_buffer_start_index);
	}
	
	char* ClearBuffer ()
	{
		AppendCurrentXml ();

		buffer_depth = 0;
		buffer_until_element = NULL;

		if (!buffer)
			return g_strdup ("");

		char* res = buffer->str;
		g_string_free (buffer, FALSE);
		buffer = NULL;
		return res;
	}

	void SetXmlBuffer (const char* xml_buffer)
	{
		if (InBufferingMode ())
			AppendCurrentXml ();

		if (this->xml_buffer)
			multi_buffer_offset += strlen (this->xml_buffer);

		this->xml_buffer = xml_buffer;
		xml_buffer_start_index = 0;
	}

	void ValidateTemplate (const char* buffer, XamlContext* context, FrameworkTemplate *binding_source)
	{
		XamlLoader *loader = new XamlLoader (NULL, buffer, NULL, context);
		Type::Kind dummy;

		context->SetTemplateBindingSource (binding_source);

		loader->SetImportDefaultXmlns (true);

		MoonError error;
		Value *result = loader->CreateFromStringWithError (buffer, true, &dummy, XamlLoader::IMPORT_DEFAULT_XMLNS | XamlLoader::VALIDATE_TEMPLATES, &error);

		delete result;
		delete loader;

		if (error.number != MoonError::NO_ERROR) {
			int line_number = error.line_number + XML_GetCurrentLineNumber (parser);
			error_args = new ParserErrorEventArgs (error.message, file_name, line_number, error.char_position, error.code, NULL, NULL);
		}
	}

	FrameworkTemplate *GetTemplateParent (XamlElementInstance *item)
	{
		XamlElementInstance *parent = item->parent;

		while (parent && !parent->IsTemplate ())
			parent = parent->parent;
		
		if (parent)
			return (FrameworkTemplate *) parent->GetManagedPointer ();

		if (!loader)
			return NULL;

		XamlContext *context = loader->GetContext ();
		if (!context)
			return NULL;

		return context->internal->template_parent;
	}

	Value *GetTopElementPtr ()
	{
		XamlContext *context = loader->GetContext ();
		if (context)
			return context->internal->top_element;

		if (top_element)
			return top_element->GetAsValue ();
 
		return NULL;
	}

	~XamlParserInfo ()
	{
		created_elements = g_list_reverse (created_elements);
		g_list_foreach (created_elements, unref_xaml_element, NULL);
		g_list_free (created_elements);

		g_list_foreach (created_namespaces, destroy_created_namespace, NULL);
		g_list_free (created_namespaces);

		g_hash_table_destroy (namespace_map);

		if (cdata)
			g_string_free (cdata, TRUE);
		if (top_element)
			delete top_element;
		namescope->unref ();
	}

 
};


class XamlElementInfoNative : public XamlElementInfo {
	Type *type;
	
 public:
	XamlElementInfoNative (Type *t) : XamlElementInfo (NULL, t->GetName (), t->GetKind ())
	{
		type = t;
	}

	Type* GetType ()
	{
		return type;
	}

	const char* GetName ()
	{
		return type->GetName ();
	}

	const char* GetContentProperty (XamlParserInfo *p);

	XamlElementInstance* CreateElementInstance (XamlParserInfo *p);
	XamlElementInstance* CreateWrappedElementInstance (XamlParserInfo *p, Value *o);
	XamlElementInstance* CreatePropertyElementInstance (XamlParserInfo *p, const char *name);
};


class XamlElementInstanceNative : public XamlElementInstance {
	XamlElementInfoNative *element_info;
	XamlParserInfo *parser_info;
	
 public:
	XamlElementInstanceNative (XamlElementInfoNative *element_info, XamlParserInfo *parser_info, const char *name, ElementType type, bool create_item = true);

	virtual DependencyObject *CreateItem ();

	virtual XamlElementInfo* FindPropertyElement (XamlParserInfo *p, const char *el, const char *dot);
	
	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value);
	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, const char* value);
	virtual void AddChild (XamlParserInfo *p, XamlElementInstance *child);
	virtual void SetAttributes (XamlParserInfo *p, const char **attr);
};


class XamlElementInstanceValueType : public XamlElementInstance {
	XamlElementInfoNative *element_info;
	XamlParserInfo *parser_info;

 public:
	XamlElementInstanceValueType (XamlElementInfoNative *element_info, XamlParserInfo *parser_info, const char *name, ElementType type);

	virtual bool IsDependencyObject ()
	{
		return false;
	}

	virtual Value *GetAsValue ()
	{
		if (value == NULL) {
			// we are an empty element (e.g.  <sys:String></sys:String>).  do type specific magic here.
			CreateValueItemFromString ("");
		}

		return value;
	}

	bool CreateValueItemFromString (const char* str);

	// A Value type doesn't really support anything. It's just here so people can do <SolidColorBrush.Color><Color>#FF00FF</Color></SolidColorBrush.Color>
	virtual DependencyObject *CreateItem () { return NULL; }
	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value) { return false; }
	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, const char *value) { return false; }
	virtual void AddChild (XamlParserInfo *p, XamlElementInstance *child) { }
	virtual void SetAttributes (XamlParserInfo *p, const char **attr);

	virtual bool TrySetContentProperty (XamlParserInfo *p, XamlElementInstance *value) { return false; }
	virtual bool TrySetContentProperty (XamlParserInfo *p, const char *value) { return CreateValueItemFromString (value); }
};

class XamlElementInfoEnum : public XamlElementInfo {
 public:
	XamlElementInfoEnum (const char *name) : XamlElementInfo (NULL, name, Type::INT32)
	{
	}

	XamlElementInstance* CreateElementInstance (XamlParserInfo *p);
	XamlElementInstance* CreateWrappedElementInstance (XamlParserInfo *p, Value *o);
	XamlElementInstance* CreatePropertyElementInstance (XamlParserInfo *p, const char *name) { return NULL; }
};

class XamlElementInstanceEnum : public XamlElementInstance {

 public:
	XamlElementInstanceEnum (XamlElementInfoEnum *element_info, const char *name, ElementType type);

	virtual bool IsDependencyObject ()
	{
		return false;
	}

	virtual Value *GetAsValue ()
	{
		return value;
	}

	bool CreateEnumFromString (const char* str);

	// An enum type doesn't really support anything. It's just here so people can do <Visibility>Visible</Visibility>
	virtual DependencyObject *CreateItem () { return NULL; }
	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value) { return false; }
	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, const char *value) { return false; }
	virtual void AddChild (XamlParserInfo *p, XamlElementInstance *child) { }
	virtual void SetAttributes (XamlParserInfo *p, const char **attr);

	virtual bool TrySetContentProperty (XamlParserInfo *p, XamlElementInstance *value) { return false; }
	virtual bool TrySetContentProperty (XamlParserInfo *p, const char *value) { return CreateEnumFromString (value); }
};

class XamlElementInstanceTemplate : public XamlElementInstanceNative {
public:
	XamlElementInstanceTemplate (XamlElementInfoNative *element_info, XamlParserInfo *parser_info, const char *name, ElementType type, bool create_item = true)
		: XamlElementInstanceNative (element_info, parser_info, name, type, create_item)
	{
	}

	virtual bool IsTemplate ()
	{
		return true;
	}
};


class DefaultNamespace : public XamlNamespace {
 public:
	DefaultNamespace ()
	{
		AddPrefix ("");
	}

	virtual ~DefaultNamespace () { }

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el, const char **attr, bool create)
	{
		Type* t = Type::Find (p->deployment, el, false);
		if (t && !kind_requires_managed_load (t->GetKind ()))
			return new XamlElementInfoNative (t);

		if (enums_is_enum_name (el))
			return new XamlElementInfoEnum (g_strdup (el));

		XamlElementInfo* managed_element = create_element_info_from_imported_managed_type (p, el, attr, create);
		if (managed_element)
			return managed_element;

		return NULL;
		
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		return false;
	}

	virtual const char* GetUri () { return "http://schemas.microsoft.com/winfx/2006/xaml/presentation"; }
};

class XmlNamespace : public XamlNamespace {
 public:
	XmlNamespace ()
	{
		AddPrefix ("xml");
	}

	virtual ~XmlNamespace () { }

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el, const char **attr, bool create)
	{
		return NULL;
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		if (!strcmp ("lang", attr)) {
			if (item->IsDependencyObject ()) {
				DependencyObject *dob = item->GetAsDependencyObject ();
				if (dob->Is(Type::FRAMEWORKELEMENT)) {
					((FrameworkElement*)dob)->SetLanguage (value);
					return true;
				}
			}
		}

		return false;
	}

	virtual const char* GetUri () { return "http://www.w3.org/XML/1998/namespace"; }
};

class XNamespace : public XamlNamespace {
 public:
	XNamespace ()
	{
		AddPrefix ("x");
	}

	virtual ~XNamespace () { }

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el, const char **attr, bool create)
	{
		return NULL;
	}

	virtual char *FindTypeName (const char **attr, char **xmlns)
	{
		char *res = NULL;

		if (!attr)
			return NULL;

		for (int i = 0; attr [i]; i += 2) {
			const char *ns = strchr (attr [i], '|');
			if (!ns)
				continue;
					
			if (strncmp (GetUri (), attr [i], ns - attr [i]) || strcmp ("Class", ns + 1))
				continue;

			ns = strchr (attr [i + 1], ';');
			if (!ns) {
				*xmlns = g_strdup ("");
				res = g_strdup (attr [i + 1]);
			} else {
				*xmlns = g_strdup (ns + 1);
				res = g_strndup (attr [i + 1], attr [i + 1] - ns);
			}
			return res;
		}
		return NULL;
	}

	bool IsParentResourceDictionary (XamlElementInstance *parent)
	{
		if (parent == NULL)
			return false;

		return Type::IsSubclassOf (Deployment::GetCurrent (), parent->info->GetKind (), Type::RESOURCE_DICTIONARY);
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		if (!strcmp ("Name", attr)) {
			//
			// Causes breakage in airlines
			// Maybe x:Name overwrites but Name does not?
			//
			// if (p->namescope->FindName (value)) {
			//	parser_error (p, p->current_element->element_name, "x:Name", 2028, "The name already exists in the tree: %s.", value);
			//	return false;
			// }
			//

			if (IsParentResourceDictionary (p->current_element)) {
				if (item->GetKey ()) {
					// XXX don't know the proper values here...
					parser_error (p, item->element_name, NULL, 2028,
						      "The name already exists in the tree: %s.", value);
					return false;
				}
			}

			if (item->GetName ()) {
				parser_error (p, item->element_name, NULL, 2016, "Cannot specify both Name and x:Name attributes.");
				return false;
			}

			item->SetName (p, value);

			if (item->IsDependencyObject ()) {
				NameScope *scope = p->namescope;
				if (!item->GetAsDependencyObject ()->SetName (value, scope)) {
					if (IsParentResourceDictionary (p->current_element)) {
						// FIXME: inside of a resource dictionary this has an extremly
						// strange behavior.  this isn't exactly right, since not only
						// does the exception get swallowed, but the name seems to also
						// be unregistered.
					}
					else {
						parser_error (p, item->element_name, NULL, 2028,
							      "The name already exists in the tree: %s.", value);
						return false;
					}
				}
				return true;
			}

			return false;
		}

		if (!strcmp ("Key", attr)) {
			if (item->GetKey () && IsParentResourceDictionary (p->current_element) && !Type::IsSubclassOf (p->deployment, item->info->GetKind (), Type::STORYBOARD)) {
				// XXX don't know the proper values here...
				parser_error (p, item->element_name, NULL, 2028,
					      "The name already exists in the tree: %s.", value);
				return false;
			}
			item->SetKey (p, value);
			return true;
		}

		if (!strcmp ("Class", attr)) {
			if (!is_legal_top_level_kind (item->info->GetKind ())) {
				// XXX don't know the proper values here...
				parser_error (p, item->element_name, attr, -1,
					      "Cannot specify x:Class type '%s' on value type element (%s).", value, item->element_name);
				return false;
			}

			if (p->top_element != item) {
				// HAH: what a useless error message
				parser_error (p, item->element_name, attr, 2012,
					      "Unknown attribute %s on element %s.", attr, item->element_name);
				return false;
			}

			// While hydrating, we do not need to create the toplevel class, its created already
			if (p->hydrating)
				return true;
			else {
				parser_error (p, item->element_name, attr, 4005,
					      "Cannot specify x:Class in xaml files outside of a xap.");
				return false;
			}
		}

		return false;
	}

	virtual const char* GetUri () { return X_NAMESPACE_URI; }
};


class PrimitiveNamespace : public XamlNamespace {

 public:
	PrimitiveNamespace (char *prefix)
	{
		AddPrefix (prefix);
	}

	virtual ~PrimitiveNamespace ()
	{
	}

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el, const char **attr, bool create)
	{
		if (!strcmp ("String", el)) {
			Type* t = Type::Find (p->deployment, Type::STRING);
			// it's not as easy in this case, because primitive clr strings require that the
			// character data be read in verbatim, including all whitespace.
			XamlElementInfo *info = new XamlElementInfoNative (t);
			info->SetIsCDataVerbatim (true);
			return info;
		} else if (!strcmp ("Int32", el)) {
			Type* t = Type::Find (p->deployment, Type::INT32);
			return new XamlElementInfoNative (t);
		} else if (!strcmp ("Double", el)) {
			Type* t = Type::Find (p->deployment, Type::DOUBLE);
			return new XamlElementInfoNative (t);
		} else if (!strcmp ("Boolean", el)) {
			Type* t = Type::Find (p->deployment, Type::BOOL);
			return new XamlElementInfoNative (t);
		} else if (!strcmp ("TimeSpan", el)) {
			Type* t = Type::Find (p->deployment, Type::TIMESPAN);
			return new XamlElementInfoNative (t);
		}

		return NULL;
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		return false;
	}

	virtual const char* GetUri () { return PRIMITIVE_NAMESPACE_URI; }
};


class MCIgnorableNamespace : public XamlNamespace {

 public:
	MCIgnorableNamespace (char *prefix)
	{
		AddPrefix (prefix);
	}

	virtual ~MCIgnorableNamespace ()
	{
	}

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el, const char **attr, bool create)
	{
		return NULL;
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		if (!strcmp ("Ignorable", attr)) {
			const char *start = value;
			do {
				const char *space = strchr (start, ' ');
				char *prefix;
				if (space) {
					prefix = g_strndup (start, space - start);
					start = space + 1;
				} else {
					prefix = g_strdup (start);
					start = NULL;
				}

				XamlNamespace *ns = (XamlNamespace *) g_hash_table_find (p->namespace_map, namespace_for_prefix, prefix);
				if (ns)
					ns->is_ignored = true;
				
			} while (start);

			return true;
		}

		return false;
	}

	virtual const char* GetUri () { return MC_IGNORABLE_NAMESPACE_URI; }
};


static void
destroy_created_namespace (gpointer data, gpointer user_data)
{
	XamlNamespace* ns = (XamlNamespace *) data;
	delete ns;
}


class XamlElementInfoManaged : public XamlElementInfo {
 public:
	XamlElementInfoManaged (const char *xmlns, const char *name, XamlElementInfo *parent, Type::Kind dependency_type, Value *obj) : XamlElementInfo (xmlns, name, dependency_type)
	{
		this->obj = obj;
	}

	Value *obj;

	const char* GetName () { return name; }

	const char* GetContentProperty (XamlParserInfo *p);

	virtual bool RequiresManagedSet () { return true; }
	
	XamlElementInstance* CreateElementInstance (XamlParserInfo *p);
	XamlElementInstance* CreateWrappedElementInstance (XamlParserInfo *p, Value *o);
	XamlElementInstance* CreatePropertyElementInstance (XamlParserInfo *p, const char *name);
};


class XamlElementInstanceManaged : public XamlElementInstance {
 public:
	XamlElementInstanceManaged (XamlElementInfo *info, const char *name, ElementType type, Value *obj);

	virtual bool IsDependencyObject ()
	{
		return is_dependency_object;
	}

	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value);
	virtual bool SetProperty (XamlParserInfo *p, XamlElementInstance *property, const char* value);
	virtual void AddChild (XamlParserInfo *p, XamlElementInstance *child);
	virtual void SetAttributes (XamlParserInfo *p, const char **attr);

	virtual bool TrySetContentProperty (XamlParserInfo *p, XamlElementInstance *value);
	virtual bool TrySetContentProperty (XamlParserInfo *p, const char *value);

	virtual void* GetManagedPointer ();
	virtual Value* GetParentPointer ();
 private:
	bool is_dependency_object;
};


class XamlElementInfoImportedManaged : public XamlElementInfoManaged {
 public:
	XamlElementInfoImportedManaged (const char *name, XamlElementInfo *parent, Value *obj) : XamlElementInfoManaged (NULL, name, parent, obj->GetKind (), obj)
	{
	}

	const char* GetContentProperty (XamlParserInfo *p);

	XamlElementInstance* CreateElementInstance (XamlParserInfo *p);
	XamlElementInstance* CreateWrappedElementInstance (XamlParserInfo *p, Value *o);
	XamlElementInstance* CreatePropertyElementInstance (XamlParserInfo *p, const char *name);
};



class ManagedNamespace : public XamlNamespace {
 public:
	char *xmlns;

	ManagedNamespace (char *xmlns, char *prefix)
	{
		this->xmlns = xmlns;
		AddPrefix (prefix);
	}

	virtual ~ManagedNamespace ()
	{
		g_free (xmlns);
	}

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el, const char **attr, bool create)
	{
		char* type_name = NULL;
		char* type_xmlns = NULL;
		const char* use_xmlns = xmlns;

		if (!p->loader)
			return NULL;

		if (x_namespace) {
			// We might have an x:Class attribute specified, so we need to use that for the
			// type_name that we pass to LookupObject
			if (strcmp ("Application", el)) {
				type_name = x_namespace->FindTypeName (attr, &type_xmlns);
				if (type_name) {
					el = type_name;
					use_xmlns = type_xmlns;

					if (!p->hydrating) {
						parser_error (p, el, "x:Class", 4005, "Cannot specify x:Class in xaml files outside of a xap.");
						return NULL;
					}
				}
			}
		}

		Value *value = new Value ();
		if (!p->loader->LookupObject (p, p->GetTopElementPtr (), p->current_element ? p->current_element->GetAsValue () : NULL, use_xmlns, el, create, false, value)) {
			parser_error (p, el, NULL, 2007, "Unable to resolve managed type %s.", el);
			delete value;
			if (type_name)
				g_free (type_name);
			if (type_xmlns)
				g_free (type_xmlns);
			return  NULL;
		}

		if (p->hydrate_expecting) {
			//
			// If we are hydrating a top level managed object, use the Value* passed
			// to Hydrate as our value
			//
			Value *v = value;
			value = p->hydrate_expecting;
			delete v;
		}

		XamlElementInfoManaged *info = new XamlElementInfoManaged (xmlns, g_strdup (el), NULL, value->GetKind (), value);
		if (type_name)
			g_free (type_name);
		if (type_xmlns)
			g_free (type_xmlns);
		return info;
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value)
	{
		if (is_ignored)
			return true;

		if (p->loader) {
			Value v = Value (value);
			return p->loader->SetProperty (p, p->GetTopElementPtr (), item->info->xmlns, item->GetAsValue (), item, item->GetParentPointer (), xmlns, attr, &v, NULL);
		}
		return false;
	}

	
	virtual const char* GetUri () { return xmlns; }
};

bool
XamlLoader::LookupObject (void *p, Value *top_level, Value *parent, const char* xmlns, const char* type_name, bool create, bool is_property, Value *value)
{
	if (callbacks.lookup_object) {
		if (!vm_loaded && !LoadVM ())
			return false;
		MoonError error;
		XamlCallbackData data = XamlCallbackData (this, p, top_level);
		bool res = callbacks.lookup_object (&data, parent, xmlns, type_name, create, is_property, value, &error);
		return res;
	}
		
	return false;
}

const char *
XamlLoader::GetContentPropertyName (void *p, Value *top_level, Value *object)
{
	if (callbacks.get_content_property_name) {
		MoonError error;
		XamlCallbackData data = XamlCallbackData (this, p, top_level);
		return callbacks.get_content_property_name (&data, object, &error);
	}
	return NULL;
}

bool
XamlLoader::SetProperty (void *p, Value *top_level, const char* xmlns, Value *target, void *target_data, Value *target_parent, const char* prop_xmlns, const char *name, Value *value, void* value_data, int flags)
{
	if (callbacks.set_property) {
		MoonError error;
		XamlCallbackData data = XamlCallbackData (this, p, top_level, flags);
		bool res = callbacks.set_property (&data, xmlns, target, target_data, target_parent, prop_xmlns, name, value, value_data, &error);

		if (error.number != MoonError::NO_ERROR) {
			parser_error ((XamlParserInfo *) p, ((XamlElementInstance *) target_data)->element_name, NULL, error.code, error.message);
			return false;
		}

		return res;
	}

	return false;
}

bool
XamlLoader::AddChild (void *p, Value *top_level, Value *parent_parent, bool parent_is_property, const char* parent_xmlns, Value *parent, void *parent_data, Value *child, void *child_data)
{
	if (callbacks.add_child) {
		MoonError error;
		XamlCallbackData data = XamlCallbackData (this, p, top_level);
		bool res = callbacks.add_child (&data, parent_parent, parent_is_property, parent_xmlns, parent, parent_data, child, child_data, &error);

		if (error.number != MoonError::NO_ERROR) {
			parser_error ((XamlParserInfo *) p, ((XamlElementInstance *) child_data)->element_name, NULL, error.code, error.message);
			return false;
		}

		return res;
	}
	return false;
}

XamlLoader::XamlLoader (const char *resourceBase, const char* filename, const char* str, Surface* surface, XamlContext *context)
{
	Initialize (resourceBase, filename, str, surface, context);
}

XamlLoader::XamlLoader (const char* filename, const char* str, Surface* surface, XamlContext *context)
{
	Initialize (NULL, filename, str, surface, context);
}

void
XamlLoader::Initialize (const char *resourceBase, const char* filename, const char* str, Surface* surface, XamlContext *context)
{
	this->filename = g_strdup (filename);
	this->resource_base = g_strdup (resourceBase);
	this->str = g_strdup (str);
	this->surface = surface;
	if (surface)
		surface->ref ();
	this->context = context;
	this->vm_loaded = false;
	this->error_args = NULL;
	this->expanding_template = false;
	this->template_owner = NULL;
	this->import_default_xmlns = false;

	if (context) {
		callbacks = context->internal->callbacks;		
		this->vm_loaded = true;

		if (!surface && context->internal->surface) {
			this->surface = context->internal->surface;
			this->surface->ref ();
		}
	}
#if DEBUG
	if (!surface && debug_flags & RUNTIME_DEBUG_XAML) {
		printf ("XamlLoader::XamlLoader ('%s', '%s', %p): Initializing XamlLoader without a surface.\n",
			filename, str, surface);
	}
#endif
}

XamlLoader::~XamlLoader ()
{
	g_free (filename);
	g_free (resource_base);
	g_free (str);
	if (surface)
		surface->unref ();
	surface = NULL;
	filename = NULL;
	str = NULL;
	if (error_args)
		error_args->unref();
}

bool
XamlLoader::LoadVM ()
{
	return false;
}

XamlLoader* 
xaml_loader_new (const char *resourceBase, const char* filename, const char* str, Surface* surface)
{
	return new XamlLoader (resourceBase, filename, str, surface);
}

void
xaml_loader_free (XamlLoader* loader)
{
	delete loader;
}

void 
xaml_loader_set_callbacks (XamlLoader* loader, XamlLoaderCallbacks callbacks)
{
	if (!loader) {
		LOG_XAML ("Trying to set callbacks for a null object\n");
		return;
	}

	loader->callbacks = callbacks;
	loader->vm_loaded = true;
}

static gboolean
namespace_for_prefix (gpointer key, gpointer value, gpointer user_data)
{
	XamlNamespace *ns = (XamlNamespace *) value;
	const char *prefix = (const char *) user_data;

	return ns->HasPrefix (prefix);
}

char*
xaml_uri_for_prefix (void *parser, char* prefix)
{
	XamlParserInfo *p = (XamlParserInfo *) parser;

	XamlNamespace *ns = (XamlNamespace *) g_hash_table_find (p->namespace_map, namespace_for_prefix, prefix);
	if (!ns)
		return NULL;

	return g_strdup (ns->GetUri ());
}

//
// Called when we encounter an error.  Note that memory ownership is taken for everything
// except the message, this allows you to use g_strdup_printf when creating the error message
//
static void
parser_error (XamlParserInfo *p, const char *el, const char *attr, int error_code, const char *format, ...)
{
	char *message;
	va_list args;
	
	// Already failed
	if (p->error_args)
		return;
	
	// if parsing fails too early it's not safe (i.e. sigsegv) to call some functions, e.g. XML_GetCurrentLineNumber
	bool report_line_col = (error_code != XML_ERROR_XML_DECL);
	int line_number = report_line_col ? XML_GetCurrentLineNumber (p->parser) : 0;
	int char_position = report_line_col ? XML_GetCurrentColumnNumber (p->parser) : 0;
	
	va_start (args, format);
	message = g_strdup_vprintf (format, args);
	va_end (args);

	p->error_args = new ParserErrorEventArgs (message, p->file_name, line_number, char_position, error_code, el, attr);
	
	g_free (message);
	
	LOG_XAML ("PARSER ERROR, STOPPING PARSING:  (%d) %s  line: %d   char: %d\n", error_code, message,
		  line_number, char_position);
	
	XML_StopParser (p->parser, FALSE);
}

void
expat_parser_error (XamlParserInfo *p, XML_Error expat_error)
{
	// Already had an error
	if (p->error_args)
		return;
	
	LOG_XAML ("expat error is:  %d\n", expat_error);
	
	switch (expat_error) {
	case XML_ERROR_DUPLICATE_ATTRIBUTE:
		parser_error (p, NULL, NULL, 7031, "wfc: unique attribute spec");
		break;
	case XML_ERROR_UNBOUND_PREFIX:
		parser_error (p, NULL, NULL, 7055, "undeclared prefix");
		break;
	case XML_ERROR_NO_ELEMENTS:
		parser_error (p, NULL, NULL, 7000, "unexpected end of input");
		break;
	case XML_ERROR_SYNTAX:
		parser_error (p, NULL, NULL, 2103, "syntax error");
		break;
	default:
		parser_error (p, NULL, NULL, expat_error, "Unhandled XML error %s", XML_ErrorString (expat_error));
		break;
	}
}

static void
start_element (void *data, const char *el, const char **attr)
{
	XamlParserInfo *p = (XamlParserInfo *) data;
	XamlElementInfo *elem = NULL;
	XamlElementInstance *inst;
	Types *types = Deployment::GetCurrent ()->GetTypes ();

	if (!strcmp (el, INTERNAL_IGNORABLE_ELEMENT))
		return;

	if (p->ShouldBeginBuffering ()) {
		p->BeginBuffering ();
		return;
	}

	if (p->InBufferingMode ()) {
		if (!strcmp (p->buffer_until_element, el))
			p->buffer_depth++;
		return;
	}	

	const char *dot = strchr (el, '.');
	if (!dot)
		elem = p->current_namespace->FindElement (p, el, attr, p->hydrate_expecting == NULL);

	if (p->error_args)
		return;

	
	if (elem) {
		if (p->hydrate_expecting){
			/*
			Type::Kind expecting_type =  p->hydrate_expecting->GetObjectType ();

			if (!types->IsSubclassOf (expecting_type, elem->GetKind ())) {
				parser_error (p, el, NULL, -1, "Invalid top-level element found %s, expecting %s", el,
					      types->Find (expecting_type)->GetName ());
				return;
			}
			*/

			inst = elem->CreateWrappedElementInstance (p, p->hydrate_expecting);
			p->hydrate_expecting = NULL;
		} else
			inst = elem->CreateElementInstance (p);

		if (!inst)
			return;

		inst->parent = p->current_element;

		if (!p->top_element) {
			p->top_element = inst;
			if (inst->GetAsDependencyObject ())
				NameScope::SetNameScope (inst->GetAsDependencyObject (), p->namescope);
		}

		inst->SetAttributes (p, attr);
		if (p->error_args)
			return;

		if (inst->IsDependencyObject ()) {
			if (p->current_element){
				if (p->current_element->info) {
					p->current_element->AddChild (p, inst);
					if (p->error_args)
						return;
				}
			}
		}
	} else {
		// it's actually valid (from SL point of view) to have <Ellipse.Triggers> inside a <Rectangle>
		// however we can't add properties to something bad, like a <Recta.gle> element
		XamlElementInfo *prop_info = NULL;
		if (dot) {
			gchar *prop_elem = g_strndup (el, dot - el);
			prop_info = p->current_element->FindPropertyElement (p, el, dot);
			g_free (prop_elem);
		}

		if (prop_info != NULL) {
			inst = prop_info->CreatePropertyElementInstance (p, g_strdup (el));
			inst->parent = p->current_element;

			if (attr [0] != NULL) {
				// It appears there is a bug in the error string but it matches the MS runtime
				parser_error (p, el, NULL, 2018, "The element %s does not support attributes.", attr[0]);
				return;
			}
			
			if (prop_info && !strcmp (el, "TextBox.Text"))
				prop_info->SetIsCDataVerbatim (true);
			
			if (!p->top_element) {
				if (types->IsSubclassOf (prop_info->GetKind (), Type::COLLECTION)) {
					XamlElementInstance *wrap = prop_info->CreateElementInstance (p);
					NameScope::SetNameScope (wrap->GetAsDependencyObject (), p->namescope);
					p->top_element = wrap;
					p->current_element = wrap;
					return;
				}
			}
		} else {
			g_warning ("Unknown element: %s.", el);
			parser_error (p, el, NULL, 2007, "Unknown element: %s.", el);
			return;
		}
	}

	if (p->current_element) {
		p->current_element->children->Append (inst);
	}
	p->current_element = inst;

	if (elem && element_begins_buffering (elem->GetKind ())) {
		p->QueueBeginBuffering (g_strdup (el), BUFFER_MODE_TEMPLATE);
	}
}

static bool
allow_value_from_str_in_flush (XamlParserInfo *p, XamlElementInstance *parent)
{
	if (parent == NULL || parent->element_type != XamlElementInstance::PROPERTY || parent->parent == NULL || !parent->parent->IsDependencyObject ())
		return false;

	if (parent->info->GetKind () == Type::OBJECT)
		return true;

	return false;
}

static void
flush_char_data (XamlParserInfo *p)
{
	if (p->InBufferingMode ())
		return;

	if (!p->cdata || !p->current_element)
		return;

	if (p->current_element->info->IsCDataVerbatim()) {
		p->cdata->str = g_strstrip (p->cdata->str);
	}

	if (p->current_element->element_type == XamlElementInstance::ELEMENT) {
		if (!p->current_element->TrySetContentProperty (p, p->cdata->str) && p->cdata_content) {
			if (allow_value_from_str_in_flush (p, p->current_element->parent)) {
				Value *v;
				if (value_from_str (p->current_element->info->GetKind (), NULL, p->cdata->str, &v)) {
					p->current_element->SetValue (v);
					goto cleanup;
				}
			}
			parser_error (p, p->current_element->element_name, NULL, 2011,
				      "%s does not support text content.", p->current_element->element_name);
		}
	} else if (p->current_element->element_type == XamlElementInstance::PROPERTY) {
		if (p->cdata_content && p->current_element->parent && !p->current_element->parent->SetProperty (p, p->current_element, p->cdata->str)) {
			parser_error (p, p->current_element->element_name, NULL, 2011,
				      "%s does not support text content.", p->current_element->element_name);
		}
	}

cleanup:
	if (p->cdata) {
		g_string_free (p->cdata, TRUE);
		p->cdata_content = false;
		p->cdata = NULL;
	}
}

static bool
element_begins_buffering (Type::Kind kind)
{
	return Type::IsSubclassOf (Deployment::GetCurrent (), kind, Type::FRAMEWORKTEMPLATE);
}

static gboolean
is_default_namespace (gpointer key, gpointer value, gpointer user_data)
{
	return value == default_namespace;
}

static void
start_element_handler (void *data, const char *el, const char **attr)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (p->error_args)
		return;

	char **name = g_strsplit (el, "|",  -1);
	XamlNamespace *next_namespace = NULL;
	char *element = NULL;

	if (g_strv_length (name) == 2) {
		// Find the proper namespace for our next element
		next_namespace = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, name [0]);
		element = name [1];
	}
	
	if (!next_namespace && p->implicit_default_namespace) {
		// Use the default namespace for the next element
		next_namespace = default_namespace;
		element = name [0];
	} else if (!next_namespace) {
		if (!g_hash_table_find (p->namespace_map, is_default_namespace, NULL))
			return parser_error (p, el, NULL, 2263, "AG_E_PARSER_MISSING_DEFAULT_NAMESPACE");
	}

	if (next_namespace && next_namespace->is_ignored) {
		p->current_namespace = next_namespace;
		if (!p->InBufferingMode ())
			p->QueueBeginBuffering (g_strdup (element), BUFFER_MODE_IGNORE);
		
		start_element (data, element, attr); // This will force the buffering to start/build depth if needed
		return;
	}

	p->next_element = element;

	flush_char_data (p);
	
	// Now update our namespace
	p->current_namespace = next_namespace;
	
	if (!p->current_namespace && !p->InBufferingMode ()) {
		if (name[1])
			parser_error (p, name[1], NULL, -1, "No handlers available for namespace: '%s' (%s)\n", name[0], el);
		else
			parser_error (p, name[1], NULL, -1, "No namespace mapping available for element: '%s'\n", el);
		
		g_strfreev (name);
		return;
	}

	p->next_element = NULL;
	start_element (data, element, attr);

	g_strfreev (name);
}

static char*
get_element_name (XamlParserInfo* p, const char *el)
{
	char **names = g_strsplit (el, "|",  -1);
	char *name = g_strdup (names [g_strv_length (names) - 1]);

	g_strfreev (names);

	return name;
}

static GSList *
create_resource_list (XamlParserInfo *p)
{
	GSList *list = NULL;
	XamlElementInstance *walk = p->current_element;

	Types * types = Deployment::GetCurrent ()->GetTypes ();
	while (walk) {
		if (walk->element_type == XamlElementInstance::ELEMENT && types->IsSubclassOf (walk->info->GetKind (), Type::FRAMEWORKELEMENT)) {
			FrameworkElement *fwe = (FrameworkElement *)walk->GetAsDependencyObject ();
			if (g_slist_index (list, fwe) == -1)
				list = g_slist_prepend (list, fwe);
		}
		if (walk->element_type == XamlElementInstance::ELEMENT && types->IsSubclassOf (walk->info->GetKind (), Type::RESOURCE_DICTIONARY)) {
			ResourceDictionary *rd = (ResourceDictionary *) walk->GetAsDependencyObject ();
			if (g_slist_index (list, rd) == -1)
				list = g_slist_prepend (list, walk->GetAsDependencyObject ());
		}
		walk = walk->parent;
	}

	list = g_slist_reverse (list);
	return list;
}

static XamlContext *
create_xaml_context (XamlParserInfo *p, FrameworkTemplate *template_, XamlContext *parent_context)
{
	GSList *resources = create_resource_list (p);
	XamlContextInternal *ic =  new XamlContextInternal (p->loader->callbacks, p->GetTopElementPtr (), template_, p->namespace_map, resources, parent_context ? parent_context->internal : NULL);
	return new XamlContext (ic);
}

static void
end_element_handler (void *data, const char *el)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (!strcmp (el, INTERNAL_IGNORABLE_ELEMENT))
		return;

	if (p->error_args)
		return;

	if (!p->current_element) {
		g_warning ("p->current_element == NULL, current_element = %p (%s)\n",
			   p->current_element, p->current_element ? p->current_element->element_name : "<NULL>");
		return;
	}

	if (p->InBufferingMode ()) {
		char* name = get_element_name (p, el);
		if (!strcmp (p->buffer_until_element, name)) {
			p->buffer_depth--;

			if (p->buffer_depth == 0) {
				if (p->buffer_mode == BUFFER_MODE_TEMPLATE) {
					// OK we are done buffering, the element we are buffering for
					FrameworkTemplate* template_ = (FrameworkTemplate *) p->current_element->GetAsDependencyObject ();

					char* buffer = p->ClearBuffer ();

					XamlContext *context = create_xaml_context (p, template_, p->loader->GetContext());

					if (p->validate_templates) {
						p->ValidateTemplate (buffer, context, template_);

						if (p->error_args)
							return;
					}

					template_->SetXamlBuffer (context, buffer);
					p->current_element = p->current_element->parent;
				} else if (p->buffer_mode == BUFFER_MODE_IGNORE) {
					// For now we'll actually keep/clear this buffer because it makes testing easier
					char *buffer = p->ClearBuffer ();
					g_free (buffer);
				}
			}
		}

		g_free (name);
		return;
	}

	switch (p->current_element->element_type) {
	case XamlElementInstance::ELEMENT:

		p->current_element->SetDelayedProperties (p);
		flush_char_data (p);

		// according to http://blogs.msdn.com/devdave/archive/2008/10/11/control-lifecycle.aspx
		// default styles are apply when the end tag is read.
		//
		if (p->current_element->IsDependencyObject () &&
		    p->current_element->GetAsDependencyObject() &&
		    p->current_element->GetAsDependencyObject()->Is(Type::CONTROL)) {

			Control *control = (Control*)p->current_element->GetAsDependencyObject();
			ManagedTypeInfo *key = control->GetDefaultStyleKey ();

			if (key) {
				if (Application::GetCurrent () == NULL)
					g_warning ("attempting to use a null application applying default style while parsing.");
				else
					Application::GetCurrent()->ApplyDefaultStyle (control, key);
			}
		}
		else if (!p->current_element->IsDependencyObject ()) {

			if (p->current_element->parent)
				p->current_element->parent->AddChild (p, p->current_element);
		}
		break;
	case XamlElementInstance::PROPERTY: {
		List::Node *walk = p->current_element->children->First ();
		while (walk) {
			XamlElementInstance *child = (XamlElementInstance *) walk;
			if (p->current_element->parent) {
				p->current_element->parent->SetProperty (p, p->current_element, child);
			}
			walk = walk->next;
		}
		flush_char_data (p);
		break;
	}
	}

	p->current_element = p->current_element->parent;
}

static void
char_data_handler (void *data, const char *in, int inlen)
{
	XamlParserInfo *p = (XamlParserInfo *) data;
	register const char *inptr = in;
	const char *inend = in + inlen;
	const char *start;

	if (p->InBufferingMode ())
		return;

	if (p->error_args)
		return;
	
	if (p->current_element && p->current_element->info->IsCDataVerbatim()) {
		if (!p->cdata)
			p->cdata = g_string_new ("");
			
		g_string_append_len (p->cdata, inptr, inlen);
		p->cdata_content = true;
		return;
	}

	if (!p->cdata) {
		p->cdata = g_string_new ("");
		
		if (g_ascii_isspace (*inptr)) {
			g_string_append_c (p->cdata, ' ');
			inptr++;
			
			while (inptr < inend && g_ascii_isspace (*inptr))
				inptr++;
		}
		
		if (inptr == inend)
			return;
	} else if (g_ascii_isspace (p->cdata->str[p->cdata->len - 1])) {
		// previous cdata chunk ended with lwsp, skip over leading lwsp for this chunk
		while (inptr < inend && g_ascii_isspace (*inptr))
			inptr++;
	}
	
	while (inptr < inend) {
		start = inptr;
		while (inptr < inend && !g_ascii_isspace (*inptr))
			inptr++;
		
		if (inptr > start) {
			g_string_append_len (p->cdata, start, inptr - start);
			p->cdata_content = true;
		}
		
		if (inptr < inend) {
			g_string_append_c (p->cdata, ' ');
			inptr++;
			
			while (inptr < inend && g_ascii_isspace (*inptr))
				inptr++;
		}
	}
}

static void
start_namespace_handler (void *data, const char *prefix, const char *uri)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (p->InBufferingMode ())
		return;

	if (p->error_args)
		return;

	if (p->loader != NULL && p->loader->callbacks.import_xaml_xmlns != NULL) {
		MoonError error;
		XamlCallbackData data = XamlCallbackData (p->loader, p, p->GetTopElementPtr ());
		if (!p->loader->callbacks.import_xaml_xmlns (&data, uri, &error))
			return parser_error (p, p->current_element ? p->current_element->element_name : NULL, prefix, 2005, "Unknown namespace %s", uri);
	}

	for (int i = 0; default_namespace_names [i]; i++) {
		if (!strcmp (default_namespace_names [i], uri)) {
			g_hash_table_insert (p->namespace_map, g_strdup (uri), default_namespace);
			return;
		}
	}
		
	if (!strcmp (X_NAMESPACE_URI, uri)){
		g_hash_table_insert (p->namespace_map, g_strdup (uri), x_namespace);
	} else if (!strcmp (PRIMITIVE_NAMESPACE_URI, uri)) {
		PrimitiveNamespace *pn = new PrimitiveNamespace (g_strdup (prefix));
		g_hash_table_insert (p->namespace_map, g_strdup (uri), pn);
		p->AddCreatedNamespace (pn);
	} else if (!strcmp (MC_IGNORABLE_NAMESPACE_URI, uri)) {
		MCIgnorableNamespace *mc = new MCIgnorableNamespace (g_strdup (prefix));
		g_hash_table_insert (p->namespace_map, g_strdup (uri), mc);
		p->AddCreatedNamespace (mc);
	} else {
		if (!p->loader) {
			return parser_error (p, (p->current_element ? p->current_element->element_name : NULL), prefix, -1,
					     "No managed element callback installed to handle %s", uri);
		}

		if (!prefix) {
			parser_error (p, (p->current_element ? p->current_element->element_name : NULL), NULL, 2262,
				      "AG_E_PARSER_NAMESPACE_NOT_SUPPORTED");
			return;
		}

		XamlNamespace *ns = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, uri);

		if (ns) {
			ns->AddPrefix (prefix);
			return;
		}

		ManagedNamespace *c = new ManagedNamespace (g_strdup (uri), g_strdup (prefix));
		g_hash_table_insert (p->namespace_map, g_strdup (c->xmlns), c);
		p->AddCreatedNamespace (c);
	}
}

static void
start_doctype_handler (void *data,
		const XML_Char *doctype_name,
		const XML_Char *sysid,
		const XML_Char *pubid,
		int has_internal_subset)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (p->InBufferingMode ())
		return;

	if (sysid)
		return parser_error (p, NULL, NULL, 7050, "DTD was found but is prohibited");
	
	if (doctype_name)
		return parser_error (p, NULL, NULL, 7016, "incorrect document syntax.");
}

static void
add_default_namespaces (XamlParserInfo *p, bool sl_xmlns)
{
	if (sl_xmlns) {
		p->implicit_default_namespace = true;
		g_hash_table_insert (p->namespace_map, g_strdup ("http://schemas.microsoft.com/winfx/2006/xaml/presentation"), default_namespace);
		g_hash_table_insert (p->namespace_map, g_strdup (X_NAMESPACE_URI), x_namespace);
	}
	g_hash_table_insert (p->namespace_map, g_strdup (XML_NAMESPACE_URI), xml_namespace);
}

static void
print_tree (XamlElementInstance *el, int depth)
{
#if DEBUG
	if (debug_flags & RUNTIME_DEBUG_XAML) {
		for (int i = 0; i < depth; i++)
			printf ("\t");
	
		const char *name = NULL;

		if (!el) {
			printf (" -null- \n");
			return;
		}

		if (el->element_type == XamlElementInstance::ELEMENT && el->IsDependencyObject ())
			name = el->GetAsDependencyObject ()->GetName ();
		printf ("%s  (%s)  (%p) (%s)\n", el->element_name, name ? name : "-no name-", el->parent, el->element_type == XamlElementInstance::PROPERTY ? "PROPERTY" : "ELEMENT");

		for (List::Node *walk = el->children->First (); walk != NULL; walk = walk->next) {
			XamlElementInstance *el = (XamlElementInstance *) walk;
			print_tree (el, depth + 1);
		}
	}
#endif
}

void		
xaml_parse_xmlns (const char* xmlns, char** type_name, char** ns, char** assembly)
{
	const char delimiters[]  = ";";
	char* decl;
	char* buffer = g_strdup (xmlns);
	
	*type_name = NULL;
	*ns = NULL;
	*assembly = NULL;
	
	decl = strtok (buffer, delimiters);
	while (decl != NULL) {
		if (strstr (decl, "clr-namespace:") == decl) {
			if (*ns)
				g_free (*ns);
			*ns = g_strdup (decl + 14);
		} else if (strstr (decl, "assembly=") == decl) {
			if (*assembly)
				g_free (*assembly);
			*assembly = g_strdup (decl + 9);
		} else {
			if (*type_name)
				g_free (*type_name);
			*type_name = g_strdup (decl);			
		}
		
		decl = strtok (NULL, delimiters);
	}
	g_free (buffer);
}

Value *
XamlLoader::CreateFromFile (const char *xaml_file, bool create_namescope,
			    Type::Kind *element_type)
{
	Value *res = NULL;
	XamlParserInfo *parser_info = NULL;
	XML_Parser p = NULL;
	bool first_read = true;
	const char *inptr, *inend;
	TextStream *stream;
	char buffer[4096];
	ssize_t nread, n;

	LOG_XAML ("attemtping to load xaml file: %s\n", xaml_file);
	
	stream = new TextStream ();
	if (!stream->OpenFile (xaml_file, false)) {
		LOG_XAML ("can not open file\n");
		goto cleanup_and_return;
	}
	
	if (!(p = XML_ParserCreateNS ("UTF-8", '|'))) {
		LOG_XAML ("can not create parser\n");
		goto cleanup_and_return;
	}

	parser_info = new XamlParserInfo (p, xaml_file);
	
	parser_info->namescope->SetTemporary (!create_namescope);

	parser_info->loader = this;

	// TODO: This is just in here temporarily, to make life less difficult for everyone
	// while we are developing.  
	add_default_namespaces (parser_info, false);

	XML_SetUserData (p, parser_info);

	XML_SetElementHandler (p, start_element_handler, end_element_handler);
	XML_SetCharacterDataHandler (p, char_data_handler);
	XML_SetNamespaceDeclHandler(p, start_namespace_handler, NULL);
	XML_SetDoctypeDeclHandler(p, start_doctype_handler, NULL);

	/*
	XML_SetProcessingInstructionHandler (p, proc_handler);
	*/
	
	while ((nread = stream->Read (buffer, sizeof (buffer))) >= 0) {
		inptr = buffer;
		n = nread;
		
		if (first_read && nread > 0) {
			// Remove preceding white space
			inend = buffer + nread;
			
			while (inptr < inend && g_ascii_isspace (*inptr))
				inptr++;
			
			if (inptr == inend)
				continue;
			
			n = (inend - inptr);
			first_read = false;
		}

		parser_info->SetXmlBuffer (inptr);
		if (!XML_Parse (p, inptr, n, nread == 0)) {
			expat_parser_error (parser_info, XML_GetErrorCode (p));
			goto cleanup_and_return;
		}
		
		if (nread == 0)
			break;
	}
	
	print_tree (parser_info->top_element, 0);
	
	if (parser_info->top_element) {
		res = parser_info->top_element->GetAsValue ();
		// We want a copy because the old one is going to be deleted
		res = new Value (*res);

		if (element_type)
			*element_type = parser_info->top_element->info->GetKind ();

		if (parser_info->error_args) {
			*element_type = Type::INVALID;
			goto cleanup_and_return;
		}	
	}
	
 cleanup_and_return:
	
	if (!parser_info)
		error_args = new ParserErrorEventArgs ("Error opening xaml file", xaml_file, 0, 0, 1, "", "");
	else if (parser_info->error_args) {
		error_args = parser_info->error_args;
		error_args->ref ();
	}
	
	delete stream;
	
	if (p)
		XML_ParserFree (p);
	
	if (parser_info)
		delete parser_info;
	
	return res;
}

Value *
XamlLoader::CreateFromString (const char *xaml, bool create_namescope, Type::Kind *element_type, int flags)
{
	return HydrateFromString (xaml, NULL, create_namescope, element_type, flags);
}

DependencyObject *
value_to_dependency_object (Value *value)
{
	if (!value || !value->Is (Deployment::GetCurrent (), Type::DEPENDENCY_OBJECT))
		return NULL;
	return value->AsDependencyObject ();
}

DependencyObject *
XamlLoader::CreateDependencyObjectFromFile (const char *xaml, bool create_namescope, Type::Kind *element_type)
{
	return value_to_dependency_object (CreateFromFile (xaml, create_namescope, element_type));
}

DependencyObject *
XamlLoader::CreateDependencyObjectFromString (const char *xaml, bool create_namescope, Type::Kind *element_type)
{
	return value_to_dependency_object (CreateFromString (xaml, create_namescope, element_type, IMPORT_DEFAULT_XMLNS));
}

/**
 * Hydrates an existing DependencyObject (@object) with the contents from the @xaml
 * data
 */
Value *
XamlLoader::HydrateFromString (const char *xaml, Value *object, bool create_namescope, Type::Kind *element_type, int flags)
{
	XML_Parser p = XML_ParserCreateNS ("utf-8", '|');
	XamlParserInfo *parser_info = NULL;
	Value *res = NULL;
	char *start = (char*)xaml;
	char *prepend = NULL;
	char *append = NULL;
	char * inputs [4] = {NULL, NULL, NULL, NULL};

	inputs [0] = start;

	if (!p) {
		LOG_XAML ("can not create parser\n");
		goto cleanup_and_return;
	}

#if 0
	if (true) {
		static int id = 0;
		char filename[64];
		FILE *fp;
		
		sprintf (filename, "createFromXaml.%d.xaml", id++);
		if ((fp = fopen (filename, "wt")) != NULL) {
			fwrite (xaml, strlen (xaml), 1, fp);
			fclose (fp);
		}
	}
#endif
	
	parser_info = new XamlParserInfo (p, NULL);

	parser_info->namescope->SetTemporary (!create_namescope);

	parser_info->loader = this;
	parser_info->validate_templates = (flags & VALIDATE_TEMPLATES) == VALIDATE_TEMPLATES;

	//
	// If we are hydrating, we are not null
	//
	if (object != NULL) {
		parser_info->hydrate_expecting = object;
		parser_info->hydrating = true;
		if (Type::IsSubclassOf (parser_info->deployment, object->GetKind (), Type::DEPENDENCY_OBJECT)) {
			DependencyObject *dob = object->AsDependencyObject ();
			dob->SetSurface (GetSurface());
			dob->SetResourceBase (GetResourceBase());
		}
	} else {
		parser_info->hydrate_expecting = NULL;
		parser_info->hydrating = false;
	}
	
	// from_str gets the default namespaces implictly added
	add_default_namespaces (parser_info, (flags & IMPORT_DEFAULT_XMLNS) == IMPORT_DEFAULT_XMLNS);

	XML_SetUserData (p, parser_info);

	XML_SetElementHandler (p, start_element_handler, end_element_handler);
	XML_SetCharacterDataHandler (p, char_data_handler);
	XML_SetNamespaceDeclHandler(p, start_namespace_handler, NULL);
	XML_SetDoctypeDeclHandler(p, start_doctype_handler, NULL);

	/*
	XML_SetProcessingInstructionHandler (p, proc_handler);
	*/

	if (context) {
		prepend = context->internal->CreateIgnorableTagOpen ();
		append = context->internal->CreateIgnorableTagClose ();

		inputs [0] = prepend;
		inputs [1] = start;
		inputs [2] = append;
	}

	for (int i = 0; inputs [i]; i++) {
		char *start = inputs [i];

		// don't freak out if the <?xml ... ?> isn't on the first line (see #328907)
		while (g_ascii_isspace (*start))
			start++;

		parser_info->SetXmlBuffer (start);
		if (!XML_Parse (p, start, strlen (start), inputs [i + 1] == NULL)) {
			expat_parser_error (parser_info, XML_GetErrorCode (p));
			LOG_XAML ("error parsing:  %s\n\n", xaml);
			goto cleanup_and_return;
		}
	}
	
	print_tree (parser_info->top_element, 0);
	
	if (parser_info->top_element) {
		if (is_legal_top_level_kind (parser_info->top_element->info->GetKind ())) {
			res = parser_info->top_element->GetAsValue ();
			res = new Value (*res);
			if (res->Is (parser_info->deployment, Type::DEPENDENCY_OBJECT) && object) {
				DependencyObject *dob = res->AsDependencyObject ();
				dob->unref ();
				dob->SetIsHydratedFromXaml (parser_info->hydrating);
			}
		}

		if (element_type)
			*element_type = parser_info->top_element->info->GetKind ();

		if (!res && !parser_info->error_args)
			parser_info->error_args = new ParserErrorEventArgs ("No DependencyObject found", "", 0, 0, 1, "", "");

		if (parser_info->error_args) {
			delete res;
			res = NULL;
			if (element_type)
				*element_type = Type::INVALID;
			goto cleanup_and_return;
		}
	}

 cleanup_and_return:
	
	if (parser_info->error_args) {
		error_args = parser_info->error_args;
		printf ("Could not parse element %s, attribute %s, error: %s\n",
			error_args->xml_element,
			error_args->xml_attribute,
			error_args->GetErrorMessage());
	}
	
	if (p)
		XML_ParserFree (p);
	if (parser_info)
		delete parser_info;
	if (prepend)
		g_free (prepend);
	if (append)
		g_free (append);

	return res;
}

Value *
XamlLoader::CreateFromFileWithError (const char *xaml_file, bool create_namescope, Type::Kind *element_type, MoonError *error)
{
	Value *res = CreateFromFile (xaml_file, create_namescope, element_type);
	if (error_args && error_args->GetErrorCode () != -1)
		MoonError::FillIn (error, error_args);
	return res;
}

Value *
XamlLoader::CreateFromStringWithError  (const char *xaml, bool create_namescope, Type::Kind *element_type, int flags, MoonError *error)
{
	Value *res = CreateFromString (xaml, create_namescope, element_type, flags);
	if (error_args && error_args->GetErrorCode () != -1)
		MoonError::FillIn (error, error_args);
	return res;
}

Value *
XamlLoader::HydrateFromStringWithError (const char *xaml, Value *object, bool create_namescope, Type::Kind *element_type, int flags, MoonError *error)
{
	Value *res = HydrateFromString (xaml, object, create_namescope, element_type, flags);
	if (error_args && error_args->GetErrorCode () != -1)
		MoonError::FillIn (error, error_args);
	return res;
}

static int
parse_int (const char **pp, const char *end)
{
	const char *p = *pp;
#if 0
	if (optional && AtEnd)
		return 0;
#endif

	int res = 0;
	int count = 0;

	while (p <= end && g_ascii_isdigit (*p)) {
		res = res * 10 + *p - '0';
		p++;
		count++;
	}

#if 0
	if (count == 0)
		formatError = true;
#endif

	*pp = p;
	return res;
}

static gint64
parse_ticks (const char **pp, const char *end)
{
	gint64 mag = 1000000;
	gint64 res = 0;
	bool digitseen = false;

	const char *p = *pp;

	while (mag > 0 && p <= end && g_ascii_isdigit (*p)) {
		res = res + (*p - '0') * mag;
		p++;
		mag = mag / 10;
		digitseen = true;
	}

#if 0
	if (!digitseen)
		formatError = true;
#endif

	*pp = p;
	return res;
}

bool
time_span_from_str (const char *str, TimeSpan *res)    
{
	const char *end = str + strlen (str);
	const char *p;

	bool negative = false;
	int days;
	int hours;
	int minutes;
	int seconds;
	gint64 ticks;

	p = str;

	if (*p == '-') {
		p++;
		negative = true;
	}

	days = parse_int (&p, end);
	
	if (*p == '.') {
		p++;
		hours = parse_int (&p, end);
	}
	else {
		hours = days;
		days = 0;
	}
	
	if (*p == ':') p++;
	minutes = parse_int (&p, end);
	if (*p == ':') p++;
	seconds = parse_int (&p, end);
	if (*p == '.') {
		p++;
		ticks = parse_ticks (&p, end);
	}
	else {
		ticks = 0;
	}

	gint64 t = (days * 86400) + (hours * 3600) + (minutes * 60) + seconds;
	t *= 10000000L;

	*res = negative ? (-t - ticks) : (t + ticks);

	return true;
}

bool
repeat_behavior_from_str (const char *str, RepeatBehavior *res)
{
	if (!g_ascii_strcasecmp ("Forever", str)) {
		*res = RepeatBehavior::Forever;
		return true;
	}

	// check for "<float>x".

	// XXX more validation work is needed here.. but how do we
	// report an error?
	const char *x = strchr (str, 'x');
	if (x) {
		if (*(x + 1) != '\0') {
			return false;
		}
		else {
			char *endptr;
			errno = 0;
			double d = g_ascii_strtod (str, &endptr);

			if (errno || endptr == str)
				return false;

			*res = RepeatBehavior (d);
			return true;
		}
	}

	/* XXX RepeatBehavior='XX:XX:XX' syntax is NOT correctly supported by
	   Silverlight 1.0 (most likely a bug). It works fine in Silverlight 2.0 
	   though. We currently stick to the 2.0 behavior, not replicating the bug
	   from 1.0. 
	*/
	TimeSpan t;
	if (!time_span_from_str (str, &t))
		return false;

	*res = RepeatBehavior (t);

	return true;
}

bool
duration_from_str (const char *str, Duration *res)
{
	if (!g_ascii_strcasecmp ("Automatic", str)) {
		*res = Duration::Automatic;
		return true;
	}

	if (!g_ascii_strcasecmp ("Forever", str)) {
		*res = Duration::Forever;
		return true;
	}

	TimeSpan ts;
	if (!time_span_from_str (str, &ts))
		return false;

	*res = Duration (ts);
	return true;
}

bool
keytime_from_str (const char *str, KeyTime *res)
{
	if (!g_ascii_strcasecmp ("Uniform", str)) {
		*res = KeyTime::Uniform;
		return true;
	}

	if (!g_ascii_strcasecmp ("Paced", str)) {
		*res = KeyTime::Paced;
		return true;
	}

	/* check for a percentage first */
	const char *last = str + strlen(str) - 1;
	if (*last == '%') {
		char *ep;
		double pct = g_ascii_strtod (str, &ep);
		if (ep == last) {
			*res = KeyTime (pct);
			return true;
		}
	}

	TimeSpan ts;
	if (!time_span_from_str (str, &ts))
		return false;

	*res = KeyTime (ts);
	return true;
}

bool
key_spline_from_str (const char *str, KeySpline **res)
{	
	PointCollection *pts = PointCollection::FromStr (str);

	if (!pts)
		return false;

	if (pts->GetCount () != 2) { 
		pts->unref ();
		return false;
	}

	*res = new KeySpline (*pts->GetValueAt (0)->AsPoint (), *pts->GetValueAt (1)->AsPoint ());
	
	pts->unref ();
	
	return true;
}

Matrix *
matrix_from_str (const char *str)
{
	if (!g_ascii_strcasecmp ("Identity", str)) {
		return new Matrix ();
	}

	DoubleCollection *values = DoubleCollection::FromStr (str);
	Matrix *matrix;
	
	if (!values)
		return new Matrix ();

	if (values->GetCount () < 6) {
		values->unref ();
		return NULL;
	}

	matrix = new Matrix ();
	matrix->SetM11 (values->GetValueAt (0)->AsDouble ());
	matrix->SetM12 (values->GetValueAt (1)->AsDouble ());
	matrix->SetM21 (values->GetValueAt (2)->AsDouble ());
	matrix->SetM22 (values->GetValueAt (3)->AsDouble ());
	matrix->SetOffsetX (values->GetValueAt (4)->AsDouble ());
	matrix->SetOffsetY (values->GetValueAt (5)->AsDouble ());
	
	values->unref ();

	return matrix;
}

Matrix3D *
matrix3d_from_str (const char *str)
{
	if (!g_ascii_strcasecmp ("Identity", str)) {
		return new Matrix3D ();
	}

	DoubleCollection *values = DoubleCollection::FromStr (str);
	Matrix3D *matrix;
	
	if (!values)
		return new Matrix3D ();

	if (values->GetCount () < 12) {
		values->unref ();
		return NULL;
	}

	matrix = new Matrix3D ();
	matrix->SetM11 (values->GetValueAt (0)->AsDouble ());
	matrix->SetM12 (values->GetValueAt (1)->AsDouble ());
	matrix->SetM13 (values->GetValueAt (2)->AsDouble ());
	matrix->SetM13 (values->GetValueAt (3)->AsDouble ());
	matrix->SetM21 (values->GetValueAt (4)->AsDouble ());
	matrix->SetM22 (values->GetValueAt (5)->AsDouble ());
	matrix->SetM23 (values->GetValueAt (6)->AsDouble ());
	matrix->SetM24 (values->GetValueAt (7)->AsDouble ());
	matrix->SetM31 (values->GetValueAt (8)->AsDouble ());
	matrix->SetM32 (values->GetValueAt (9)->AsDouble ());
	matrix->SetM33 (values->GetValueAt (10)->AsDouble ());
	matrix->SetM34 (values->GetValueAt (11)->AsDouble ());
	matrix->SetOffsetX (values->GetValueAt (12)->AsDouble ());
	matrix->SetOffsetY (values->GetValueAt (13)->AsDouble ());
	matrix->SetOffsetZ (values->GetValueAt (14)->AsDouble ());
	matrix->SetM44 (values->GetValueAt (15)->AsDouble ());

	values->unref ();

	return matrix;
}

bool
grid_length_from_str (const char *str, GridLength *grid_length)
{
	if (IS_NULL_OR_EMPTY (str)) {
		*grid_length = GridLength (0.0, GridUnitTypePixel);
		return true;
	}

	if (str [0] == '*') {
		*grid_length = GridLength (1.0, GridUnitTypeStar);
		return true;
	}

	// unit tests shows that "Auto", "auto", "aUtO"... all works
	if (!g_ascii_strcasecmp (str, "Auto")) {
		*grid_length = GridLength ();
		return true;
	}

	char *endptr;
	errno = 0;
	double d = g_ascii_strtod (str, &endptr);

	if (errno || endptr == str)
		return false;

	*grid_length = GridLength (d, *endptr == '*' ? GridUnitTypeStar : GridUnitTypePixel);
	return true;
}

static void
advance (char **in)
{
	char *inptr = *in;
	
	while (*inptr && !g_ascii_isalnum (*inptr) && *inptr != '.' && *inptr != '-' && *inptr != '+')
		inptr = g_utf8_next_char (inptr);
	
	*in = inptr;
}

static bool
get_point (Point *p, char **in)
{
	char *end, *inptr = *in;
	double x, y;
	
	x = g_ascii_strtod (inptr, &end);
	if (end == inptr)
		return false;
	
	inptr = end;
	while (g_ascii_isspace (*inptr))
		inptr++;
	
	if (*inptr == ',')
		inptr++;
	
	while (g_ascii_isspace (*inptr))
		inptr++;
	
	y = g_ascii_strtod (inptr, &end);
	if (end == inptr)
		return false;
	
	p->x = x;
	p->y = y;
	
	*in = end;
	
	return true;
}

static void
make_relative (const Point *cp, Point *mv)
{
	mv->x += cp->x;
	mv->y += cp->y;
}

static bool
more_points_available (char **in)
{
	char *inptr = *in;
	
	while (g_ascii_isspace (*inptr) || *inptr == ',')
		inptr++;
	
	*in = inptr;
	
	return (g_ascii_isdigit (*inptr) || *inptr == '.' || *inptr == '-' || *inptr == '+');
}

Geometry *
geometry_from_str (const char *str)
{
	char *inptr = (char *) str;
	Point cp = Point (0, 0);
	Point cp1, cp2, cp3;
	Point start;
	char *end;
	PathGeometry *pg = NULL;
	FillRule fill_rule = FillRuleEvenOdd;
	bool cbz = false; // last figure is a cubic bezier curve
	bool qbz = false; // last figure is a quadratic bezier curve
	Point cbzp, qbzp; // points needed to create "smooth" beziers

	moon_path *path = moon_path_new (10);

	while (*inptr) {
		if (g_ascii_isspace (*inptr))
			inptr++;
		
		if (!inptr[0])
			break;
		
		bool relative = false;
		
		char c = *inptr;
		inptr = g_utf8_next_char (inptr);
		
		switch (c) {
		case 'f':
		case 'F':
			advance (&inptr);

			if (*inptr == '0')
				fill_rule = FillRuleEvenOdd;
			else if (*inptr == '1')
				fill_rule = FillRuleNonzero;
			else
				// FIXME: else it's a bad value and nothing should be rendered
				goto bad_pml; // partial: only this Path won't be rendered

			inptr = g_utf8_next_char (inptr);
			break;
		case 'm':
			relative = true;
		case 'M':
			if (!get_point (&cp1, &inptr))
				break;
			
			if (relative)
				make_relative (&cp, &cp1);

			// start point
			moon_move_to (path, cp1.x, cp1.y);

			start.x = cp.x = cp1.x;
			start.y = cp.y = cp1.y;

			advance (&inptr);
			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp1);
				
				moon_line_to (path, cp1.x, cp1.y);
			}

			cp.x = cp1.x;
			cp.y = cp1.y;
			cbz = qbz = false;
			break;

		case 'l':
			relative = true;
		case 'L':
		{
			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp1);

				moon_line_to (path, cp1.x, cp1.y);

				cp.x = cp1.x;
				cp.y = cp1.y;

				advance (&inptr);
			}
			cbz = qbz = false;
			break;
		}
		
		case 'h':
			relative = true;
		case 'H':
		{
			double x = g_ascii_strtod (inptr, &end);
			if (end == inptr)
				break;
			
			inptr = end;
			
			if (relative)
				x += cp.x;
			cp = Point (x, cp.y);

			moon_line_to (path, cp.x, cp.y);
			cbz = qbz = false;
			break;
		}
		
		case 'v':
			relative = true;
		case 'V':
		{
			double y = g_ascii_strtod (inptr, &end);
			if (end == inptr)
				break;
			
			inptr = end;
			
			if (relative)
				y += cp.y;
			cp = Point (cp.x, y);

			moon_line_to (path, cp.x, cp.y);
			cbz = qbz = false;
			break;
		}
		
		case 'c':
			relative = true;
		case 'C':
		{
			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;

				if (relative)
					make_relative (&cp, &cp1);
			
				advance (&inptr);

				if (!get_point (&cp2, &inptr))
					break;
			
				if (relative)
					make_relative (&cp, &cp2);
			
				advance (&inptr);
			
				if (!get_point (&cp3, &inptr))
					break;
			
				if (relative)
					make_relative (&cp, &cp3);
			
				advance (&inptr);
			
				moon_curve_to (path, cp1.x, cp1.y, cp2.x, cp2.y, cp3.x, cp3.y);

				cp1.x = cp3.x;
				cp1.y = cp3.y;
			}
			cp.x = cp3.x;
			cp.y = cp3.y;
			cbz = true;
			cbzp.x = cp2.x;
			cbzp.y = cp2.y;
			qbz = false;
			break;
		}
		case 's':
			relative = true;
		case 'S':
		{
			while (more_points_available (&inptr)) {
				if (!get_point (&cp2, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp2);

				advance (&inptr);
				
				if (!get_point (&cp3, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp3);

				if (cbz) {
					cp1.x = 2 * cp.x - cbzp.x;
					cp1.y = 2 * cp.y - cbzp.y;
				} else
					cp1 = cp;

				moon_curve_to (path, cp1.x, cp1.y, cp2.x, cp2.y, cp3.x, cp3.y);
				cbz = true;
				cbzp.x = cp2.x;
				cbzp.y = cp2.y;

				cp.x = cp3.x;
				cp.y = cp3.y;

				advance (&inptr);
			}
			qbz = false;
			break;
		}
		case 'q':
			relative = true;
		case 'Q':
		{
			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;
			
				if (relative)
					make_relative (&cp, &cp1);

				advance (&inptr);
			
				if (!get_point (&cp2, &inptr))
					break;
			
				if (relative)
					make_relative (&cp, &cp2);
			
				advance (&inptr);

				moon_quad_curve_to (path, cp1.x, cp1.y, cp2.x, cp2.y);

				cp.x = cp2.x;
				cp.y = cp2.y;
			}
			qbz = true;
			qbzp.x = cp1.x;
			qbzp.y = cp1.y;
			cbz = false;
			break;
		}
		case 't':
			relative = true;
		case 'T':
		{
			while (more_points_available (&inptr)) {
				if (!get_point (&cp2, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp2);

				if (qbz) {
					cp1.x = 2 * cp.x - qbzp.x;
					cp1.y = 2 * cp.y - qbzp.y;
				} else
					cp1 = cp;

				moon_quad_curve_to (path, cp1.x, cp1.y, cp2.x, cp2.y);
				qbz = true;
				qbzp.x = cp1.x;
				qbzp.y = cp1.y;
				
				cp.x = cp2.x;
				cp.y = cp2.y;

				advance (&inptr);
			}
			cbz = false;
			break;
		}
		case 'a':
			relative = true;
		case 'A':
		{
			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;
				
				advance (&inptr);
				
				double angle = g_ascii_strtod (inptr, &end);
				if (end == inptr)
					break;
				
				inptr = end;
				advance (&inptr);
				
				int is_large = strtol (inptr, &end, 10);
				if (end == inptr)
					break;
				
				inptr = end;
				advance (&inptr);
				
				int sweep = strtol (inptr, &end, 10);
				if (end == inptr)
					break;
				
				inptr = end;
				advance (&inptr);
				
				if (!get_point (&cp2, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp2);

				moon_arc_to (path, cp1.x, cp1.y, angle, is_large, sweep, cp2.x, cp2.y);
					
				cp.x = cp2.x;
				cp.y = cp2.y;

				advance (&inptr);
			}
			cbz = qbz = false;
			break;
		}
		case 'z':
		case 'Z':
			moon_line_to (path, start.x, start.y);
			moon_close_path (path);
			moon_move_to (path, start.x, start.y);

			cp.x = start.x;
			cp.y = start.y;
			cbz = qbz = false;
			break;
		default:
			break;
		}
	}

	pg = new PathGeometry (path);
	pg->SetFillRule (fill_rule);
	return pg;

bad_pml:
	moon_path_destroy (path);
	return NULL;
}

static bool
value_is_explicit_null (const char *str)
{
	return !strcmp ("{x:Null}", str);
}

static bool
is_managed_kind (Type::Kind kind)
{
	
	if (kind == Type::MANAGED ||
	    kind == Type::OBJECT ||
	    kind == Type::URI ||
	    kind == Type::MANAGEDTYPEINFO ||
	    kind == Type::DEPENDENCYPROPERTY)
		return true;

	return false;
}

static bool
kind_requires_managed_load (Type::Kind kind)
{
	if (kind == Type::USERCONTROL) {
		return true;
	}

	return false;
}

static bool
is_legal_top_level_kind (Type::Kind kind)
{
	if (kind == Type::MANAGED || kind == Type::OBJECT || Type::IsSubclassOf (Deployment::GetCurrent (), kind, Type::DEPENDENCY_OBJECT))
		return true;
	return false;
}

// NOTE: Keep definition in sync with class/System.Windows/Mono/NativeMethods.cs
bool
value_from_str_with_typename (const char *type_name, const char *prop_name, const char *str, Value **v)
{
	Type *t = Type::Find (Deployment::GetCurrent (), type_name);
	if (!t)
		return false;

	return value_from_str (t->GetKind (), prop_name, str, v);
}

char *
expand_property_path (XamlParserInfo *p, PropertyPath *path)
{
	if (!path->path)
		return NULL;

	bool expanded = false;
	GString *res = g_string_new (path->path);

	int len = strlen (res->str);
	for (int i = 0; i < len; i++) {
		if (res->str [i] == ':') {
			int e = i;
			int s = i - 1;
			int te = i + 1;
			for ( ; s > 0; s--) {
				if (!g_ascii_isalnum (res->str [s]))
					break;
			}

			for ( ; te < len; te++) {
				if (!g_ascii_isalpha (res->str [te]) || res->str [te] == '_')
					break;
			}

			char *prefix = g_strndup (res->str + s + 1, e - s - 1);
			char *type = g_strndup (res->str + e + 1, te - e - 1);

			res = g_string_erase (res, s + 1, te - s - 1);

			XamlNamespace *ns = (XamlNamespace *) g_hash_table_find (p->namespace_map, namespace_for_prefix, prefix);
			if (!ns) {
				g_free (prefix);
				g_free (type);
				g_string_free (res, true);
				return NULL;
			}

			XamlElementInfo *info = ns->FindElement (p, type, NULL, false);

			if (!info) {
				g_free (prefix);
				g_free (type);
				g_string_free (res, true);
				return NULL;
			}
			
			char *uri = g_strdup_printf ("'%s'", Type::Find (p->deployment, info->GetKind ())->GetName ());

			res = g_string_insert (res, s + 1, uri);
			i = s + 1 + strlen (uri);
			len = strlen (res->str);

			delete info;
			g_free (uri);
			g_free (prefix);
			g_free (type);

			expanded = true;
		}
	}

	if (!expanded) {
		g_string_free (res, true);
		return NULL;
	}

	char *expanded_str = res->str;
	g_string_free (res, false);

	return expanded_str;
}

bool
value_from_str (Type::Kind type, const char *prop_name, const char *str, Value **v)
{
	bool v_set = false;

	value_from_str_with_parser (NULL, type, prop_name, str, v, &v_set);

	return v_set;
}

bool
xaml_bool_from_str (const char *s, bool *res)
{
	bool b;
	char *endptr;

	if (!g_ascii_strcasecmp ("true", s))
		b = true;
	else if (!g_ascii_strcasecmp ("false", s))
		b = false;
	else {
		// Check if it's a string representing a decimal value
		gint64 l;

		errno = 0;
		l = strtol (s, &endptr, 10);

		if (errno || endptr == s || *endptr || l > G_MAXINT32 || l < G_MININT32)
			return false;;

		if (l == 0)
			b = false;
		else
			b = true;
	}

	*res = b;
	return true;
}

static bool
value_from_str_with_parser (XamlParserInfo *p, Type::Kind type, const char *prop_name, const char *str, Value **v, bool *v_set)
{
	char *endptr;
	*v = NULL;
	
	if (value_is_explicit_null (str)) {
		*v = NULL;
		*v_set = true;
		return true;
	}

	char *s = g_strdup (str);

	if (type == Type::OBJECT || type == Type::STRING) {
		// object and string use the literal string
	}
	else {
		// everything else depends on the string being stripped
		s = g_strstrip (s);
	}

	switch (type) {
	case Type::OBJECT: {
		// not much more can do here, unless we want to try to
		// probe str to see if it's actually meant to be a
		// specific type.  just assume it's a string.
		*v = new Value (s);
		*v_set = true;
		break;
	}

	case Type::BOOL: {
		bool b;

		if (!xaml_bool_from_str (s, &b))
			break;

		*v = new Value (b);
		*v_set = true;
		break;
	}
	case Type::DOUBLE: {
		double d;

		// empty string should not reset default values with 0
		//
		// FIXME: this causes a 2.0 unit test to fail (PrimitiveTest.ParseEmptyDouble)
		if (IS_NULL_OR_EMPTY(s)) {
			g_free (s);
			return false;
		}

		bool is_nan = false;
		if (!g_ascii_strcasecmp (s, "NAN"))
			is_nan = true;
		else {
			errno = 0;
			d = g_ascii_strtod (s, &endptr);
		}

		if (is_nan || errno || endptr == s || *endptr) {
			if (prop_name
			    && (!strcmp (prop_name, "Width") || !strcmp (prop_name, "Height"))
			    && (!g_ascii_strcasecmp (s, "Auto") || is_nan))
				d = NAN;
			else
				break;
		}

		*v = new Value (d);
		*v_set = true;
		break;
	}
	case Type::INT64: {
		gint64 l;

		errno = 0;
		l = strtol (s, &endptr, 10);

		if (errno || endptr == s)
			break;

		*v = new Value (l, Type::INT64);
		*v_set = true;
		break;
	}
	case Type::TIMESPAN: {
		TimeSpan ts;

		if (!time_span_from_str (s, &ts))
			break;

		*v = new Value (ts, Type::TIMESPAN);
		*v_set = true;
		break;
	}
	case Type::INT32: {
		int i;

		if (IS_NULL_OR_EMPTY(s))
			i = 0;
		else if (g_ascii_isalpha (s[0]) && prop_name) {
			i = enums_str_to_int (prop_name, s);
			if (i == -1) {
//				g_warning ("'%s' enum is not valid on '%s' property", str, prop_name);
				break;
			}
		} else {
			errno = 0;
			long l = strtol (s, &endptr, 10);

			if (errno || endptr == s)
				break;

			i = (int) l;
		}

		*v = new Value (i);
		*v_set = true;
		break;
	}
	case Type::CHAR: {
		gunichar unichar = g_utf8_get_char_validated (str, -1);
		const char *next;
		
		if ((int) unichar < 0)
			break;
		
		if (!(next = g_utf8_next_char (str)) || *next != '\0')
			break;
		
		*v = new Value (unichar, Type::CHAR);
		*v_set = true;
		break;
	}
	case Type::STRING: {
		*v = new Value (str);
		*v_set = true;
		break;
	}
	case Type::COLOR: {
		Color *c = color_from_str (s);
		if (c == NULL)
			break;
		*v = new Value (*c);
		*v_set = true;
		delete c;
		break;
	}
	case Type::REPEATBEHAVIOR: {
		RepeatBehavior rb = RepeatBehavior::Forever;

		if (!repeat_behavior_from_str (s, &rb))
			break;

		*v = new Value (rb);
		*v_set = true;
		break;
	}
	case Type::DURATION: {
		Duration d = Duration::Forever;

		if (!duration_from_str (s, &d))
			break;

		*v = new Value (d);
		*v_set = true;
		break;
	}
	case Type::KEYTIME: {
		KeyTime kt = KeyTime::Paced;

		if (!keytime_from_str (s, &kt))
			break;

		*v = new Value (kt);
		*v_set = true;
		break;
	}
	case Type::KEYSPLINE: {
		KeySpline *ks;

		if (!key_spline_from_str (s, &ks))
			break;

		*v = Value::CreateUnrefPtr (ks);
		*v_set = true;
		break;
	}
	case Type::BRUSH:
	case Type::SOLIDCOLORBRUSH: {
		// Only solid color brushes can be specified using attribute syntax
		Color *c = color_from_str (s);
		
		if (c == NULL)
			break;

		SolidColorBrush *scb = new SolidColorBrush ();
		
		scb->SetColor (c);
		delete c;
		
		*v = Value::CreateUnrefPtr (scb);
		*v_set = true;
		break;
	}
	case Type::POINT: {
		Point p;

		if (!Point::FromStr (s, &p))
			break;

		*v = new Value (p);
		*v_set = true;
		break;
	}
	case Type::SIZE: {
		Size size;

		if (!Size::FromStr (s, &size))
			break;

		*v = new Value (size);
		*v_set = true;
		break;
	}
	case Type::RECT: {
		Rect rect;

		if (!Rect::FromStr (s, &rect))
			break;

		*v = new Value (rect);
		*v_set = true;
		break;
	}
	case Type::CACHEMODE: {
		if (!strcmp (s, "BitmapCache")) {
			BitmapCache *bc = new BitmapCache ();
			*v = Value::CreateUnrefPtr (bc);
			*v_set = true;
		}
		break;
	}
	case Type::URI: {
		Uri uri;

		if (!uri.Parse (s)) {
			break;
		}

		*v = new Value (uri);
		*v_set = true;
		break;
	}
	case Type::DOUBLE_COLLECTION: {
		DoubleCollection *doubles = DoubleCollection::FromStr (s);
		if (!doubles) {
			*v = Value::CreateUnrefPtr (new DoubleCollection ());
			*v_set = true;
			break;
		}

		*v = Value::CreateUnrefPtr (doubles);
		*v_set = true;
		break;
	}
	case Type::POINT_COLLECTION: {
		PointCollection *points = PointCollection::FromStr (s);
		if (!points) {
			*v = Value::CreateUnrefPtr (new PointCollection ());
			*v_set = true;
			break;
		}

		*v = Value::CreateUnrefPtr (points);
		*v_set = true;
		break;
	}
	case Type::TRANSFORMGROUP: {
		if (IS_NULL_OR_EMPTY(s))
			break;

		Matrix *mv = matrix_from_str (s);
		if (!mv)
			break;

		TransformGroup *tg = new TransformGroup ();
		MatrixTransform *t = new MatrixTransform ();
		t->SetValue (MatrixTransform::MatrixProperty, Value (mv));

		tg->GetChildren()->Add (t);
		t->unref ();

		*v = new Value (tg);
		*v_set = true;
		tg->unref ();
		mv->unref ();
		break;
	}
	case Type::TRANSFORM:

		if (!g_ascii_strcasecmp ("Identity", str)) {
			*v = NULL;
			*v_set = true;
			break;
		}

		// Intentional fall through, you can create a matrix from a TRANSFORM property, but not using Identity
	case Type::MATRIXTRANSFORM:
	{
		if (IS_NULL_OR_EMPTY(s))
			break;

		Matrix *mv = matrix_from_str (s);
		if (!mv)
			break;

		MatrixTransform *t = new MatrixTransform ();
		t->SetValue (MatrixTransform::MatrixProperty, Value (mv));

		*v = new Value (t);
		*v_set = true;
		t->unref ();
		mv->unref ();
		break;
	}
	case Type::UNMANAGEDMATRIX:
	case Type::MATRIX: {
		// note: unlike TRANSFORM this creates an empty, identity, matrix for an empty string
		Matrix *matrix = matrix_from_str (s);
		if (!matrix)
			break;

		*v = new Value (matrix);
		*v_set = true;
		matrix->unref ();
		break;
	}
	case Type::PROJECTION:

		if (!g_ascii_strcasecmp ("Identity", str)) {
			*v = NULL;
			*v_set = true;
			break;
		}

		// Intentional fall through, you can create a matrix from a PROJECTION property, but not using Identity
	case Type::MATRIX3DPROJECTION:
	{
		if (IS_NULL_OR_EMPTY(s))
			break;

		Matrix3D *mv = matrix3d_from_str (s);
		if (!mv)
			break;

		Matrix3DProjection *p = new Matrix3DProjection ();
		p->SetValue (Matrix3DProjection::ProjectionMatrixProperty, Value (mv));

		*v = new Value (p);
		*v_set = true;
		p->unref ();
		mv->unref ();
		break;
	}
	case Type::UNMANAGEDMATRIX3D:
	case Type::MATRIX3D: {
		Matrix3D *matrix = matrix3d_from_str (s);
		if (!matrix)
			break;

		*v = new Value (matrix);
		*v_set = true;
		matrix->unref ();
		break;
	}
	case Type::PATHGEOMETRY:
	case Type::GEOMETRY: {
		Geometry *geometry = geometry_from_str (s);

		if (!geometry)
			break;

		*v = new Value (geometry);
		*v_set = true;
		geometry->unref ();
		break;
	}
	case Type::THICKNESS: {
		Thickness t;

		if (!Thickness::FromStr (s, &t))
			break;

		*v = new Value (t);
		*v_set = true;
		break;
	}
	case Type::CORNERRADIUS: {
		CornerRadius c;

		if (!CornerRadius::FromStr (s, &c))
			break;

		*v = new Value (c);
		*v_set = true;
		break;
	}
	case Type::GRIDLENGTH: {
		GridLength grid_length;

		if (!grid_length_from_str (s, &grid_length))
			break;

		*v = new Value (grid_length);
		*v_set = true;
		break;
	}
	case Type::IMAGESOURCE:
	case Type::BITMAPIMAGE: {
		Uri uri;

		if (!uri.Parse (s))
			break;

		BitmapImage *bi = new BitmapImage ();

		bi->SetUriSource (&uri);

		*v = Value::CreateUnrefPtr (bi); 
		*v_set = true;

		break;
	}
	case Type::MULTISCALETILESOURCE:
	case Type::DEEPZOOMIMAGETILESOURCE: {
		// As far as I know the only thing you can create here is a URI based DeepZoomImageTileSource
		Uri uri;
		if (!uri.Parse (s))
			break;
		*v = Value::CreateUnrefPtr (new DeepZoomImageTileSource (&uri));
		*v_set = true;

		break;
	}
	case Type::FONTFAMILY: {
		*v = new Value (FontFamily (s)); 
		*v_set = true;
		break;
	}
	case Type::FONTWEIGHT: {
		int fw = enums_str_to_int ("FontWeight", s);
		if (fw != -1) {
			*v = new Value (FontWeight ((FontWeights)fw));
			*v_set = true;
		}
		break;
	}
	case Type::FONTSTYLE: {
		int fs = enums_str_to_int ("FontStyle", s);
		if (fs != -1) {
			*v = new Value (FontStyle ((FontStyles)fs));
			*v_set = true;
		}
		break;
	}
	case Type::FONTSTRETCH: {
		int fs = enums_str_to_int ("FontStretch", s);
		if (fs != -1) {
			*v = new Value (FontStretch ((FontStretches)fs));
			*v_set = true;
		}
		break;
	}
	case Type::PROPERTYPATH: {
		PropertyPath path (s);
		path.expanded_path = expand_property_path (p, &path);
		*v = new Value (path);
		*v_set = true;
		break;
	}
	default:
		// we don't care about NULL or empty values
		if (!IS_NULL_OR_EMPTY (s)) {
			g_free (s);
			return true;
		}
	}

	g_free (s);
	return true;
}

bool
XamlElementInstance::TrySetContentProperty (XamlParserInfo *p, XamlElementInstance *value)
{
	const char* prop_name = info->GetContentProperty (p);

	if (!prop_name)
		return false;

	DependencyProperty *dep = DependencyProperty::GetDependencyProperty (Type::Find (p->deployment, info->GetKind ()), prop_name);
	if (!dep)
		return false;

	bool is_collection = Type::IsSubclassOf (p->deployment, dep->GetPropertyType(), Type::DEPENDENCY_OBJECT_COLLECTION);

	if (!is_collection && Type::IsSubclassOf (p->deployment, value->info->GetKind (), dep->GetPropertyType())) {
		MoonError err;
		if (!item->SetValueWithError (dep, value->GetAsValue (), &err)) {
		    parser_error (p, value->element_name, NULL, err.code, err.message);
		    return false;
		}
		return true;
	}

	// We only want to enter this if statement if we are NOT dealing with the content property element,
	// otherwise, attempting to use explicit property setting, would add the content property element
	// to the content property element collection
	if (is_collection && dep->GetPropertyType() != value->info->GetKind ()) {
		Value *col_v = item->GetValue (dep);
		Collection *col;
			
		if (!col_v) {
			col = collection_new (dep->GetPropertyType ());
			item->SetValue (dep, Value (col));
			col->unref ();
		} else {
			col = (Collection *) col_v->AsCollection ();
		}

		MoonError err;
		if (-1 == col->AddWithError (value->GetAsValue (), &err)) {
			parser_error (p, value->element_name, NULL, err.code, err.message);
			return false;
		}
			
		return true;
	}

	return false;
}

bool
XamlElementInstance::TrySetContentProperty (XamlParserInfo *p, const char *value)
{
	const char* prop_name = info->GetContentProperty (p);

	if (!prop_name) {
		if (info->GetKind () == Type::ICON) {
			prop_name = "Source";
		} else
			return false;
	}

	Type::Kind prop_type = p->current_element->info->GetKind ();
	DependencyProperty *content = DependencyProperty::GetDependencyProperty (Type::Find (p->deployment, prop_type), prop_name);
	
	// TODO: There might be other types that can be specified here,
	// but string is all i have found so far.  If you can specify other
	// types, i should pull the property setting out of set_attributes
	// and use that code

	if (content && (content->GetPropertyType ()) == Type::STRING && value) {
		item->SetValue (content, Value (g_strstrip (p->cdata->str)));
		return true;
	} else if (content && (content->GetPropertyType ()) == Type::URI && value) {
		Uri uri;

		if (!uri.Parse (g_strstrip (p->cdata->str)))
			return false;

		item->SetValue (content, Value (uri));
		return true;
	} else if (Type::IsSubclassOf (p->deployment, info->GetKind (), Type::TEXTBLOCK)) {
		TextBlock *textblock = (TextBlock *) item;
		InlineCollection *inlines = textblock->GetInlines ();
		Inline *last = NULL;
		
		if (inlines && inlines->GetCount () > 0)
			last = inlines->GetValueAt (inlines->GetCount () - 1)->AsInline ();
		
		if (!p->cdata_content) {
			if (p->next_element && !strcmp (p->next_element, "Run") && last && last->GetObjectType () == Type::RUN &&
			    !last->GetAutogenerated ()) {
				// LWSP between <Run> elements is to be treated as a single-SPACE <Run> element
				// Note: p->cdata is already canonicalized
			} else {
				// This is one of the following cases:
				//
				// 1. LWSP before the first <Run> element
				// 2. LWSP after the last <Run> element
				// 3. LWSP between <Run> and <LineBreak> elements
				return true;
			}
		} else {
			if (!p->next_element || !strcmp (p->next_element, "LineBreak"))
				g_strchomp (p->cdata->str);

			if (!last || last->GetObjectType () != Type::RUN || last->GetAutogenerated ())
				g_strchug (p->cdata->str);
		}

		Run *run = new Run ();
		run->SetText (p->cdata->str);
		
		if (!inlines) {
			inlines = new InlineCollection ();
			textblock->SetInlines (inlines);
			inlines->unref ();
		}
		
		inlines->Add (run);
		run->unref ();
		return true;
	}

	return false;
}

bool
XamlElementInstance::SetUnknownAttribute (XamlParserInfo *p, const char *name, const char *value)
{
	if (!p->loader)
		return false;

	Value v = Value (value);
	if (!p->loader->SetProperty (p, p->GetTopElementPtr (), info->xmlns, GetAsValue (), this, GetParentPointer (), NULL, name, &v, NULL)) {
		return false;
	}
	return true;
}

void
XamlElementInstance::SetDelayedProperties (XamlParserInfo *p)
{
	GSList *walk = delayed_properties;

	while (walk) {
		DelayedProperty *prop = (DelayedProperty *) walk->data;

		if (!p->loader->SetProperty (p, p->GetTopElementPtr (), info->xmlns, GetAsValue (), this, GetParentPointer (), prop->xmlns, prop->name, prop->value, NULL, XamlCallbackData::SETTING_DELAYED_PROPERTY)) {
			parser_error (p, element_name, prop->name, 2012,
					"Unknown property %s on element %s.",
					prop->name, element_name);
			return;
		}

		walk = walk->next;
	}
		
}

XamlElementInfo *
XamlElementInstance::FindPropertyElement (XamlParserInfo *p, const char *el, const char *dot)
{
	// We didn't find anything so try looking up in managed
	if (!p->loader)
		return NULL;

	Value *v = new Value ();
	if (p->loader->LookupObject (p, p->GetTopElementPtr (), GetAsValue (), p->current_namespace->GetUri (), el, false, true, v)) {
		char *type_name = g_strndup (el, dot - el);
		
		XamlElementInfoManaged *res = new XamlElementInfoManaged (g_strdup (p->current_namespace->GetUri ()), el, info, v->GetKind (), v);
		XamlElementInfo *container = p->current_namespace->FindElement (p, type_name, NULL, false);
		info->SetPropertyOwnerKind (container->GetKind ());
		g_free (type_name);
		return res;
	}

	delete v;
	return NULL;
}

static XamlElementInfo *
create_element_info_from_imported_managed_type (XamlParserInfo *p, const char *name, const char **attr, bool create)
{
	if (!p->loader)
		return NULL;

	char* type_name = NULL;
	char* type_xmlns = NULL;
	const char* use_xmlns = NULL;

	if (x_namespace) {
		// We might have an x:Class attribute specified, so we need to use that for the
		// type_name that we pass to LookupObject
		if (strcmp ("Application", name)) {
			type_name = x_namespace->FindTypeName (attr, &type_xmlns);
			if (type_name) {
				name = type_name;
				use_xmlns = type_xmlns;

				if (!p->hydrating) {
					parser_error (p, name, "x:Class", 4005, "Cannot specify x:Class in xaml files outside of a xap.");
					return NULL;
				}
			}
		}
	}

	Value *v = new Value ();
	if (!p->loader->LookupObject (p, use_xmlns ? p->GetTopElementPtr () : NULL, NULL, use_xmlns, name, create, false, v)) {
		delete v;
		if (type_name)
			g_free (type_name);
		if (type_xmlns)
			g_free (type_xmlns);
		return NULL;
	}

	XamlElementInfoImportedManaged *info = new  XamlElementInfoImportedManaged (g_strdup (name), NULL, v);

	if (create) {
		if (v->Is (p->deployment, Type::DEPENDENCY_OBJECT))
			p->AddCreatedElement (v->AsDependencyObject());
	}

	return info;
}


const char *
XamlElementInfoNative::GetContentProperty (XamlParserInfo *p)
{
	return type->GetContentPropertyName ();
}

XamlElementInstance *
XamlElementInfoNative::CreateElementInstance (XamlParserInfo *p)
{
	if (type->IsValueType ())
		return new XamlElementInstanceValueType (this, p, GetName (), XamlElementInstance::ELEMENT);
	else if (type->IsSubclassOf (Type::FRAMEWORKTEMPLATE))
		return new XamlElementInstanceTemplate (this, p, GetName (), XamlElementInstance::ELEMENT);
	else
		return new XamlElementInstanceNative (this, p, GetName (), XamlElementInstance::ELEMENT);
}

XamlElementInstance *
XamlElementInfoNative::CreateWrappedElementInstance (XamlParserInfo *p, Value *o)
{
	XamlElementInstance *res = new XamlElementInstanceNative (this, p, GetName (), XamlElementInstance::ELEMENT, false);
	res->SetDependencyObject (o->AsDependencyObject ());

	return res;
}

XamlElementInfo *
XamlElementInstanceNative::FindPropertyElement (XamlParserInfo *p, const char *el, const char *dot)
{
	if (IsDependencyObject ()) {
		const char *prop_name = dot + 1;
		DependencyProperty *prop = DependencyProperty::GetDependencyProperty (Type::Find (p->deployment, info->GetKind ()), prop_name);
		if (prop) {
			XamlElementInfoNative *info = new XamlElementInfoNative (Type::Find (p->deployment, prop->GetPropertyType ()));
			info->SetPropertyOwnerKind (prop->GetOwnerType ());
			return info;
		}
	}

	return XamlElementInstance::FindPropertyElement (p, el, dot);
}

XamlElementInstance *
XamlElementInfoNative::CreatePropertyElementInstance (XamlParserInfo *p, const char *name)
{
	return new XamlElementInstanceNative (this, p, name, XamlElementInstance::PROPERTY, false);
}

XamlElementInstanceNative::XamlElementInstanceNative (XamlElementInfoNative *element_info, XamlParserInfo *parser_info, const char *name, ElementType type, bool create_item) :
	XamlElementInstance (element_info, name, type)
{
	this->element_info = element_info;
	this->parser_info = parser_info;
	if (create_item)
		SetDependencyObject (CreateItem ());
}



DependencyObject *
XamlElementInstanceNative::CreateItem ()
{
	XamlElementInstance *walk = parser_info->current_element;
	Type *type = element_info->GetType ();

	DependencyObject *item = NULL;
	DependencyProperty *dep = NULL;

	if (type->IsSubclassOf (Type::COLLECTION) || type->IsSubclassOf (Type::RESOURCE_DICTIONARY)) {
		// If we are creating a collection, try walking up the element tree,
		// to find the parent that we belong to and using that instance for
		// our collection, instead of creating a new one

		// We attempt to advance past the property setter, because we might be dealing with a
		// content element
		
		if (walk && walk->element_type == XamlElementInstance::PROPERTY) {
			char **prop_name = g_strsplit (walk->element_name, ".", -1);
			
			walk = walk->parent;
			dep = DependencyProperty::GetDependencyProperty (Type::Find (parser_info->deployment, walk->info->GetKind ()), prop_name [1]);

			g_strfreev (prop_name);
		} else if (walk && walk->info->GetContentProperty (parser_info)) {
			dep = DependencyProperty::GetDependencyProperty (Type::Find (parser_info->deployment, walk->info->GetKind ()),
					(char *) walk->info->GetContentProperty (parser_info));			
		}

		if (dep && Type::IsSubclassOf (parser_info->deployment, dep->GetPropertyType(), type->GetKind ())) {
			Value *v = ((DependencyObject * ) walk->GetAsDependencyObject ())->GetValue (dep);
			if (v) {
				item = v->AsDependencyObject ();
				dep = NULL;
			}
			// note: if !v then the default collection is NULL (e.g. PathFigureCollection)
		}
	}

	if (!item) {
		item = element_info->GetType()->IsCtorVisible() ? element_info->GetType ()->CreateInstance () : NULL;

		if (item) {
			parser_info->AddCreatedElement (item);

			// in case we must store the collection into the parent
			if (dep && dep->GetPropertyType() == type->GetKind ()) {
				MoonError err;
				Value item_value (item);
				if (!((DependencyObject * ) walk->GetAsDependencyObject ())->SetValueWithError (dep, &item_value, &err))
					parser_error (parser_info, element_name, NULL, err.code, err.message);
			}
		} else {
			parser_error (parser_info, element_name, NULL, 2007, "Unknown element: %s.", element_name);
		}
	}

	return item;
}

bool
XamlElementInstanceNative::SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value)
{
	if (property->info->RequiresManagedSet () || value->info->RequiresManagedSet ())
		return p->loader->SetProperty (p, p->GetTopElementPtr (), NULL, GetAsValue (), this, GetParentPointer (), property->info->xmlns, property->element_name, value->GetAsValue (), NULL);

	return dependency_object_set_property (p, this, property, value, true);
}

bool
XamlElementInstanceNative::SetProperty (XamlParserInfo *p, XamlElementInstance *property, const char *value)
{
	char **prop_name = g_strsplit (property->element_name, ".", -1);
	Type *owner = Type::Find (p->deployment, prop_name [0]);
	DependencyProperty *dep;

	if (!owner)
		return false;

	dep = DependencyProperty::GetDependencyProperty (Type::Find (p->deployment, owner->GetKind ()), prop_name [1]);
	if (!dep) 
		return false;

	return xaml_set_property_from_str (item, dep, value, NULL/*XXX*/);
}

void
XamlElementInstanceNative::AddChild (XamlParserInfo *p, XamlElementInstance *child)
{
	dependency_object_add_child (p, this, child, true);
}

void
XamlElementInstanceNative::SetAttributes (XamlParserInfo *p, const char **attr)
{
	dependency_object_set_attributes (p, this, attr);
}


XamlElementInstanceValueType::XamlElementInstanceValueType (XamlElementInfoNative *element_info, XamlParserInfo *parser_info, const char *name, ElementType type) :
	XamlElementInstance (element_info, name, type)
{
	this->element_info = element_info;
	this->parser_info = parser_info;
}

bool
XamlElementInstanceValueType::CreateValueItemFromString (const char* str)
{

	bool res = value_from_str (element_info->GetType ()->GetKind (), NULL, str, &value);
	return res;
}

void
XamlElementInstanceValueType::SetAttributes (XamlParserInfo *p, const char **attr)
{
	value_type_set_attributes (p, this, attr);
}

XamlElementInstanceEnum::XamlElementInstanceEnum (XamlElementInfoEnum *element_info, const char *name, ElementType type) :
	XamlElementInstance (element_info, name, type)
{
}

bool
XamlElementInstanceEnum::CreateEnumFromString (const char* str)
{
	int i = enums_str_to_int (element_name, str);
	if (i == -1)
		return false;
		
	value = new Value (i);
	return true;
}

void
XamlElementInstanceEnum::SetAttributes (XamlParserInfo *p, const char **attr)
{
	value_type_set_attributes (p, this, attr);
}

XamlElementInstance *
XamlElementInfoEnum::CreateElementInstance (XamlParserInfo *p)
{
	return new XamlElementInstanceEnum (this, name, XamlElementInstance::ELEMENT);
}

XamlElementInstance *
XamlElementInfoEnum::CreateWrappedElementInstance (XamlParserInfo *p, Value *o)
{
	XamlElementInstance *res = new XamlElementInstanceEnum (this, name, XamlElementInstance::ELEMENT);
	return res;
}

const char *
XamlElementInfoManaged::GetContentProperty (XamlParserInfo *p)
{
	if (!p->loader)
		return NULL;

	// TODO: We could cache this, but for now lets keep things as simple as possible.
	const char *res = p->loader->GetContentPropertyName (p, p->GetTopElementPtr (), obj);
	if (res)
		return res;
	return XamlElementInfo::GetContentProperty (p);
}

XamlElementInstance *
XamlElementInfoManaged::CreateElementInstance (XamlParserInfo *p)
{
	XamlElementInstanceManaged *inst = new XamlElementInstanceManaged (this, GetName (), XamlElementInstance::ELEMENT, obj);

	if (obj->Is (p->deployment, Type::DEPENDENCY_OBJECT))
		p->AddCreatedElement (inst->GetAsDependencyObject ());

	return inst;
}

XamlElementInstance *
XamlElementInfoManaged::CreateWrappedElementInstance (XamlParserInfo *p, Value *o)
{
	XamlElementInstanceManaged *inst = new XamlElementInstanceManaged (this, GetName (), XamlElementInstance::ELEMENT, o);

	return inst;
}

XamlElementInstance *
XamlElementInfoManaged::CreatePropertyElementInstance (XamlParserInfo *p, const char *name)
{
	XamlElementInstanceManaged *inst = new XamlElementInstanceManaged (this, name, XamlElementInstance::PROPERTY, obj);

	return inst;
}

XamlElementInstanceManaged::XamlElementInstanceManaged (XamlElementInfo *info, const char *name, ElementType type, Value *obj) :
	XamlElementInstance (info, name, type)
{
	// The managed code owns our Value objects
	cleanup_value = false;

	this->value = obj;

	if (obj->Is (Deployment::GetCurrent (), Type::DEPENDENCY_OBJECT)) {
		this->is_dependency_object = true;
		this->SetDependencyObject (obj->AsDependencyObject ());
	}
	else
		this->is_dependency_object = false;
}

void *
XamlElementInstanceManaged::GetManagedPointer ()
{
	if (value->Is (Deployment::GetCurrent (), Type::DEPENDENCY_OBJECT))
		return value->AsDependencyObject ();
	return value->AsManagedObject ();
}

Value *
XamlElementInstanceManaged::GetParentPointer ()
{
	XamlElementInstance *walk = parent;
	while (walk && walk->element_type != XamlElementInstance::ELEMENT)
		walk = walk->parent;

	if (!walk) {
		return NULL;
	}

	return walk->GetAsValue ();
}

bool
XamlElementInstanceManaged::SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value)
{
	if (GetAsDependencyObject () != NULL && dependency_object_set_property (p, this, property, value, false))
		return true;
	return p->loader->SetProperty (p, p->GetTopElementPtr (), info->xmlns, GetAsValue (), this, GetParentPointer (), property->info->xmlns, property->element_name, value->GetAsValue (), value);
}

bool
XamlElementInstanceManaged::SetProperty (XamlParserInfo *p, XamlElementInstance *property, const char *value)
{
	Value v = Value (value);
	return p->loader->SetProperty (p, p->GetTopElementPtr (), info->xmlns, GetAsValue (), this, GetParentPointer (), property->info->xmlns, property->element_name, &v, NULL);
}

void
XamlElementInstanceManaged::AddChild (XamlParserInfo *p, XamlElementInstance *child)
{
	if (element_type == XamlElementInstance::PROPERTY) {
		Value *prop = new Value (element_name);
		p->loader->AddChild (p, p->GetTopElementPtr (), GetParentPointer (), true, info->xmlns, prop, this, child->GetAsValue (), child);
		delete prop;
		return;
	}

	p->loader->AddChild (p, p->GetTopElementPtr (), GetParentPointer (), false, info->xmlns, GetAsValue (), this, child->GetAsValue (), child);
}

void
XamlElementInstanceManaged::SetAttributes (XamlParserInfo *p, const char **attr)
{
	dependency_object_set_attributes (p, this, attr);
}

bool
XamlElementInstanceManaged::TrySetContentProperty (XamlParserInfo *p, XamlElementInstance *value)
{
	Value *v = value->GetAsValue ();
	const char* prop_name = info->GetContentProperty (p);

	return p->loader->SetProperty (p, p->GetTopElementPtr (), info->xmlns, GetAsValue (), this, GetParentPointer (), NULL, prop_name, v, value);
}

bool
XamlElementInstanceManaged::TrySetContentProperty (XamlParserInfo *p, const char *value)
{
	if (Type::IsSubclassOf (p->deployment, info->GetKind (), Type::CONTENTCONTROL)) {
		// Content controls are not allowed to have their content set as text, they need to have a child element
		// if you want to set the content of a contentcontrol to text you need to use attribute syntax
		return false;
	}

	if (!XamlElementInstance::TrySetContentProperty (p, value)) {
		const char* prop_name = info->GetContentProperty (p);
		if (!p->cdata_content) 
			return false;
		Value v = Value (value);
		bool res = p->loader->SetProperty (p, p->GetTopElementPtr (), info->xmlns, GetAsValue (), this, GetParentPointer (), NULL, prop_name, &v, NULL);
		return res;
	}

	return false;
}

XamlElementInstance *
XamlElementInfoImportedManaged::CreateElementInstance (XamlParserInfo *p)
{
	XamlElementInstanceManaged *inst = new XamlElementInstanceManaged (this, name, XamlElementInstance::ELEMENT, obj);

	return inst;
}

const char *
XamlElementInfoImportedManaged::GetContentProperty (XamlParserInfo *p)
{
	if (!p->loader)
		return NULL;

	// TODO: Test, it's possible that managed objects that aren't DOs are allowed to have content properties.
	if (!obj->Is (p->deployment, Type::DEPENDENCY_OBJECT))
		return XamlElementInfo::GetContentProperty (p);

	
	// TODO: We could cache this, but for now lets keep things as simple as possible.
	const char *res = p->loader->GetContentPropertyName (p, p->GetTopElementPtr (), obj);
	if (res)
		return res;
	
	return XamlElementInfo::GetContentProperty (p);
}

XamlElementInstance *
XamlElementInfoImportedManaged::CreateWrappedElementInstance (XamlParserInfo *p, Value *o)
{
	XamlElementInstanceManaged *inst = new XamlElementInstanceManaged (this, Type::Find (p->deployment, o->GetKind ())->GetName (), XamlElementInstance::ELEMENT, o);

	return inst;
}

XamlElementInstance *
XamlElementInfoImportedManaged::CreatePropertyElementInstance (XamlParserInfo *p, const char *name)
{
	XamlElementInstanceManaged *inst = new XamlElementInstanceManaged (this, name, XamlElementInstance::PROPERTY, obj);

	return inst;
}


///
/// Add Child funcs
///

static const char*
get_key_from_child (XamlElementInstance *child)
{
	const char *key = child->GetKey ();
	if (key)
		return key;

	key = child->GetName ();
	if (key)
		return key;

	if (child->IsDependencyObject ()) {
		DependencyObject *c = child->GetAsDependencyObject();

		if (Type::IsSubclassOf (c->GetDeployment (), Type::STYLE, child->info->GetKind ())) {
			Value *v = c->GetValue (Style::TargetTypeProperty);
			if (v && v->GetKind () == Type::MANAGEDTYPEINFO)
				key = v->AsManagedTypeInfo ()->full_name;

			if (key)
				return key;
		}
	}

	return NULL;
}

static void
dependency_object_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child, bool fail_if_no_prop)
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	if (parent->element_type == XamlElementInstance::PROPERTY) {

		if (parent->info->RequiresManagedSet ())
			return;

		char **prop_name = g_strsplit (parent->element_name, ".", -1);
		Type *owner = types->Find (prop_name [0]);

		if (owner) {
			DependencyProperty *dep = DependencyProperty::GetDependencyProperty (Type::Find (p->deployment, owner->GetKind ()), prop_name [1]);

			g_strfreev (prop_name);

			if (!dep) {
				g_warning ("Unknown element: %s.", parent->element_name);
				if (fail_if_no_prop)
					parser_error (p, parent->element_name, NULL, 2007, "Unknown element: %s.", parent->element_name);
				return;
			}

			// XamlElementInfoEnum has Type::INVALID as
			// its kind, which is why that first check is
			// here.
			if (child->info->GetKind() != Type::MANAGED &&
			    !types->Find (child->info->GetKind())->IsCtorVisible()) {
				// we can't instantiate this type
				return parser_error (p, child->element_name, NULL, 2007,
						     "Unknown element: %s.", child->element_name);
			}

			// Don't add the child element, if it is the entire collection
			if (dep->GetPropertyType() == child->info->GetKind ())
				return;

			Type::Kind prop_type = dep->GetPropertyType ();
			if (!types->IsSubclassOf (prop_type, Type::DEPENDENCY_OBJECT_COLLECTION)
			    && !types->IsSubclassOf (prop_type, Type::RESOURCE_DICTIONARY))
				return;

			// Most common case, we will have a parent that we can snag the collection from
			DependencyObject *obj = (DependencyObject *) parent->parent->GetAsDependencyObject ();
			if (!obj)
				return;

			Value *col_v = obj->GetValue (dep);
			if (!col_v) {
				Type *col_type = types->Find (prop_type);
				DependencyObject *c_obj = col_type->CreateInstance ();
				obj->SetValue (dep, Value::CreateUnrefPtr (c_obj));
				col_v = obj->GetValue (dep);
				c_obj->unref ();
			}
			Collection *col = col_v->AsCollection ();
			MoonError err;

			if (types->IsSubclassOf (prop_type, Type::DEPENDENCY_OBJECT_COLLECTION)) {
				Value child_val (child->GetAsDependencyObject ());
				if (-1 == col->AddWithError (&child_val, &err))
					return parser_error (p, child->element_name, NULL, err.code, err.message);
			}
			else if (types->IsSubclassOf (prop_type, Type::RESOURCE_DICTIONARY)) {
				ResourceDictionary *dict = (ResourceDictionary *)col;

				const char *key = get_key_from_child (child);

				if (key == NULL) {
					// XXX don't know the proper values here...
					return parser_error (p, child->element_name, NULL, 2007,
							     "You must specify an x:Key or x:Name for elements in a ResourceDictionary");
				}

				Value *child_as_value = child->GetAsValue ();

				if (!child_as_value) {
					// XXX don't know the proper values here...
					return parser_error (p, child->element_name, NULL, 2007,
							     "Error adding child to ResourceDictionary");
				}

				bool added = dict->AddWithError (key, child_as_value, &err);
				if (!added)
					return parser_error (p, child->element_name, NULL, err.code, err.message);
			}

			return;
		}

		return;
	}

	if (types->IsSubclassOf (parent->info->GetKind (), Type::DEPENDENCY_OBJECT_COLLECTION)) {
		Collection *col = (Collection *) parent->GetAsDependencyObject ();
		MoonError err;
		Value child_val ((DependencyObject*)child->GetAsDependencyObject ());

		if (-1 == col->AddWithError (&child_val, &err))
			return parser_error (p, child->element_name, NULL, err.code, err.message);
		return;
	}
	else if (types->IsSubclassOf (parent->info->GetKind (), Type::RESOURCE_DICTIONARY)) {
		ResourceDictionary *dict = (ResourceDictionary *) parent->GetAsDependencyObject ();

		MoonError err;
		const char *key = get_key_from_child (child);

		if (key == NULL) {
			// XXX don't know the proper values here...
			return parser_error (p, child->element_name, NULL, 2007,
					     "You must specify an x:Key or x:Name for elements in a ResourceDictionary");
		}

		Value *child_as_value = child->GetAsValue ();
		bool added = dict->AddWithError (key, child_as_value, &err);
		if (!added)
			return parser_error (p, child->element_name, NULL, err.code, err.message);
	}


	if (parent->element_type != XamlElementInstance::PROPERTY) {
		parent->TrySetContentProperty (p, child);
	}

	// Do nothing if we aren't adding to a collection, or a content property collection
}


///
/// set property funcs
///

// these are just a bunch of special cases
static void
dependency_object_missed_property (XamlElementInstance *item, XamlElementInstance *prop, XamlElementInstance *value, char **prop_name)
{

}

static bool
set_managed_attached_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *prop, XamlElementInstance *value)
{
	if (!p->loader)
		return false;

	return p->loader->SetProperty (p, p->GetTopElementPtr (), item->info->xmlns, item->GetAsValue (), item, item->GetParentPointer (), prop->info->xmlns, prop->element_name, value->GetAsValue (), value);
}

static bool
dependency_object_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value, bool raise_errors)
{
	char **prop_name = g_strsplit (property->element_name, ".", -1);
	DependencyObject *dep = item->GetAsDependencyObject ();
	DependencyProperty *prop = NULL;
	bool res;
	Types *types = Deployment::GetCurrent ()->GetTypes ();

	if (types->Find (item->info->GetKind ())->IsValueType ()) {
		if (raise_errors) parser_error (p, item->element_name, NULL, -1, "Value types (%s) do not have properties.", property->element_name);
		g_strfreev (prop_name);
		return false;
	}

	if (types->Find (property->info->GetPropertyOwnerKind ())->IsCustomType ()) {
		g_strfreev (prop_name);
		return set_managed_attached_property (p, item, property, value);
	}

	if (!dep) {
		// FIXME is this really where this check should live
		if (raise_errors)
			parser_error (p, item->element_name, NULL, 2030,
			      "Property element %s cannot be used inside another property element.",
			      property->element_name);
	
		g_strfreev (prop_name);
		return false;
	}

	prop = DependencyProperty::GetDependencyProperty (Type::Find (p->deployment, item->info->GetKind ()), prop_name [1]);

	if (prop) {
		if (prop->IsReadOnly ()) {
			if (raise_errors)
				parser_error (p, item->element_name, NULL, 2014,
					"The attribute %s is read only and cannot be set.", prop->GetName ());
			res = false;
		} else if (types->IsSubclassOf (value->info->GetKind (), prop->GetPropertyType())) {
			// an empty collection can be NULL and valid
			if (item->IsPropertySet (prop->GetName())) {
				if (raise_errors)
					parser_error (p, item->element_name, NULL, 2033,
						"Cannot specify the value multiple times for property: %s.",
						property->element_name);
				res = false;
			} else {
				MoonError err;

				// HACK - since the Setter is added to the collection *before* its properties are set
				// we find ourselves with a sealed Setter - which should not be possible at the parse time
				SetterBase *sb = NULL;
				if (types->IsSubclassOf (dep->GetObjectType (), Type::SETTERBASE)) {
					sb = (SetterBase*) dep;
					sb->SetIsSealed (false);
				}

				if (!is_managed_kind (value->info->GetKind ()) && !value->info->RequiresManagedSet()) {
					if (!dep->SetValueWithError (prop, value->GetAsValue (), &err)) {
						if (raise_errors)
							parser_error (p, item->element_name, NULL, err.code, err.message);
						res = false;
						goto cleanup;
					}
					
				} else {
					if (!p->loader->SetProperty (p, p->GetTopElementPtr (), NULL, item->GetAsValue (), item, item->GetParentPointer (), NULL, prop_name [1], value->GetAsValue (), NULL)) {
						if (raise_errors)
							parser_error (p, item->element_name, NULL, err.code, err.message);
						res = false;
						goto cleanup;
					}
				}
					
					
				// re-seal the Setter (end-HACK)
				if (sb)
					sb->SetIsSealed (true);
					
				item->MarkPropertyAsSet (prop->GetName());
				res = true;
			}
		} else if (types->IsSubclassOf (prop->GetPropertyType (), Type::COLLECTION) || types->IsSubclassOf (prop->GetPropertyType (), Type::RESOURCE_DICTIONARY)) {
			// The items were added in add_child
			return true;
		} else {
			if (raise_errors)
				parser_error (p, item->element_name, NULL, 2010, "does not support %s as content.", value->element_name);
			res = false;
		}
	} else {
		dependency_object_missed_property (item, property, value, prop_name);
		res = false;
	}

cleanup:
	g_strfreev (prop_name);
	return res;
}

bool
xaml_set_property_from_str (DependencyObject *obj, DependencyProperty *prop, const char *value, MoonError *error)
{
	Value *v = NULL;
	bool rv = true;

	if (!value_from_str (prop->GetPropertyType(), prop->GetName(), value, &v))
		return false;
	
	// it's possible for (a valid) value to be NULL (and we must keep the default value)
	if (v) {
		rv = obj->SetValueWithError (prop, v, error);
		delete v;
	}
	
	return rv;
}

bool
xaml_is_valid_event_name (Deployment *deployment, Type::Kind kind, const char *name, bool allow_desktop_events)
{
	Type *type = Type::Find (deployment, kind);
	if (!type)
		return false;

	int event_id = type->LookupEvent (name);
	if (event_id == -1)
		return false;

	if (!allow_desktop_events || (moonlight_flags & RUNTIME_INIT_DESKTOP_EXTENSIONS) == 0) {
		// if we're not allowing desktop-only events, or if the user hasn't allowed them,
		// return false if the name corresponds to one of them.
		if (!strcmp (name, "MouseRightButtonDown") ||
		    !strcmp (name, "MouseRightButtonUp") ||
		    !strcmp (name, "MouseWheel"))
		return false;
	}

	return true;
}

static void
value_type_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	// the only attributes value on value types seem to be x:Key
	// and x:Name, but reuse the generic namespace attribute stuff
	// anyway.
	//
	for (int i = 0; attr [i]; i += 2) {
		// Skip empty attrs
		if (attr[i + 1] == NULL || attr[i + 1][0] == '\0')
			continue;

		char **attr_name = g_strsplit (attr [i], "|", -1);

		if (attr_name [1]) {
			
			XamlNamespace *ns = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, attr_name [0]);

			if (!ns)
				return parser_error (p, item->element_name, attr[i], 7055, "undeclared prefix");

			ns->SetAttribute (p, item, attr_name [1], attr [i + 1]);

			g_strfreev (attr_name);

			// Setting managed attributes can cause errors galore
			if (p->error_args)
				return;

			continue;
		}

		g_strfreev (attr_name);
	}
}

static void
dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	GList *delay_att = NULL;

	for (int i = 0; attr [i]; i += 2) {

		if (p->error_args)
			return;

		// Setting attributes like x:Class can change item->item, so we
		// need to make sure we have an up to date pointer
		DependencyObject *dep = item->GetAsDependencyObject ();
		char **attr_name = g_strsplit (attr [i], "|", -1);

		if (attr_name [1]) {
			XamlNamespace *ns = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, attr_name [0]);

			if (ns != x_namespace) {
				delay_att = g_list_append (delay_att, GINT_TO_POINTER (i));
				g_strfreev (attr_name);
				continue;
			}

			if (!ns) {
				g_strfreev (attr_name);
				return parser_error (p, item->element_name, attr[i], 5055, "undeclared prefix");
			}

			ns->SetAttribute (p, item, attr_name [1], attr [i + 1]);

			g_strfreev (attr_name);

			// Setting managed attributes can cause errors galore
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
		if (atchname) {
			Type *attached_type = types->Find (atchname);
			if (attached_type)
				prop = DependencyProperty::GetDependencyProperty (attached_type, pname);
		} else {
			prop = DependencyProperty::GetDependencyProperty (Type::Find (p->deployment, item->info->GetKind ()), pname);
		}

		if (prop) {
			if (prop->GetId () == DependencyObject::NameProperty) {

				if (item->GetName ()) {
					parser_error (p, item->element_name, NULL, 2016, "Cannot specify both Name and x:Name attributes.");
					return;
				}

				// XXX toshok - I don't like doing this here... but it fixes airlines.
				item->SetKey (p, attr[i+1]);

				NameScope *scope = p->namescope;
				if (!item->GetAsDependencyObject ()->SetName (attr [i+1], scope)) {
					parser_error (p, item->element_name, NULL, 2028,
						      "The name already exists in the tree: %s.", attr [i+1]);
					g_free (atchname);
					g_list_free (delay_att);
					return;
				}
				continue;
			}

			if (prop->IsReadOnly ()) {
				parser_error (p, item->element_name, NULL, 2014,
					      "The attribute %s is read only and cannot be set.", prop->GetName ());
				g_free (atchname);
				g_list_free (delay_att);
				return;
			} 

			if (item->IsPropertySet (prop->GetName())) {
				parser_error (p, item->element_name, attr [i], 2033,
					      "Cannot specify the value multiple times for property: %s.", prop->GetName ());
				g_free (atchname);
				g_list_free (delay_att);
				return;
			}

			Value *v = NULL;
			bool v_set = false;
			char *attr_value = g_strdup (attr [i+1]);

			bool need_managed = false;
			if (attr[i+1][0] == '{') {
				if (attr[i+1][1] == '}') {
					// {} is an escape sequence so you can have strings like {StaticResource}
					char *nv = attr_value;
					attr_value = g_strdup (attr_value + 2);
					g_free (nv);
				}
				else if (attr[i+1][strlen(attr[i+1]) - 1] == '}') {
					need_managed = true; 
				}
			}

			if (!need_managed) {
				if (!value_from_str_with_parser (p, prop->GetPropertyType(), prop->GetName(), attr_value, &v, &v_set)) {
					delete v;
					g_free (attr_value);
					continue;
				}
			}

			Type::Kind propKind = prop->GetPropertyType ();

			if (need_managed || is_managed_kind (propKind) || types->Find (prop->GetOwnerType ())->IsCustomType () || (v && is_managed_kind (v->GetKind ()))) {
				bool str_value = false;
				if (!v_set) {
					v = new Value (attr [i + 1]); // Note that we passed the non escaped value, not attr_value
					v_set = true;
					str_value = true;
				}

//				printf ("setting managed property: %s::%s to %s=%s\n", dep->GetType ()->GetName (), prop->GetName (), attr [i], attr [i + 1]);
				if (p->loader->SetProperty (p, p->GetTopElementPtr (), NULL, item->GetAsValue (), item, item->GetParentPointer (), NULL, g_strdup (attr [i]), v, NULL)) {
					delete v;
					g_free (attr_value);
					continue;
				} else {
					if (str_value) {
						delete v;
						v = NULL;
					}
					if (!value_from_str_with_parser (p, prop->GetPropertyType(), prop->GetName(), attr_value, &v, &v_set)) {
						delete v;
						g_free (attr_value);
						continue;
					}
				}
					
			}

			if (!v_set && !value_is_explicit_null (attr [i + 1])) { // Check the non escaped value
				parser_error (p, item->element_name, attr [i], 2024, "Invalid attribute value %s for property %s.", attr [i+1], attr [i]);

				g_free (attr_value);
				g_free (atchname);
				g_list_free (delay_att);
				return;
			}

			MoonError err;
//			printf ("settng:  %s  %s  value type:  %s    prop type:  %s\n", attr [i], attr [i+1], v ? Type::Find (v->GetKind ())->GetName () : "--null--", Type::Find (prop->GetPropertyType ())->GetName ());
			if (!dep->SetValueWithError (prop, v, &err))
				parser_error (p, item->element_name, attr [i], err.code, err.message);
			else
				item->MarkPropertyAsSet (prop->GetName());				

			delete v;
			g_free (attr_value);
		} else {
			delay_att = g_list_append (delay_att, GINT_TO_POINTER (i));
		}

		if (atchname)
			g_free (atchname);
	}

	GList *walk = g_list_first (delay_att);
	while (walk) {
		int i = GPOINTER_TO_INT (walk->data);

		if (p->error_args)
			return;

		char **attr_name = g_strsplit (attr [i], "|", -1);

		if (attr_name [1]) {
			XamlNamespace *ns = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, attr_name [0]);

			if (ns == x_namespace) {
				// Skip these, they are handled earlier
				g_strfreev (attr_name);
				walk = walk->prev;
			}

			if (!ns) {
				g_strfreev (attr_name);
				parser_error (p, item->element_name, attr[i], 5055, "undeclared prefix");
				break;
			}

			ns->SetAttribute (p, item, attr_name [1], attr [i + 1]);

			g_strfreev (attr_name);

			// Setting managed attributes can cause errors galore
			if (p->error_args)
				break;
		} else {
			if (!item->SetUnknownAttribute (p, attr [i], attr [i + 1])) {
				parser_error (p, item->element_name, attr [i], 2012,
						"Unknown attribute %s on element %s.",
						attr [i], item->element_name);
				break;
			}
		}

		walk = walk->next;
	}

	g_list_free (delay_att);
}


static Value *
lookup_named_item (XamlElementInstance *top, const char *name)
{
	Types *types = Deployment::GetCurrent ()->GetTypes ();
	XamlElementInstance *inst = top;

	while (inst) {
		if (inst->element_type == XamlElementInstance::ELEMENT) {
			ResourceDictionary *rd = NULL;
			Type::Kind kind = inst->info->GetKind ();
			
			if (types->IsSubclassOf (kind, Type::FRAMEWORKELEMENT)) {
				rd = inst->GetAsDependencyObject ()->GetValue (UIElement::ResourcesProperty)->AsResourceDictionary ();
			} else if (types->IsSubclassOf (kind, Type::RESOURCE_DICTIONARY)) {
				rd = (ResourceDictionary*) inst->GetAsDependencyObject ();
			}
			
			if (rd) {
				bool exists;
				Value *res = lookup_resource_dictionary (rd, name, &exists);
				if (exists)
					return res;
			}
		}
		
		inst = inst->parent;
	}

	return NULL;
}

static Value *
lookup_resource_dictionary (ResourceDictionary *rd, const char *name, bool *exists)
{
	*exists = false;
	Value *resource_value = rd->Get (name, exists);
	return *exists ? new Value (*resource_value) : NULL;
}

Value *
xaml_lookup_named_item (void *parser, void *instance, const char* name)
{
	XamlParserInfo *p = (XamlParserInfo *) parser;
	XamlElementInstance *inst = (XamlElementInstance *) instance;
	Value *res = NULL;

	if (inst)
		res = lookup_named_item (inst, name);

	XamlContext *context = p->loader->GetContext ();
	if (!res && context)
		context->internal->LookupNamedItem (name, &res);

	if (!res) {
		Application *app = Application::GetCurrent ();
		if (app) {
			ResourceDictionary *rd = app->GetResources ();

			bool exists = false;
			res = lookup_resource_dictionary (rd, name, &exists);

			if (res && Type::IsSubclassOf (p->deployment, res->GetKind (), Type::DEPENDENCY_OBJECT)) {
				DependencyObject *dob = res->AsDependencyObject ();
				NameScope::SetNameScope (dob, dob->FindNameScope ());
			}
		}
	}

	return res;
}

void *
xaml_get_template_parent (void *parser, void *element_instance)
{
	XamlParserInfo *p = (XamlParserInfo *) parser;
	XamlElementInstance *item = (XamlElementInstance *) element_instance;

	return p->GetTemplateParent (item);
}

char *
xaml_get_element_key (void *parser, void *element_instance)
{
	XamlElementInstance *item = (XamlElementInstance *) element_instance;
	const char *key = item->GetKey ();
	if (!key)
		key = item->GetName ();	
	return g_strdup (key);
}

char *
xaml_get_element_name (void *parser, void *element_instance)
{
	XamlElementInstance *item = (XamlElementInstance *) element_instance;
	return g_strdup (item->element_name);
}

bool
xaml_is_property_set (void *parser, void *element_instance, char *name)
{
	XamlElementInstance *item = (XamlElementInstance *) element_instance;
	return item->IsPropertySet (name);
}

void
xaml_mark_property_as_set (void *parser, void *element_instance, char *name)
{
	XamlElementInstance *item = (XamlElementInstance *) element_instance;
	item->MarkPropertyAsSet (g_strdup (name));
}

void
xaml_delay_set_property (void *parser, void *element_instance, const char *xmlns, const char *name, const Value *value)
{
	XamlElementInstance *item = (XamlElementInstance *) element_instance;
	item->DelaySetProperty (xmlns, name, value);
}

void
xaml_init (void)
{
	default_namespace = new DefaultNamespace ();
	x_namespace = new XNamespace ();
	xml_namespace = new XmlNamespace ();
}
