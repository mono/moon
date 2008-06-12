/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#include <malloc.h>
#include <glib.h>
#include <stdlib.h>
#include <expat.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>


#include "xaml.h"
#include "error.h"
#include "shape.h"
#include "animation.h"
#include "geometry.h"
#include "text.h"
#include "media.h"
#include "list.h"
#include "rect.h"
#include "point.h"
#include "array.h"
#include "canvas.h"
#include "color.h"
#include "namescope.h"
#include "stylus.h"
#include "runtime.h"
#include "utils.h"
#include "control.h"

#if SL_2_0
#include "deployment.h"
#include "grid.h"
#endif

#ifdef DEBUG_XAML
#define d(x) x
#else
#define d(x)
#endif

#ifdef XAML_WARNINGS
#define w(x) x
#else
#define w(x)
#endif


class XamlElementInfo;
class XamlElementInstance;
class XamlParserInfo;
class XamlNamespace;
class DefaultNamespace;
class XNamespace;
class XamlElementInfoNative;
class XamlElementInstanceNative;
class XamlElementInfoCustom;
class XamlElementInstanceCustom;

		

static DefaultNamespace *default_namespace = NULL;
static DefaultNamespace *deploy_namespace = NULL;
static XNamespace *x_namespace = NULL;

static const char* default_namespace_names [] = {
	"http://schemas.microsoft.com/winfx/2006/xaml/presentation",
	"http://schemas.microsoft.com/client/2007",
	"http://schemas.microsoft.com/xps/2005/06",
	NULL
};


typedef DependencyObject *(*create_item_func) (void);
typedef void  (*set_property_func) (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);
typedef void  (*set_attributes_func) (XamlParserInfo *p, XamlElementInstance *item, const char **attr);

XamlElementInstance* create_toplevel_property_element_instance (XamlParserInfo *p, const char *name);
XamlElementInstance* dependency_object_create_element_instance (XamlParserInfo *p, XamlElementInfo *i);
void dependency_object_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);
void dependency_object_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child);
void dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr);
void parser_error (XamlParserInfo *p, const char *el, const char *attr, int error_code, const char *message);

static XamlElementInstance *wrap_type (XamlParserInfo *p, Type *t);

static Type *get_type_for_property_name (const char* prop_name);


void  custom_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr);
void  custom_add_child      (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child);
void  custom_set_property   (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);


class XamlElementInstance : public List::Node {
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
	DependencyObject *item;

	GHashTable *set_properties;

	XamlElementInstance (XamlElementInfo *info, const char* element_name, ElementType type)
	{
		this->element_name = element_name;
		this->set_properties = NULL;
		this->element_type = type;
		this->parent = NULL;
		this->info = info;
		this->item = NULL;
		
		children = new List ();
	}
	
	virtual ~XamlElementInstance ()
	{
		children->Clear (true);
		delete children;

		if (set_properties)
			g_hash_table_destroy (set_properties);

		// if (instance_name)
		//	delete instance_name;
		// if (element_name && element_type == PROPERTY)
		//	delete element_name;
	}

	virtual void SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value)
	{
		MarkPropertyAsSet (property->element_name);
	}

	virtual void AddChild (XamlParserInfo *p, XamlElementInstance *child)
	{
	}

	virtual void SetAttributes (XamlParserInfo *p, const char **attr)
	{
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

	void ClearSetProperties ()
	{
		if (!set_properties)
			return;

#if GLIB_CHECK_VERSION(2,12,0)
		g_hash_table_remove_all (set_properties);
#else
		// this will cause the hash table to be recreated the
		// next time a property is set.
		g_hash_table_destroy (set_properties);
		set_properties = NULL;
#endif
	}

};

void 
unref_xaml_element (gpointer data, gpointer user_data)
{
	DependencyObject* dob = (DependencyObject*) data;
	//printf ("unref_xaml_element: %i\n", dob->id);
	base_unref (dob);
}

class XamlParserInfo {
 public:
	XML_Parser parser;

	const char *file_name;

	NameScope *namescope;
	XamlElementInstance *top_element;
	XamlNamespace *current_namespace;
	XamlElementInstance *current_element;

	GHashTable *namespace_map;
	bool cdata_content;
	GString *cdata;
	
	bool implicit_default_namespace;

	ParserErrorEventArgs *error_args;

	XamlLoader* loader;
	GList* created_elements;

	//
	// If set, this is used to hydrate an existing object, not to create a new toplevel one
	//
	DependencyObject *hydrate_expecting;
	bool hydrating;
	
	void AddCreatedElement (DependencyObject* element)
	{
		created_elements = g_list_prepend (created_elements, element);
	}

	XamlParserInfo (XML_Parser parser, const char *file_name) :
	  
		parser (parser), file_name (file_name), namescope (new NameScope()), top_element (NULL),
		current_namespace (NULL), current_element (NULL),
		cdata_content (false), cdata (NULL), implicit_default_namespace (false), error_args (NULL),
		loader (NULL), created_elements (NULL), hydrate_expecting(NULL), hydrating(false)
	{
		namespace_map = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
	}

	~XamlParserInfo ()
	{
		created_elements = g_list_reverse (created_elements);
		g_list_foreach (created_elements, unref_xaml_element, NULL);
		g_list_free (created_elements);

		g_hash_table_destroy (namespace_map);

		if (cdata)
			g_string_free (cdata, TRUE);
		if (top_element)
			delete top_element;
		namescope->unref ();
	}
};


class XamlElementInfo {
 public:
	const char *name;
	XamlElementInfo *parent;

	XamlElementInfo (const char *name, Type::Kind kind) : name (name), kind (kind)
	{
	}

	virtual Type::Kind GetKind () { return kind; }

	virtual const char* GetContentProperty () = 0;
	virtual XamlElementInstance* CreateElementInstance (XamlParserInfo *p) = 0;
	virtual XamlElementInstance* CreateWrappedElementInstance (XamlParserInfo *p, DependencyObject *o) = 0;
	virtual XamlElementInstance* CreatePropertyElementInstance (XamlParserInfo *p, const char *name) = 0;

 protected:
	Type::Kind kind;
};



class XamlElementInfoNative : public XamlElementInfo {
 public:
	XamlElementInfoNative (Type *t) : XamlElementInfo (t->name, t->type)
	{
		type = t;
	}

	Type* GetType ()
	{
		return type;
	}

	const char* GetName ()
	{
		return type->name;
	}

	const char* GetContentProperty ();

	XamlElementInstance* CreateElementInstance (XamlParserInfo *p);
	XamlElementInstance* CreateWrappedElementInstance (XamlParserInfo *p, DependencyObject *o);
	XamlElementInstance* CreatePropertyElementInstance (XamlParserInfo *p, const char *name);

 private:
	Type* type;

};


class XamlElementInstanceNative : public XamlElementInstance {

 public:
	XamlElementInstanceNative (XamlElementInfoNative *element_info, XamlParserInfo *parser_info, const char *name, ElementType type, bool create_item = true);

	virtual DependencyObject *CreateItem ();

	virtual void SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value);
	virtual void AddChild (XamlParserInfo *p, XamlElementInstance *child);
	virtual void SetAttributes (XamlParserInfo *p, const char **attr);

 private:
	XamlElementInfoNative *element_info;
	XamlParserInfo *parser_info;
};



class XamlNamespace {
 public:
	const char *name;

	XamlNamespace () : name (NULL) { }

	virtual ~XamlNamespace () { }
	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el) = 0;
	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value, bool *reparse) = 0;
};

class DefaultNamespace : public XamlNamespace {
 public:
	GHashTable *element_map;

	DefaultNamespace (GHashTable *element_map) : element_map (element_map) { }

	virtual ~DefaultNamespace () { }

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el)
	{
		Type* t = Type::Find (el);
		if (!t) {
			if (!strchr (el, '.'))
				g_critical ("Type not found:  %s\n", el);
			return NULL;
		}

		return new XamlElementInfoNative (t);
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value, bool *reparse)
	{
		return false;
	}
};

class XNamespace : public XamlNamespace {
 public:
	GHashTable *element_map;

	XNamespace (GHashTable *element_map) : element_map (element_map) { }

	virtual ~XNamespace () { }

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el)
	{
		return NULL;
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value, bool *reparse)
	{
		*reparse = false;

		if (!strcmp ("Name", attr)) {

			//
			// Causes breakage in airlines
			// Maybe x:Name overwrites but Name does not?
			//
			// if (p->namescope->FindName (value)) {
			//	parser_error (p, p->current_element->element_name, "x:Name", 2028, g_strdup_printf ("The name already exists in the tree: %s.", value));
			//	return false;
			// }
			//

			p->namescope->RegisterName (value, (DependencyObject *) item->item);
			item->item->SetValue (DependencyObject::NameProperty, Value (value));
			if (p->loader) {
				p->loader->SetNameAttribute (item->item, value);
				return true;
			}

			return false;
		}

		if (!strcmp ("Class", attr)) {

			// While hydrating, we do not need to create the toplevel class, its created already
			if (p->hydrating)
				return TRUE;

			//
			// FIXME: On Silverlight 2.0 only the toplevel node should contain an x:Class
			// must validate that.
			//
			DependencyObject *old = item->item;

			item->item = NULL;
			DependencyObject *dob = NULL;
			if (p->loader) {
				// The managed DependencyObject will unref itself
				// once it's finalized, so don't unref anything here.
				dob = p->loader->CreateManagedObject (value, NULL);
			}

			if (!dob) {
				parser_error (p, item->element_name, attr, -1,
					      g_strdup_printf ("Unable to resolve x:Class type '%s'\n", value));
				return false;
			}

			// Special case the namescope for now, since attached properties aren't copied
			NameScope *ns = NameScope::GetNameScope (old);
			if (ns)
				NameScope::SetNameScope (dob, ns);
			item->item = dob;

			p->AddCreatedElement (item->item);

			*reparse = true;
			return true;
		}

		return false;
	}
};


class XamlElementInfoCustom : public XamlElementInfo {

 public:
	XamlElementInfoCustom (const char *name, XamlElementInfo *parent, Type::Kind dependency_type, DependencyObject *dob) : XamlElementInfo (name, dependency_type)
	{
		this->dependency_object = dob;
	}

	const char* GetContentProperty () { return NULL; }

	XamlElementInstance* CreateElementInstance (XamlParserInfo *p);
	XamlElementInstance* CreateWrappedElementInstance (XamlParserInfo *p, DependencyObject *o);
	XamlElementInstance* CreatePropertyElementInstance (XamlParserInfo *p, const char *name);

 private:
	DependencyObject *dependency_object;
};


class XamlElementInstanceCustom : public XamlElementInstance {

 public:
	XamlElementInstanceCustom (XamlElementInfo *info, const char *name, ElementType type, DependencyObject *dob);

	virtual void SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value);
	virtual void AddChild (XamlParserInfo *p, XamlElementInstance *child);
	virtual void SetAttributes (XamlParserInfo *p, const char **attr);
};


class CustomNamespace : public XamlNamespace {

 public:
	char *xmlns;

	CustomNamespace (char *xmlns) :
		xmlns (xmlns)
	{
	}

	virtual ~CustomNamespace () { }

	virtual XamlElementInfo* FindElement (XamlParserInfo *p, const char *el)
	{
		DependencyObject *dob = NULL;
		if (p->loader)
			dob = p->loader->CreateManagedObject (xmlns, el);

		if (!dob) {
			parser_error (p, el, NULL, -1, g_strdup_printf ("Unable to resolve custom type %s\n", el));
			return NULL;
		}

		XamlElementInfoCustom *info = new XamlElementInfoCustom (g_strdup (el), NULL, dob->GetObjectType (), dob);

		return info;
	}

	virtual bool SetAttribute (XamlParserInfo *p, XamlElementInstance *item, const char *attr, const char *value, bool *reparse)
	{
		if (p->loader)
			return p->loader->SetAttribute (item->item, attr, value);
		return false;
	}
};


/*
	XamlLoader
*/
DependencyObject* 
XamlLoader::CreateManagedObject (const char* xmlns, const char* name)
{
	DependencyObject* result = NULL;
	char *assembly = NULL, *ns = NULL, *type_name = NULL;
	
	//printf ("XamlLoader::CreateManagedObject (%s, %s)\n", xmlns, name);

	xaml_parse_xmlns (xmlns, &type_name, &ns, &assembly);
	
	//printf ("XamlLoader::CreateManagedObject: assembly: %s, ns: %s, typename: %s\n", assembly, ns, type_name);

	if (!assembly) {
		//printf ("XamlLoader::CreateManagedObject (%s, %s): Invalid assembly: %s\n", xmlns, name, assembly);
		goto cleanup;
	}
	
	if (!vm_loaded && !LoadVM ())
		return NULL;
	
	if (type_name == NULL)
		type_name = g_strdup (name);
	
	//printf ("XamlLoader::CreateManagedObject: assembly: %s\n", assembly);
	
	result = CreateManagedObject (assembly, assembly, ns, type_name);
	
cleanup:
	g_free (assembly);
	g_free (type_name);
	g_free (ns);
	
	return result;

}

DependencyObject*
XamlLoader::CreateManagedObject (const char* asm_name, const char* asm_path, const char* name, const char* type_name)
{
	//printf ("XamlLoader::CreateManagedObject (%s, %s, %s, %s)\n", asm_name, asm_path, name, type_name);
	
	if (callbacks.load_managed_object)
		return callbacks.load_managed_object (asm_name, asm_path, name, type_name);
		
	return NULL;
}

bool
XamlLoader::SetAttribute (void* target, const char* name, const char* value)
{
	if (callbacks.set_custom_attribute)
		return callbacks.set_custom_attribute (target, name, value);

	return false;
}

void 
XamlLoader::SetNameAttribute (void* target, const char* name)
{
	if (callbacks.set_name_attribute) {
		callbacks.set_name_attribute (target, name);
		return;
	}
}

bool
XamlLoader::HookupEvent (void* target, const char* name, const char* value)
{
	if (callbacks.hookup_event)
		return callbacks.hookup_event (target, name, value);
	
	//printf ("XamlLoader::HookupEvent (%p, %s, %s)\n", target, name, value);
	return false;
}

void
XamlLoader::AddMissing (const char* assembly)
{		
	//printf ("XamlLoader::AddMissing (%s).\n", assembly);
	
	if (!vm_loaded)
		LoadVM ();
	
	if (!missing_assemblies)
		missing_assemblies = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
		
	g_hash_table_insert (missing_assemblies, g_strdup (assembly), g_strdup (assembly));
}

void 
XamlLoader::RemoveMissing (const char* assembly)
{
	//printf ("XamlLoader::RemoveMissing ('%s').\n", assembly);

	g_hash_table_remove (missing_assemblies, assembly);
}

gboolean
xaml_loader_find_any (gpointer key, gpointer value, gpointer user_data)
{
	return TRUE;
}

const char*
XamlLoader::GetMissing ()
{
	if (!missing_assemblies)
		return NULL;

	return (const char*) g_hash_table_find (missing_assemblies, xaml_loader_find_any, NULL);
}

void
XamlLoader::InsertMapping (const char* key, const char* value)
{
	//printf ("XamlLoader::InsertMapping (%s, %s), insert_mapping: %p, mappings: %p\n", key, value, callbacks.insert_mapping, mappings);
	
	if (!mappings)
		mappings = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, g_free);
	g_hash_table_insert (mappings, g_strdup (key), g_strdup (value));
	
	if (callbacks.insert_mapping)
		callbacks.insert_mapping (key, value);
}

const char*
XamlLoader::GetMapping (const char* key)
{
	
	const char* result = NULL;
	if (mappings) {
		result = (const char*) g_hash_table_lookup (mappings, key);
		if (result) {
			// printf ("XamlLoader::GetMapping (%s): returning cached result: %s\n", key, result);
			return result;
		}
	}
	
	if (callbacks.get_mapping) {
		result = callbacks.get_mapping (key);
		 // printf ("XamlLoader::GetMapping (%s): returning managed result: %s\n", key, result);
	}

	return result;
}

XamlLoader::XamlLoader (const char* filename, const char* str, Surface* surface)
{
	this->filename = g_strdup (filename);
	this->str = g_strdup (str);
	this->surface = surface;
	surface->ref ();
	this->missing_assemblies = NULL;
	this->mappings = NULL;
	this->vm_loaded = false;
	this->error_args = NULL;
	
#if d(!)0
	if (!surface) {
		printf ("XamlLoader::XamlLoader ('%s', '%s', %p): Initializing XamlLoader without a surface.\n",
			filename, str, surface);
	}
#endif
}

XamlLoader::~XamlLoader ()
{
	g_free (filename);
	g_free (str);
	surface->unref ();
	if (missing_assemblies)
		g_hash_table_destroy (missing_assemblies);
	if (mappings)
		g_hash_table_destroy (mappings);
	
	mappings = NULL;
	missing_assemblies = NULL;
	surface = NULL;
	filename = NULL;
	str = NULL;
}

bool
XamlLoader::LoadVM ()
{
	return false;
}

XamlLoader* 
xaml_loader_new (const char* filename, const char* str, Surface* surface)
{
	return new XamlLoader (filename, str, surface);
}

void
xaml_loader_free (XamlLoader* loader)
{
	delete loader;
}

void
xaml_loader_add_missing (XamlLoader* loader, const char* file)
{
	if (!loader) {
		d(printf ("Trying to add missing file '%s' to a null loader.\n", file));
		return;
	}
	
	loader->AddMissing (file);
}

void 
xaml_loader_set_callbacks (XamlLoader* loader, XamlLoaderCallbacks callbacks)
{
	if (!loader) {
		d(printf ("Trying to set callbacks for a null object\n"));
		return;
	}
	
	loader->callbacks = callbacks;
	loader->vm_loaded = true;
}


//
// Called when we encounter an error.  Note that memory ownership is taken for everything
// except the message, this allows you to use g_strdup_printf when creating the error message
//
void
parser_error (XamlParserInfo *p, const char *el, const char *attr, int error_code, const char *message)
{
	// Already failed
	if (p->error_args)
		return;

	// if parsing fails too early it's not safe (i.e. sigsegv) to call some functions, e.g. XML_GetCurrentLineNumber
	bool report_line_col = (error_code != XML_ERROR_XML_DECL);
	int line_number = report_line_col ? XML_GetCurrentLineNumber (p->parser) : 0;
	int char_position = report_line_col ? XML_GetCurrentColumnNumber (p->parser) : 0;

	p->error_args = new ParserErrorEventArgs (message, p->file_name, line_number, char_position, error_code, el, attr);

	w(g_warning ("PARSER ERROR, STOPPING PARSING:  (%d) %s  line: %d   char: %d\n", error_code, message,
		     line_number, char_position));

	XML_StopParser (p->parser, FALSE);
}

void
expat_parser_error (XamlParserInfo *p, XML_Error expat_error)
{
	// Already had an error
	if (p->error_args)
		return;

	int error_code;
	char *message;
	
	d(printf ("expat error is:  %d\n", expat_error));
	
	switch (expat_error) {
	case XML_ERROR_DUPLICATE_ATTRIBUTE:
		error_code = 5031;
		message = g_strdup ("wfc: unique attribute spec");
		break;
	case XML_ERROR_UNBOUND_PREFIX:
		error_code = 5055;
		message = g_strdup ("undeclared prefix");
		break;
	case XML_ERROR_NO_ELEMENTS:
		error_code = 5000;
		message = g_strdup ("unexpected end of input");
		break;
	default:
		error_code = expat_error;
		message = g_strdup_printf ("Unhandled XML error %s", XML_ErrorString (expat_error));
		break;
	}

	parser_error (p, NULL, NULL, error_code, message);

	g_free (message);
}

static DependencyObject *
get_parent (XamlElementInstance *inst)
{
	XamlElementInstance *walk = inst;

	while (walk) {
		if (walk->element_type == XamlElementInstance::ELEMENT)
			return walk->item;
		walk = walk->parent;
	}

	return NULL;
}

static void
start_element (void *data, const char *el, const char **attr)
{
	XamlParserInfo *p = (XamlParserInfo *) data;
	XamlElementInfo *elem;
	XamlElementInstance *inst;

	elem = p->current_namespace->FindElement (p, el);

	if (p->error_args)
		return;

	if (elem) {
		if (p->hydrate_expecting){
			Type::Kind expecting_type =  p->hydrate_expecting->GetObjectType ();

			if (elem->GetKind () != expecting_type){
				parser_error (p, el, NULL, -1,
					      g_strdup_printf ("Invalid top-level element found %s, expecting %s", el,
							       Type::Find (expecting_type)->GetName ()));
				return;
			}
			inst = elem->CreateWrappedElementInstance (p, p->hydrate_expecting);
			p->hydrate_expecting = NULL;
		} else
			inst = elem->CreateElementInstance (p);

		if (!inst && !inst->item)
			return;

		if (!p->top_element) {
			p->top_element = inst;
			NameScope::SetNameScope (inst->item, p->namescope);
		} else {
			DependencyObject *parent = get_parent (p->current_element);
			if (parent) {
				inst->item->SetLogicalParent (parent);
			}
		}

		inst->SetAttributes (p, attr);

		// Setting the attributes can kill the item
		if (!inst->item)
			return;

		if (p->current_element){			
			if (p->current_element->info) {
				p->current_element->AddChild (p, inst);
				if (p->error_args)
					return;
			}
		}
	} else {
		// it's actually valid (from SL point of view) to have <Ellipse.Triggers> inside a <Rectangle>
		// however we can't add properties to something bad, like a <Recta.gle> element
		bool property = false;
		char *dot = strchr (el, '.');
		if (dot) {
			gchar *prop_elem = g_strndup (el, dot - el);
			property = (Type::Find (prop_elem) != NULL);
			g_free (prop_elem);
		}

		if (property) {
			XamlElementInfo *prop_info = (p->current_element ? p->current_element->info : NULL);

			if (!prop_info) {
				// hmmmmm.  this is legal but i is not exactly sure how to handle it
				inst = create_toplevel_property_element_instance (p, g_strdup (el));
			} else
				inst = prop_info->CreatePropertyElementInstance (p, g_strdup (el));

			if (attr [0] != NULL) {
				// It appears there is a bug in the error string but it matches the MS runtime
				parser_error (p, el, NULL, 2018, g_strdup_printf ("The element %s does not support attributes.", attr [0]));
				return;
			}

			if (!p->top_element) {
				Type* property_type = get_type_for_property_name (inst->element_name);
				if (property_type->IsSubclassOf (Type::COLLECTION)) {
					XamlElementInstance *wrap = wrap_type (p, property_type);
					NameScope::SetNameScope (wrap->item, p->namescope);
					p->top_element = wrap;
					p->current_element = wrap;
					return;
				}
			}
		} else {
			parser_error (p, el, NULL, 2007, g_strdup_printf ("Unknown element: %s.", el));
			return;
		}
	}

	inst->parent = p->current_element;

	if (p->current_element) {
		p->current_element->children->Append (inst);
	}
	p->current_element = inst;	
}

static void
flush_char_data (XamlParserInfo *p, const char *next_element)
{
	DependencyProperty *content;
	const char *prop_name = NULL;
	Type::Kind prop_type;
	
	if (!p->cdata || !p->current_element)
		return;

	if (p->current_element->info && p->current_element->element_type == XamlElementInstance::ELEMENT)
		prop_name = p->current_element->info->GetContentProperty ();

	if (!prop_name && p->cdata_content) {
		char *err = g_strdup_printf ("%s does not support text content.", p->current_element->element_name);
		parser_error (p, p->current_element->element_name, NULL, 2011, err);
		goto done;
	} else if (!prop_name) {
		goto done;
	}
	
	prop_type = p->current_element->info->GetKind ();
	content = DependencyObject::GetDependencyProperty (prop_type, prop_name);
	
	// TODO: There might be other types that can be specified here,
	// but string is all i have found so far.  If you can specify other
	// types, i should pull the property setting out of set_attributes
	// and use that code
	
	if ((content->value_type) == Type::STRING && p->cdata_content) {
		p->current_element->item->SetValue (content, Value (g_strstrip (p->cdata->str)));
	} else if (Type::Find (p->current_element->info->GetKind ())->IsSubclassOf (Type::TEXTBLOCK)) {
		TextBlock *textblock = (TextBlock *) p->current_element->item;
		Inlines *inlines = textblock->GetInlines ();
		Collection::Node *last = inlines ? (Collection::Node *) inlines->list->Last () : NULL;
		DependencyObject *obj = last ? last->obj : NULL;
		
		if (!p->cdata_content) {
			if (next_element && !strcmp (next_element, "Run") && obj && obj->GetObjectType () == Type::RUN &&
			    !((Inline *) last->obj)->autogen) {
				// LWSP between <Run> elements is to be treated as a single-SPACE <Run> element
				// Note: p->cdata is already canonicalized
			} else {
				// This is one of the following cases:
				//
				// 1. LWSP before the first <Run> element
				// 2. LWSP after the last <Run> element
				// 3. LWSP between <Run> and <LineBreak> elements
				goto done;
			}
		} else {
			if (!next_element)
				g_strchomp (p->cdata->str);
			
			//if (next_element && !strcmp (next_element, "Run"))
			//	g_strchug (p->cdata->str);
			
			if (!obj || obj->GetObjectType () != Type::RUN || ((Inline *) last->obj)->autogen)
				g_strchug (p->cdata->str);
		}
		
		Run *run = new Run ();
		run->SetText (p->cdata->str);
		
		if (!inlines) {
			inlines = new Inlines ();
			textblock->SetInlines (inlines);
			inlines->unref ();
		}
		
		inlines->Add (run);
		run->unref ();
	}
	
done:
	
	if (p->cdata) {
		g_string_free (p->cdata, TRUE);
		p->cdata_content = false;
		p->cdata = NULL;
	}
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
	char *err;
	
	if (g_strv_length (name) == 2) {
		// Find the proper namespace for our next element
		next_namespace = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, name [0]);
		element = name [1];
	}
	
	if (!next_namespace && p->implicit_default_namespace) {
		// Use the default namespace for the next element
		next_namespace = default_namespace;
		element = name [0];
	}
	
	// Flush our cdata, passing the name of the next element so it
	// can do smart things for TextBlock cdata (and maybe others).
	flush_char_data (p, element);
	
	// Now update our namespace
	p->current_namespace = next_namespace;
	
	if (!p->current_namespace) {
		if (name [1])
			err = g_strdup_printf ("No handlers available for namespace: '%s' (%s)\n", name[0], el);
		else
			err = g_strdup_printf ("No namespace mapping available for element: '%s'\n", el);
		
		parser_error (p, name [1], NULL, -1, err);
		g_strfreev (name);
		return;
	}

	start_element (data, element, attr);

	g_strfreev (name);
}

static void
end_element_handler (void *data, const char *el)
{
	XamlParserInfo *info = (XamlParserInfo *) data;

	if (info->error_args)
		return;

	switch (info->current_element->element_type) {
	case XamlElementInstance::ELEMENT:
		flush_char_data (info, NULL);
		break;
	case XamlElementInstance::PROPERTY: {
		List::Node *walk = info->current_element->children->First ();
		while (walk) {
			XamlElementInstance *child = (XamlElementInstance *) walk;
			if (info->current_element->parent) {
				info->current_element->parent->SetProperty (info, info->current_element, child);
				break;
			}
			walk = walk->next;
		}
		flush_char_data (info, NULL);
		break;
	}
	}

	info->current_element = info->current_element->parent;
}

static void
char_data_handler (void *data, const char *in, int inlen)
{
	XamlParserInfo *p = (XamlParserInfo *) data;
	register const char *inptr = in;
	const char *inend = in + inlen;
	const char *start;
	
	if (p->error_args)
		return;
	
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

	if (p->error_args)
		return;

	for (int i = 0; default_namespace_names [i]; i++) {
		if (!strcmp (default_namespace_names [i], uri)) {
			g_hash_table_insert (p->namespace_map, g_strdup (uri), default_namespace);
			return;
		}
	}
		
	if (!strcmp ("http://schemas.microsoft.com/winfx/2006/xaml", uri)){
		g_hash_table_insert (p->namespace_map, g_strdup (uri), x_namespace);
	} else if (!strcmp ("http://schemas.microsoft.com/client/2007/deployment", uri)){
		g_hash_table_insert (p->namespace_map, g_strdup (uri), deploy_namespace);
	} else {
		if (!p->loader) {
			return parser_error (p, (p->current_element ? p->current_element->element_name : NULL), prefix, -1,
					     g_strdup_printf ("No custom element callback installed to handle %s", uri));
		}

		if (!prefix) {
			parser_error (p, (p->current_element ? p->current_element->element_name : NULL), NULL, 2262,
				      g_strdup ("AG_E_PARSER_NAMESPACE_NOT_SUPPORTED"));
			return;
		}
		
		CustomNamespace *c = new CustomNamespace (g_strdup (uri));
		g_hash_table_insert (p->namespace_map, g_strdup (c->xmlns), c);
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

	if (sysid)
		parser_error (p, NULL, NULL, 5050, g_strdup ("DTD was found but is prohibited"));
}

static void
add_default_namespaces (XamlParserInfo *p)
{
	p->implicit_default_namespace = true;
	g_hash_table_insert (p->namespace_map, g_strdup ("http://schemas.microsoft.com/winfx/2006/xaml/presentation"), default_namespace);
	g_hash_table_insert (p->namespace_map, g_strdup ("http://schemas.microsoft.com/winfx/2006/xaml"), x_namespace);
}

static void
print_tree (XamlElementInstance *el, int depth)
{
	for (int i = 0; i < depth; i++)
		printf ("\t");
	
	Value *v = NULL;

	if (el->element_type == XamlElementInstance::ELEMENT)
		v = el->item->GetValue (DependencyObject::NameProperty);
	printf ("%s  (%s)  (%p)\n", el->element_name, v ? v->AsString () : "-no name-", el->parent);
	
	for (List::Node *walk = el->children->First (); walk != NULL; walk = walk->next) {
		XamlElementInstance *el = (XamlElementInstance *) walk;
		print_tree (el, depth + 1);
	}
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

DependencyObject *
xaml_create_from_file (XamlLoader* loader, const char *xaml_file, bool create_namescope,
		       Type::Kind *element_type)
{
	DependencyObject *res = NULL;
	XamlParserInfo *parser_info = NULL;
	XML_Parser p = NULL;
	bool first_read = true;
	const char *inptr, *inend;
	TextStream *stream;
	char buffer[4096];
	ssize_t nread;
	
	d(printf ("attemtping to load xaml file: %s\n", xaml_file));
	
	stream = new TextStream ();
	if (!stream->Open (xaml_file, false)) {
		d(printf ("can not open file\n"));
		goto cleanup_and_return;
	}
	
	if (!(p = XML_ParserCreateNS ("UTF-8", '|'))) {
		d(printf ("can not create parser\n"));
		goto cleanup_and_return;
	}

	parser_info = new XamlParserInfo (p, xaml_file);
	
	parser_info->namescope->SetTemporary (!create_namescope);

	parser_info->loader = loader;

	// TODO: This is just in here temporarily, to make life less difficult for everyone
	// while we are developing.  
	add_default_namespaces (parser_info);

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
		
		if (first_read) {
			// Remove preceding white space
			inend = buffer + nread;
			
			while (inptr < inend && isspace ((unsigned char) *inptr))
				inptr++;
			
			if (inptr == inend)
				continue;
			
			nread = (inend - inptr);
			first_read = false;
		}
		
		if (!XML_Parse (p, inptr, nread, nread == 0)) {
			expat_parser_error (parser_info, XML_GetErrorCode (p));
			goto cleanup_and_return;
		}
		
		if (nread == 0)
			break;
	}
	
	d (print_tree (parser_info->top_element, 0));
	
	if (parser_info->top_element) {
		res = parser_info->top_element->item;
		if (element_type)
			*element_type = parser_info->top_element->info->GetKind ();

		if (parser_info->error_args) {
			*element_type = Type::INVALID;
			goto cleanup_and_return;
		}
		
		res->ref ();
	}
	
 cleanup_and_return:
	
	if (!parser_info)
		loader->error_args = new ParserErrorEventArgs ("Error opening xaml file", xaml_file, 0, 0, 1, "", "");
	else if (parser_info->error_args)
		loader->error_args = parser_info->error_args;
	
	delete stream;
	
	if (p)
		XML_ParserFree (p);
	
	if (parser_info)
		delete parser_info;
	
	return res;
}

DependencyObject *
xaml_create_from_str (XamlLoader *loader, const char *xaml, bool create_namescope,
		      Type::Kind *element_type)
{
	return xaml_hydrate_from_str (loader, xaml, NULL, create_namescope, element_type);
}

/**
 * Hydrates an existing DependencyObject (@object) with the contents from the @xaml
 * data
 */
DependencyObject *
xaml_hydrate_from_str (XamlLoader *loader, const char *xaml, DependencyObject *object, bool create_namescope, Type::Kind *element_type)
{
	XML_Parser p = XML_ParserCreateNS ("utf-8", '|');
	XamlParserInfo *parser_info = NULL;
	DependencyObject *res = NULL;
	char *start = (char*)xaml;
	
	if (!p) {
		d(printf ("can not create parser\n"));
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

	parser_info->loader = loader;

	//
	// If we are hydrating, we are not null
	//
	if (object != NULL)
		object->ref ();
	
	parser_info->hydrate_expecting = object;
	parser_info->hydrating = true;
	
	// from_str gets the default namespaces implictly added
	add_default_namespaces (parser_info);

	XML_SetUserData (p, parser_info);

	XML_SetElementHandler (p, start_element_handler, end_element_handler);
	XML_SetCharacterDataHandler (p, char_data_handler);
	XML_SetNamespaceDeclHandler(p, start_namespace_handler, NULL);
	XML_SetDoctypeDeclHandler(p, start_doctype_handler, NULL);

	/*
	XML_SetProcessingInstructionHandler (p, proc_handler);
	*/

	// don't freak out if the <?xml ... ?> isn't on the first line (see #328907)
	while (isspace ((unsigned char) *start))
		start++;

	if (!XML_Parse (p, start, strlen (start), TRUE)) {
		expat_parser_error (parser_info, XML_GetErrorCode (p));
		d(printf ("error parsing:  %s\n\n", xaml));
		goto cleanup_and_return;
	}
	
	d(print_tree (parser_info->top_element, 0));
	
	if (parser_info->top_element) {
		res = parser_info->top_element->item;
		if (element_type)
			*element_type = parser_info->top_element->info->GetKind ();

		if (parser_info->error_args) {
			res = NULL;
			if (element_type)
				*element_type = Type::INVALID;
			goto cleanup_and_return;
		}
		if (object == NULL)
			res->ref ();
	}

 cleanup_and_return:
	
	if (parser_info->error_args) {
		loader->error_args = parser_info->error_args;
		printf ("Could not parse element %s, attribute %s, error: %s\n",
			loader->error_args->xml_element,
			loader->error_args->xml_attribute,
			loader->error_args->error_message);
	}
	
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

static int
parse_int (const char **pp, const char *end)
{
	const char *p = *pp;
#if false
	if (optional && AtEnd)
		return 0;
#endif

	int res = 0;
	int count = 0;

	while (p <= end && isdigit (*p)) {
		res = res * 10 + *p - '0';
		p++;
		count++;
	}

#if false
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

	while (mag > 0 && p <= end && isdigit (*p)) {
		res = res + (*p - '0') * mag;
		p++;
		mag = mag / 10;
		digitseen = true;
	}

#if false
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
	if (!g_strcasecmp ("Forever", str)) {
		*res = RepeatBehavior::Forever;
		return true;
	}

	// check for "<float>x".

	// XXX more validation work is needed here.. but how do we
	// report an error?
	char *x = strchr (str, 'x');
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
	if (!g_strcasecmp ("Automatic", str)) {
		*res = Duration::Automatic;
		return true;
	}

	if (!g_strcasecmp ("Forever", str)) {
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
keytime_from_str (const char* str, KeyTime *res)
{
	if (!g_strcasecmp ("Uniform", str)) {
		*res = KeyTime::Uniform;
		return true;
	}

	if (!g_strcasecmp ("Paced", str)) {
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
	int count = 0;
	Point *pts = point_array_from_str (str, &count);

	if (!pts || count != 2) {
		delete pts;
		return false;
	}

	*res = new KeySpline (pts [0], pts [1]);
	
	delete [] pts;
	
	return true;
}

Matrix *
matrix_from_str (const char *str)
{
	Matrix *matrix;
	int count = 0;

	double *values = double_array_from_str (str, &count);

	if (!values)
		return new Matrix ();

	if (count < 6) {
		delete [] values;
		return NULL;
	}

	matrix = new Matrix ();

	matrix_set_m11 (matrix, values [0]);
	matrix_set_m12 (matrix, values [1]);
	matrix_set_m21 (matrix, values [2]);
	matrix_set_m22 (matrix, values [3]);
	matrix_set_offset_x (matrix, values [4]);
	matrix_set_offset_y (matrix, values [5]);

	delete [] values;

	return matrix;
}


void
advance (char **in)
{
	char *inptr = *in;
	
	while (*inptr && !g_ascii_isalnum (*inptr) && *inptr != '.' && *inptr != '-' && *inptr != '+')
		inptr = g_utf8_next_char (inptr);
	
	*in = inptr;
}

bool
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

void
make_relative (const Point *cp, Point *mv)
{
	mv->x += cp->x;
	mv->y += cp->y;
}

bool
more_points_available (char **in)
{
	char *inptr = *in;
	
	while (g_ascii_isspace (*inptr) || *inptr == ',')
		inptr++;
	
	*in = inptr;
	
	return (g_ascii_isdigit (*inptr) || *inptr == '.' || *inptr == '-' || *inptr == '+');
}

Point *
get_point_array (char *in, GSList *pl, int *count, bool relative, Point *cp, Point *last)
{
	int c = *count;

	while (more_points_available (&in)) {
		Point *n = new Point ();
		
		if (!get_point (n, &in)) {
			delete n;
			break;
		}
		
		advance (&in);
		
		if (relative) make_relative (cp, n);

		pl = g_slist_append (pl, n);
		c++;
	}

	Point *t;
	Point *pts = new Point [c];
	for (int i = 0; i < c; i++) {
		t = (Point *) pl->data;
		pts [i].x = t->x;
		pts [i].y = t->y;

		// locally allocated points needs to be deleted
		if (i >= *count)
			delete t;
		pl = pl->next;
	}

	last = &pts [c - 1];
	*count = c;

	return pts;
}

Geometry *
geometry_from_str (const char *str)
{
	char *inptr = (char *) str;
	//int s; // For starting expression markers
	Point cp = Point (0, 0);
	Point cp1, cp2, cp3;
	char *end;

	PathFigure *pf = NULL;
	PathSegment *prev = NULL;
	PathSegmentCollection *psc = NULL;
	
	PathGeometry *pg = new PathGeometry ();
	PathFigureCollection *pfc = new PathFigureCollection ();
	pg->SetFigures (pfc);
	pfc->unref ();

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
			if (*inptr == '0')
				pg->SetFillRule (FillRuleEvenOdd);
			else if (*inptr == '1')
				pg->SetFillRule (FillRuleNonzero);
			
			// FIXME: else it's a bad value and nothing should be rendered
			inptr = g_utf8_next_char (inptr);
			break;
		case 'm':
			relative = true;
		case 'M':
			if (!get_point (&cp1, &inptr))
				break;
			
			if (relative)
				make_relative (&cp, &cp1);

			if (pf) {
				pfc->Add (pf);
				pf->unref ();
			}

			pf = new PathFigure ();
			psc = new PathSegmentCollection ();
			pf->SetSegments (psc);
			psc->unref ();

			pf->SetStartPoint (&cp1);

			prev = NULL;

			cp.x = cp1.x;
			cp.y = cp1.y;

			advance (&inptr);
			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp1);
				
				LineSegment *ls = new LineSegment ();
				ls->SetPoint (&cp1);

				psc->Add (ls);
				ls->unref ();
				prev = ls;
			}

			cp.x = cp1.x;
			cp.y = cp1.y;
			break;

		case 'l':
			relative = true;
		case 'L':
		{
			if (!psc)
				return pg;

			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp1);

				LineSegment *ls = new LineSegment ();
				ls->SetPoint (&cp1);

				psc->Add (ls);
				ls->unref ();
				prev = ls;

				cp.x = cp1.x;
				cp.y = cp1.y;

				advance (&inptr);
			}
			break;
		}
		
		case 'h':
			relative = true;
		case 'H':
		{
			if (!psc)
				return pg;

			double x = g_ascii_strtod (inptr, &end);
			if (end == inptr)
				break;
			
			inptr = end;
			
			if (relative)
				x += cp.x;
			cp = Point (x, cp.y);

			LineSegment *ls = new LineSegment ();
			ls->SetPoint (&cp);

			psc->Add (ls);
			ls->unref ();
			prev = ls;

			break;
		}
		
		case 'v':
			relative = true;
		case 'V':
		{
			if (!psc)
				return pg;

			double y = g_ascii_strtod (inptr, &end);
			if (end == inptr)
				break;
			
			inptr = end;
			
			if (relative)
				y += cp.y;
			cp = Point (cp.x, y);

			LineSegment *ls = new LineSegment ();
			ls->SetPoint (&cp);

			psc->Add (ls);
			ls->unref ();
			prev = ls;

			break;
		}
		
		case 'c':
			relative = true;
		case 'C':
		{
			if (!psc)
				return pg;

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
			
			if (more_points_available (&inptr)) {
				GSList *pl = NULL;
				int count = 3;

				pl = g_slist_append (pl, &cp1);
				pl = g_slist_append (pl, &cp2);
				pl = g_slist_append (pl, &cp3);

				Point last;
				Point *pts = get_point_array (inptr, pl, &count, relative, &cp, &last);
				PolyBezierSegment *pbs = new PolyBezierSegment ();
				pbs->SetPoints (pts, count);

				psc->Add (pbs);
				pbs->unref ();
				prev = pbs;

				cp.x = last.x;
				cp.y = last.y;

				g_slist_free (pl);
			} else {
				BezierSegment *bs = new BezierSegment ();
				bs->SetPoint1 (&cp1);
				bs->SetPoint2 (&cp2);
				bs->SetPoint3 (&cp3);
				
				psc->Add (bs);
				bs->unref ();
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
			if (!psc)
				return pg;

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

				if (prev && prev->GetObjectType () == Type::BEZIERSEGMENT) {
					Point *p = ((BezierSegment *) prev)->GetPoint2 ();
					cp1.x = 2 * cp.x - p->x;
					cp1.y = 2 * cp.y - p->y;
				} else
					cp1 = cp;

				BezierSegment *bs = new BezierSegment ();
				bs->SetPoint1 (&cp1);
				bs->SetPoint2 (&cp2);
				bs->SetPoint3 (&cp3);

				psc->Add (bs);
				bs->unref ();
				prev = bs;

				cp.x = cp3.x;
				cp.y = cp3.y;

				advance (&inptr);
			}
			break;
		}
		case 'q':
			relative = true;
		case 'Q':
		{
			if (!psc)
				return pg;

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
			if (more_points_available (&inptr)) {
				GSList *pl = NULL;
				int count = 2;

				pl = g_slist_append (pl, &cp1);
				pl = g_slist_append (pl, &cp2);

				Point last;
				Point *pts = get_point_array (inptr, pl, &count, relative, &cp, &last);
				PolyQuadraticBezierSegment *pqbs = new PolyQuadraticBezierSegment ();
				pqbs->SetPoints (pts, count);

				psc->Add (pqbs);
				pqbs->unref ();
				prev = pqbs;

				cp.x = last.x;
				cp.y = last.y;

				g_slist_free (pl);
			} else {
				QuadraticBezierSegment *qbs = new QuadraticBezierSegment ();
				qbs->SetPoint1 (&cp1);
				qbs->SetPoint2 (&cp2);

				psc->Add (qbs);
				qbs->unref ();
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
			if (!psc)
				return pg;

			while (more_points_available (&inptr)) {
				if (!get_point (&cp2, &inptr))
					break;
				
				if (relative)
					make_relative (&cp, &cp2);

				if (prev && prev->GetObjectType () == Type::QUADRATICBEZIERSEGMENT) {
					Point *p = ((QuadraticBezierSegment *) prev)->GetPoint1 ();
					cp1.x = 2 * cp.x - p->x;
					cp1.y = 2 * cp.y - p->y;
				} else
					cp1 = cp;

				QuadraticBezierSegment *qbs = new QuadraticBezierSegment ();
				qbs->SetPoint1 (&cp1);
				qbs->SetPoint2 (&cp2);
				
				psc->Add (qbs);
				qbs->unref ();
				prev = qbs;

				cp.x = cp2.x;
				cp.y = cp2.y;

				advance (&inptr);
			}
				
			break;
		}
		case 'a':
			relative = true;
		case 'A':
		{
			if (!psc)
				return pg;

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

				ArcSegment *arc = new ArcSegment ();
				arc->SetIsLargeArc ((bool) is_large);
				arc->SetSweepDirection ((SweepDirection) sweep);
				arc->SetRotationAngle (angle);
				arc->SetSize (&cp1);
				arc->SetPoint (&cp2);

				psc->Add (arc);
				arc->unref ();
				prev = arc;
					
				cp.x = cp2.x;
				cp.y = cp2.y;

				advance (&inptr);
			}
			break;
		}
		case 'z':
		case 'Z': {
			if (!psc)
				return pg;

			Point *p = pf->GetStartPoint ();

			if (!p)
				p = new Point (0, 0);

			LineSegment *ls = new LineSegment ();
			ls->SetPoint (p);
			
			psc->Add (ls);
			ls->unref ();

			prev = NULL;
			pf->SetIsClosed (true);
			
			cp.x = p->x;
			cp.y = p->y;
			
			if (pf) {
				pfc->Add (pf);
				pf->unref ();
			}

			pf = new PathFigure ();
			psc = new PathSegmentCollection ();
			pf->SetStartPoint (p);
			pf->SetSegments (psc);
			psc->unref ();
			
			break;
		}
		default:
			break;
		}
	}

	if (pf) {
		pfc->Add (pf);
		pf->unref ();
	}
	
	return pg;
}

bool
value_from_str_with_typename (const char *type_name, const char *prop_name, const char *str, Value **v)
{
	Type *t = Type::Find (type_name);
	if (!t)
		return false;

	return value_from_str (t->type, prop_name, str, v);
}

#define IS_NULL_OR_EMPTY(str)	(!str || (*str == 0))

bool
value_from_str (Type::Kind type, const char *prop_name, const char *str, Value** v)
{
	char *endptr;
	*v = NULL;
	
	if (!strcmp ("{x:Null}", str)) {
		*v = NULL;
		return true;
	}
	
	switch (type) {
	case Type::BOOL: {
		bool b;
		if (!g_strcasecmp ("true", str))
			b = true;
		else if (!g_strcasecmp ("false", str))
			b = false;
		else {
			// Check if it's a string representing a decimal value
			gint64 l;

			errno = 0;
			l = strtol (str, &endptr, 10);

			if (errno || endptr == str || *endptr || l > G_MAXINT32 || l < G_MININT32)
				return false;

			if (l == 0)
				b = false;
			else
				b = true;
		}
				

		*v = new Value (b);
		break;
	}
	case Type::DOUBLE: {
		double d;

		// empty string should not reset default values with 0
		if (IS_NULL_OR_EMPTY(str))
			return true;

		errno = 0;
		d = g_ascii_strtod (str, &endptr);

		if (errno || endptr == str || *endptr)
			return false;

		*v = new Value (d);
		break;
	}
	case Type::INT64: {
		gint64 l;

		errno = 0;
		l = strtol (str, &endptr, 10);

		if (errno || endptr == str || *endptr)
			return false;

		*v = new Value (l, Type::INT64);
		break;
	}
	case Type::TIMESPAN: {
		TimeSpan ts;

		if (!time_span_from_str (str, &ts))
			return false;

		*v = new Value (ts, Type::TIMESPAN);
		break;
	}
	case Type::INT32: {
		int i;

		if (isalpha (str [0]) && prop_name) {
			i = enums_str_to_int (prop_name, str);
			if (i == -1) {
				g_warning ("'%s' enum is not valid on '%s' property", str, prop_name);
				return false;
			}
		} else {
			errno = 0;
			long l = strtol (str, &endptr, 10);

			if (errno || endptr == str || *endptr)
				return false;

			i = (int) l;
		}

		*v = new Value (i);
		break;
	}
	case Type::STRING: {
		*v = new Value (str);
		break;
	}
	case Type::COLOR: {
		Color *c = color_from_str (str);
		if (c == NULL)
			return false;
		*v = new Value (*c);
		delete c;
		break;
	}
	case Type::REPEATBEHAVIOR: {
		RepeatBehavior rb = RepeatBehavior::Forever;

		if (!repeat_behavior_from_str (str, &rb))
			return false;

		*v = new Value (rb);
		break;
	}
	case Type::DURATION: {
		Duration d = Duration::Forever;

		if (!duration_from_str (str, &d))
			return false;

		*v = new Value (d);
		break;
	}
	case Type::KEYTIME: {
		KeyTime kt = KeyTime::Paced;

		if (!keytime_from_str (str, &kt))
			return false;

		*v = new Value (kt);
		break;
	}
	case Type::KEYSPLINE: {
		KeySpline *ks;

		if (!key_spline_from_str (str, &ks))
			return false;

		*v = Value::CreateUnrefPtr (ks);
		break;
	}
	case Type::BRUSH:
	case Type::SOLIDCOLORBRUSH: {
		// Only solid color brushes can be specified using attribute syntax
		SolidColorBrush *scb = new SolidColorBrush ();
		Color *c = color_from_str (str);
		
		if (c == NULL)
			return false;
		
		scb->SetColor (c);
		delete c;
		
		*v = new Value (scb);
		scb->unref ();
		break;
	}
	case Type::POINT: {
		Point p;

		if (!point_from_str (str, &p))
			return false;

		*v = new Value (p);
		break;
	}
	case Type::RECT: {
		Rect rect;

		if (!rect_from_str (str, &rect))
			return false;

		*v = new Value (rect);
		break;
	}
	case Type::DOUBLE_ARRAY: {
		int count = 0;
		double *doubles = double_array_from_str (str, &count);

		*v = new Value (doubles, count);
		break;
	}
	case Type::POINT_ARRAY:	{
		int count = 0;
		Point *points = point_array_from_str (str, &count);

		*v = new Value (points, count);
		break;
	}
	case Type::TRANSFORM: {
		if (IS_NULL_OR_EMPTY(str))
			return true;

		Matrix *mv = matrix_from_str (str);
		if (!mv)
			return false;

		MatrixTransform *t = new MatrixTransform ();
		t->SetValue (MatrixTransform::MatrixProperty, Value (mv));

		*v = new Value (t);
		t->unref ();
		mv->unref ();
		break;
	}
	case Type::MATRIX: {
		// note: unlike TRANSFORM this creates an empty, identity, matrix for an empty string
		Matrix *matrix = matrix_from_str (str);
		if (!matrix)
			return false;

		*v = new Value (matrix);
		matrix->unref ();
		break;
	}
	case Type::GEOMETRY: {
		Geometry *geometry = geometry_from_str (str);

		if (!geometry)
			return false;

		*v = new Value (geometry);
		geometry->unref ();
		break;
	}
	default:
		// we don't care about NULL or empty values
		return IS_NULL_OR_EMPTY(str);
	}

	return true;
}

XamlElementInstance *
create_toplevel_property_element_instance (XamlParserInfo *p, const char *name)
{
	return new XamlElementInstanceNative (NULL, p, name, XamlElementInstance::PROPERTY, false);
}

XamlElementInstance *
dependency_object_create_element_instance (XamlParserInfo *p, XamlElementInfo *i)
{
	/*
	XamlElementInstance *inst = new XamlElementInstance (i);

	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;

	DependencyProperty *dep = NULL;
	XamlElementInstance *walk = p->current_element;

	
	return inst;
	*/
	return NULL;
}

static XamlElementInstance *
wrap_type (XamlParserInfo *p, Type *t)
{
	XamlElementInfo *info = p->current_namespace->FindElement (p, t->name);
	XamlElementInstance *inst = info->CreateElementInstance (p);
	return inst;
}

const char *
XamlElementInfoNative::GetContentProperty ()
{
	return type->GetContentPropertyName ();
}

XamlElementInstance *
XamlElementInfoNative::CreateElementInstance (XamlParserInfo *p)
{
	return new XamlElementInstanceNative (this, p, GetName (), XamlElementInstance::ELEMENT);
}

XamlElementInstance *
XamlElementInfoNative::CreateWrappedElementInstance (XamlParserInfo *p, DependencyObject *o)
{
	XamlElementInstance *res = new XamlElementInstanceNative (this, p, GetName (), XamlElementInstance::ELEMENT, false);
	res->item = o;

	return res;
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
		item = CreateItem ();
}



DependencyObject *
XamlElementInstanceNative::CreateItem ()
{
	XamlElementInstance *walk = parser_info->current_element;
	Type *type = element_info->GetType ();

	DependencyObject *item = NULL;
	DependencyProperty *dep = NULL;

	if (type->IsSubclassOf (Type::COLLECTION)) {
		

		// If we are creating a collection, try walking up the element tree,
		// to find the parent that we belong to and using that instance for
		// our collection, instead of creating a new one

		// We attempt to advance past the property setter, because we might be dealing with a
		// content element

		
		if (walk && walk->element_type == XamlElementInstance::PROPERTY) {
			char **prop_name = g_strsplit (walk->element_name, ".", -1);
			
			walk = walk->parent;
			dep = DependencyObject::GetDependencyProperty (walk->info->GetKind (), prop_name [1]);

			g_strfreev (prop_name);
		} else if (walk && walk->info->GetContentProperty ()) {
			dep = DependencyObject::GetDependencyProperty (walk->info->GetKind (),
					(char *) walk->info->GetContentProperty ());			
		}

		if (dep && Type::Find (dep->value_type)->IsSubclassOf (type->type)) {
			Value *v = ((DependencyObject * ) walk->item)->GetValue (dep);
			if (v) {
				item = v->AsCollection ();
				dep = NULL;
			}
			// note: if !v then the default collection is NULL (e.g. PathFigureCollection)
		}
	}

	if (!item) {
		item = element_info->GetType ()->CreateInstance ();

		if (parser_info->loader)
			item->SetSurface (parser_info->loader->GetSurface ());

		// in case we must store the collection into the parent
		if (dep && dep->value_type == type->type)
			((DependencyObject * ) walk->item)->SetValue (dep, new Value (item));

		parser_info->AddCreatedElement (item);
	}

	return item;
}

void
XamlElementInstanceNative::SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value)
{
	dependency_object_set_property (p, this, property, value);
}

void
XamlElementInstanceNative::AddChild (XamlParserInfo *p, XamlElementInstance *child)
{
	dependency_object_add_child (p, this, child);
}

void
XamlElementInstanceNative::SetAttributes (XamlParserInfo *p, const char **attr)
{
	dependency_object_set_attributes (p, this, attr);
}


XamlElementInstance *
XamlElementInfoCustom::CreateElementInstance (XamlParserInfo *p)
{
	XamlElementInstanceCustom *inst = new XamlElementInstanceCustom (this, dependency_object->GetTypeName (), XamlElementInstance::ELEMENT, dependency_object);

	if (p->loader)
        	inst->item->SetSurface (p->loader->GetSurface ());
	p->AddCreatedElement (inst->item);

	return inst;
}

XamlElementInstance *
XamlElementInfoCustom::CreateWrappedElementInstance (XamlParserInfo *p, DependencyObject *o)
{
	XamlElementInstanceCustom *inst = new XamlElementInstanceCustom (this, o->GetTypeName (), XamlElementInstance::ELEMENT, o);

	if (p->loader)
        	inst->item->SetSurface (p->loader->GetSurface ());
	p->AddCreatedElement (inst->item);

	return inst;
}

XamlElementInstance *
XamlElementInfoCustom::CreatePropertyElementInstance (XamlParserInfo *p, const char *name)
{
	XamlElementInstanceCustom *inst = new XamlElementInstanceCustom (this, name, XamlElementInstance::PROPERTY, dependency_object);

	if (p->loader)
        	inst->item->SetSurface (p->loader->GetSurface ());
	p->AddCreatedElement (inst->item);

	return inst;
}

XamlElementInstanceCustom::XamlElementInstanceCustom (XamlElementInfo *info, const char *name, ElementType type, DependencyObject *dob) :
	XamlElementInstance (info, name, type)
{
	this->item = dob;
};


void
XamlElementInstanceCustom::SetProperty (XamlParserInfo *p, XamlElementInstance *property, XamlElementInstance *value)
{
	dependency_object_set_property (p, this, property, value);
}

void
XamlElementInstanceCustom::AddChild (XamlParserInfo *p, XamlElementInstance *child)
{
	dependency_object_add_child (p, this, child);
}

void
XamlElementInstanceCustom::SetAttributes (XamlParserInfo *p, const char **attr)
{
	dependency_object_set_attributes (p, this, attr);
}


///
/// Add Child funcs
///

void
dependency_object_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	if (parent->element_type == XamlElementInstance::PROPERTY) {
		char **prop_name = g_strsplit (parent->element_name, ".", -1);
		Type *owner = Type::Find (prop_name [0]);

		if (!owner) {
			g_strfreev (prop_name);
			return parser_error (p, parent->element_name, NULL, 2007,
					     g_strdup_printf ("Unknown element: %s.", parent->element_name));
		}

		DependencyProperty *dep = DependencyObject::GetDependencyProperty (owner->type, prop_name [1]);

		g_strfreev (prop_name);

		if (!dep)
			return parser_error (p, parent->element_name, NULL, 2007,
					     g_strdup_printf ("Unknown element: %s.", parent->element_name));

		// Don't add the child element, if it is the entire collection
		if (dep->value_type == child->info->GetKind ())
			return;

		Type *col_type = Type::Find (dep->value_type);
		if (!col_type->IsSubclassOf (Type::COLLECTION))
			return;

		// Most common case, we will have a parent that we can snag the collection from
		DependencyObject *obj = (DependencyObject *) parent->parent->item;
		if (!obj)
			return;

		Value *col_v = obj->GetValue (dep);
		Collection *col = NULL;

		if (!col_v) {
			col = (Collection *) col_type->CreateInstance ();
			obj->SetValue (dep, Value (col));
		} else {
			col = (Collection *) col_v->AsCollection ();
		}

		col->Add ((DependencyObject*)child->item);
		return;
	}

	if (Type::Find (parent->info->GetKind ())->IsSubclassOf (Type::COLLECTION)) {
		Collection *col = (Collection *) parent->item;

		col->Add ((DependencyObject *) child->item);
		return;
	}

	
	if (parent->info->GetContentProperty ()) {
		DependencyProperty *dep = DependencyObject::GetDependencyProperty (parent->info->GetKind (),
				(char *) parent->info->GetContentProperty ());

		if (!dep)
			return;

		Type *prop_type = Type::Find (dep->value_type);
		bool is_collection = prop_type->IsSubclassOf (Type::COLLECTION);

		if (!is_collection && prop_type->IsSubclassOf (child->info->GetKind ())) {
			DependencyObject *obj = (DependencyObject *) parent->item;
			obj->SetValue (dep, (DependencyObject *) child->item);
			return;

		}

		// We only want to enter this if statement if we are NOT dealing with the content property element,
		// otherwise, attempting to use explicit property setting, would add the content property element
		// to the content property element collection
		if (is_collection && dep->value_type != child->info->GetKind ()) {
			DependencyObject *obj = (DependencyObject *) parent->item;
			Value *col_v = obj->GetValue (dep);
			Collection *col;
			
			if (!col_v) {
				col = collection_new (dep->value_type);
				obj->SetValue (dep, Value (col));
				col->unref ();
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
		((Panel *) parent->item)->AddChild ((UIElement *) child->item);

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

	if (!dep) {
		// FIXME is this really where this check should live
		parser_error (p, item->element_name, NULL, 2030,
			      g_strdup_printf ("Property element %s cannot be used inside another property element.",
					       property->element_name));
		return;
	}

	prop = DependencyObject::GetDependencyProperty (item->info->GetKind (), prop_name [1]);

	if (prop) {
		if (prop->IsReadOnly ()) {
			parser_error (p, item->element_name, NULL, 2014,
				      g_strdup_printf ("The attribute %s is read only and cannot be set.", prop->name));
		} else if (Type::Find (value->info->GetKind ())->IsSubclassOf (prop->value_type)) {
			// an empty collection can be NULL and valid
			if (value->item) {
				if (item->IsPropertySet (prop->name)) {
					parser_error (p, item->element_name, NULL, 2033,
						g_strdup_printf ("Cannot specify the value multiple times for property: %s.",
								 property->element_name));
				} else {
					GError *seterror = NULL;
					if (!dep->SetValue (prop, Value (value->item), &seterror)) {
						parser_error (p, item->element_name, NULL, seterror->code, seterror->message);
						g_error_free (seterror);
					}
					item->MarkPropertyAsSet (prop->name);
				}
			}
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

bool
xaml_set_property_from_str (DependencyObject *obj, DependencyProperty *prop, const char *value)
{
	Value *v = NULL;
	
	if (!value_from_str (prop->value_type, prop->name, value, &v))
		return false;
	
	// it's possible for (a valid) value to be NULL (and we must keep the default value)
	if (v) {
		obj->SetValue (prop, v);
		delete v;
	}
	
	return true;
}

static bool
is_valid_event_name (const char *name)
{
	return (!strcmp (name, "ImageFailed") ||
		!strcmp (name, "MediaEnded") ||
		!strcmp (name, "MediaFailed") ||
		!strcmp (name, "MediaOpened") ||
		!strcmp (name, "BufferingProgressChanged") ||
		!strcmp (name, "CurrentStateChanged") ||
		!strcmp (name, "DownloadProgressChanged") ||
		!strcmp (name, "MarkerReached") ||
		!strcmp (name, "Completed") ||
		!strcmp (name, "DownloadFailed") ||
		!strcmp (name, "FullScreenChange") ||
		!strcmp (name, "Resize") ||
		!strcmp (name, "Error") ||
		!strcmp (name, "Completed") ||
		!strcmp (name, "ImageFailed") ||
		!strcmp (name, "GotFocus") ||
		!strcmp (name, "KeyDown") ||
		!strcmp (name, "KeyUp") ||
		!strcmp (name, "Loaded") ||
		!strcmp (name, "LostFocus") ||
		!strcmp (name, "MouseEnter") ||
		!strcmp (name, "MouseLeave") ||
		!strcmp (name, "MouseLeftButtonDown") ||
		!strcmp (name, "MouseLeftButtonUp") ||
		!strcmp (name, "MouseMove"));
}

bool
dependency_object_hookup_event (XamlParserInfo *p, XamlElementInstance *item, const char *name, const char *value)
{
	if (is_valid_event_name (name)) {
		if (!strncmp (value, "javascript:", strlen ("javascript:"))) {
			parser_error (p, item->element_name, name, 2024,
				      g_strdup_printf ("Invalid attribute value %s for property %s.",
						       value, name));
			return true;
		}

		if (!p->loader) {
			parser_error (p, item->element_name, name, -1,
				      g_strdup_printf ("No hookup event callback handler installed '%s' event will not be hooked up\n", name));
			return true;
		}

		if (p->loader)
			p->loader->HookupEvent (item->item, name, value);
		
		return false;
	}

	return true;
}


void
dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	int skip_attribute = -1;

start_parse:
	for (int i = 0; attr [i]; i += 2) {
		if (i == skip_attribute)
			continue;

		// Skip empty attrs
		if (attr [i + 1] == NULL || strlen (attr [i + 1]) == 0)
			continue;

		// Setting attributes like x:Class can change item->item, so we
		// need to make sure we have an up to date pointer
		DependencyObject *dep = (DependencyObject *) item->item;
		char **attr_name = g_strsplit (attr [i], "|", -1);

		if (attr_name [1]) {
			XamlNamespace *ns = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, attr_name [0]);

			if (!ns)
				return parser_error (p, item->element_name, attr [i], 5055,
						g_strdup ("undeclared prefix"));

			bool reparse = false;
			ns->SetAttribute (p, item, attr_name [1], attr [i + 1], &reparse);

			g_strfreev (attr_name);

			// Setting custom attributes can cause errors galore
			if (p->error_args)
				return;

			if (reparse) {
				skip_attribute = i;
				i = 0;
				item->ClearSetProperties ();
				goto start_parse;
			}
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
		if (atchname ) {
			Type *attached_type = Type::Find (atchname);
			prop = DependencyObject::GetDependencyProperty (attached_type->type, pname);
		} else {
			prop = DependencyObject::GetDependencyProperty (item->info->GetKind (), pname);
		}

		if (prop) {
			if (prop->IsReadOnly ()) {
				parser_error (p, item->element_name, NULL, 2014,
					      g_strdup_printf ("The attribute %s is read only and cannot be set.", prop->name));
				if (atchname)
					g_free (atchname);
				return;
			} 

			if (item->IsPropertySet (prop->name)) {
				parser_error (p, item->element_name, attr [i], 2033,
					      g_strdup_printf ("Cannot specify the value multiple times for property: %s.", prop->name));
				if (atchname)
					g_free (atchname);
				return;
			}

			Value *v = NULL;
			if (!value_from_str (prop->value_type, prop->name, attr [i + 1], &v)) {
				parser_error (p, item->element_name, attr [i], 2024,
					      g_strdup_printf ("Invalid attribute value %s for property %s.",
							       attr [i + 1], attr [i]));
				if (atchname)
					g_free (atchname);
				return;
			}

			if (v) {
				GError *seterror = NULL;
				if (!dep->SetValue (prop, v, &seterror)) {
					parser_error (p, item->element_name, attr [i], seterror->code, seterror->message);
					g_error_free (seterror);
				} else
					item->MarkPropertyAsSet (prop->name);

				delete v;
				
			} else {
				if (prop->IsNullable ())
					dep->SetValue (prop, NULL);
				else {
					parser_error (p, item->element_name, attr [i], 2017, g_strdup_printf ("Null is not a legal value for attribute %s.", attr [i]));
				}
			}
		} else {
			// This might be a property of a managed object
			if (p->loader && p->loader->SetAttribute (item->item, attr [i], attr [i + 1])) {
				if (atchname)
					g_free (atchname);
				continue;
			}

			if (dependency_object_hookup_event (p, item, pname, attr [i + 1]))
				parser_error (p, item->element_name, attr [i], 2012,
					      g_strdup_printf ("Unknown attribute %s on element %s.",
							       attr [i], item->element_name));
		}

		if (atchname)
			g_free (atchname);
	}
}


static Type*
get_type_for_property_name (const char* prop)
{
	Type *t  = NULL;
	DependencyObject *o = NULL;
	DependencyProperty *p = NULL;

	char **prop_name = g_strsplit (prop, ".", -1);
	if (!prop_name [0] || !prop_name [1])
		return NULL;

	t = Type::Find (prop_name [0]);
	if (!t) {

		g_strfreev (prop_name);
		return NULL;
	}

	o = t->CreateInstance ();
	if (!o) {

		g_strfreev (prop_name);
		return NULL;
	}

	p = o->GetDependencyProperty (prop_name [1]);
	if (!p) {

		g_strfreev (prop_name);
		return NULL;
	}

	return Type::Find (p->value_type);
}

void
xaml_init (void)
{
	GHashTable *dem = g_hash_table_new (g_str_hash, g_str_equal); // default element map
	GHashTable *deploy = g_hash_table_new (g_str_hash, g_str_equal); // deployment element map
	GHashTable *x_dem = g_hash_table_new (g_str_hash, g_str_equal); // x element map

	/*
	rdoe (deploy, "Deployment", NULL, Type::DEPLOYMENT, (create_item_func) deployment_new);
	rdoe (deploy, "AssemblyPart", NULL, Type::ASSEMBLYPART, (create_item_func) assembly_part_new);
	rdoe (deploy, "AssemblyPartCollection", col, Type::ASSEMBLYPART_COLLECTION, (create_item_func) assembly_part_collection_new);
	

	// Is this correct, since there is no SupportedCulture item
	rdoe (deploy, "SupportedCultureCollection", col, Type::SUPPORTEDCULTURE_COLLECTION, (create_item_func) supported_culture_collection_new);
	*/

	default_namespace = new DefaultNamespace (dem);
	deploy_namespace = new DefaultNamespace (deploy);
	x_namespace = new XNamespace (x_dem);
}
