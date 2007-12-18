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
#include <ctype.h>

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

#define READ_BUFFER 1024

static GHashTable *enum_map = NULL;

class XamlElementInfo;
class XamlElementInstance;
class XamlParserInfo;
class XamlNamespace;
class DefaultNamespace;
class XNamespace;


static DefaultNamespace *default_namespace = NULL;
static XNamespace *x_namespace = NULL;

typedef DependencyObject *(*create_item_func) (void);
typedef XamlElementInstance *(*create_element_instance_func) (XamlParserInfo *p, XamlElementInfo *i);
typedef void  (*add_child_func) (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child);
typedef void  (*set_property_func) (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);
typedef void  (*set_attributes_func) (XamlParserInfo *p, XamlElementInstance *item, const char **attr);

void dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr);
void parser_error (XamlParserInfo *p, const char *el, const char *attr, int error_code, const char *message);

XamlElementInstance *create_custom_element (XamlParserInfo *p, XamlElementInfo *i);
void  custom_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr);
void  custom_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child);
void  custom_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value);

XamlElementInstance *create_x_code_directive_element (XamlParserInfo *p, XamlElementInfo *i);
void process_x_code_directive (XamlParserInfo *p, const char **attr);

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
		X_CODE_DIRECTIVE,
		INVALID
	};

	int element_type;
	DependencyObject *item;

	GHashTable *set_properties;

	XamlElementInstance (XamlElementInfo *info) : element_name (NULL), instance_name (NULL),
						      info (info), parent (NULL), element_type (INVALID),
						      item (NULL), set_properties (NULL)
	{
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

	bool IsPropertySet (const char *name)
	{
		if (!set_properties)
			return false;

		return g_hash_table_lookup (set_properties, name) != NULL;
	}

	void MarkPropertyAsSet (char *name)
	{
		if (!set_properties)
			set_properties = g_hash_table_new (g_str_hash, g_str_equal);

		g_hash_table_insert (set_properties, name, GINT_TO_POINTER (TRUE));
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
	GString *cdata;
	bool has_cdata;

	bool implicit_default_namespace;

	ParserErrorEventArgs *error_args;

	XamlLoader* loader;
	GList* created_elements;

	void AddCreatedElement (DependencyObject* element)
	{
		created_elements = g_list_prepend (created_elements, element);
	}

	XamlParserInfo (XML_Parser parser, const char *file_name) :
	  
		parser (parser), file_name (file_name), namescope (new NameScope()), top_element (NULL),
		current_namespace (NULL), current_element (NULL),
		cdata (NULL), has_cdata(false), implicit_default_namespace (false), error_args (NULL),
		loader (NULL), created_elements (NULL)
	{
		namespace_map = g_hash_table_new (g_str_hash, g_str_equal);
	}

	~XamlParserInfo ()
	{
		created_elements = g_list_reverse (created_elements);
		g_list_foreach (created_elements, unref_xaml_element, NULL);
		g_list_free (created_elements);
		
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

        const char * GetContentProperty ()
	{
		XamlElementInfo *walk = this;

		while (walk) {
			if (walk->content_property)
				return walk->content_property;

			walk = walk->parent;
		}

		return NULL;
	}
	
 protected:
	XamlElementInfo () { }

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
		return (XamlElementInfo *) g_hash_table_lookup (element_map, el);
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
		return (XamlElementInfo *) g_hash_table_lookup (element_map, el);
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

		CustomElementInfo *info = new CustomElementInfo (g_strdup (el), NULL, dob->GetObjectType ());

		info->create_element = create_custom_element;
		info->set_attributes = custom_set_attributes;
		info->add_child = custom_add_child;
		info->set_property = custom_set_property;
		info->dependency_object = dob;

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
	
	if (callbacks.load_managed_object) {
		return callbacks.load_managed_object (asm_name, asm_path, name, type_name);
	}
		
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
	if (callbacks.hookup_event) {
		return callbacks.hookup_event (target, name, value);
	}
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

bool
XamlLoader::LoadCode (const char *source, const char *type)
{
	if (!vm_loaded)
		LoadVM ();

	if (callbacks.load_code)
		return callbacks.load_code (source, type);
	return false;
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

	if (!surface) {
		printf ("XamlLoader::XamlLoader ('%s', '%s', %p): Initializing XamlLoader without a surface.\n", filename, str, surface);
	}
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
		printf ("Trying to add missing file '%s' to a null loader.\n", file);
		return;
	}
	
	loader->AddMissing (file);
}

void 
xaml_loader_set_callbacks (XamlLoader* loader, XamlLoaderCallbacks callbacks)
{
	if (!loader) {
		printf ("Trying to set callbacks for a null object\n");
		return;
	}
	
	loader->callbacks = callbacks;
	loader->vm_loaded = true;
}

XamlElementInstance *
create_custom_element (XamlParserInfo *p, XamlElementInfo *i)
{
	CustomElementInfo *c = (CustomElementInfo *) i;
	XamlElementInstance *inst = new XamlElementInstance (i);

	inst->element_name = i->name;
	inst->element_type = XamlElementInstance::ELEMENT;
	inst->item = c->dependency_object;

	if (p->loader)
		inst->item->SetSurface (p->loader->GetSurface ());
	p->AddCreatedElement (inst->item);

	return inst;
}

void
custom_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	dependency_object_set_attributes (p, item, attr);
}

void
custom_add_child (XamlParserInfo *p, XamlElementInstance *parent, XamlElementInstance *child)
{
	// XXX do we need to do anything here?


	// if it's not handled, we definitely need to at least check if the
	// object is a panel subclass, and if so add it as we would a normal
	// child.
	if (parent->item->Is (Type::PANEL) && child->item->Is (Type::UIELEMENT)) {
		panel_child_add ((Panel *) parent->item, (UIElement *) child->item);
	}
}

void
custom_set_property (XamlParserInfo *p, XamlElementInstance *item, XamlElementInstance *property, XamlElementInstance *value)
{

}

XamlElementInstance *
create_x_code_directive_element (XamlParserInfo *p, XamlElementInfo *i)
{
	XamlElementInstance *inst = new XamlElementInstance (i);
	inst->element_type = XamlElementInstance::X_CODE_DIRECTIVE;

	return inst;
}

void
process_x_code_directive (XamlParserInfo *p, const char **attr)
{
	const char *source = NULL;
	const char *type = NULL;
	int i;
	bool res;

	i = 0;
	while (attr [i]) {
		if (!strcmp (attr [i], "Source"))
			source = attr [i + 1];
		else if (!strcmp (attr [i], "Type"))
			type = attr [i + 1];
		i += 2;
	}

	if (p->loader) {
		res = p->loader->LoadCode (source, type);

		if (!res)
			parser_error (p, "", "", -1,
						  g_strdup_printf ("Unable to load '%s'\n", source));
	}
}

static bool
is_instance_of (XamlElementInstance *item, Type::Kind kind)
{
	for (XamlElementInfo *walk = item->info; walk; walk = walk->parent) {
		if (walk->dependency_type == kind)
			return true;
	}

	return false;
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

	p->error_args = new ParserErrorEventArgs (message,
						  p->file_name,
						  XML_GetCurrentLineNumber (p->parser),
						  XML_GetCurrentColumnNumber (p->parser),
						  error_code,
						  el, attr);

	g_warning ("PARSER ERROR, STOPPING PARSING:  (%d) %s  line: %d   char: %d\n", error_code, message,
			p->error_args->line_number, p->error_args->char_position);

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

	printf ("expat error is:  %d\n", expat_error);

	switch (expat_error) {
	case XML_ERROR_DUPLICATE_ATTRIBUTE:
		error_code = 5031;
		message = g_strdup ("wfc: unique attribute spec");
		break;
	case XML_ERROR_UNBOUND_PREFIX:
		error_code = 5055;
		message = g_strdup ("undeclared prefix");
		break;
	default:
		error_code = expat_error;
		message = g_strdup_printf ("Unhandled XML error %s", XML_ErrorString (expat_error));
		break;
	}

	parser_error (p, NULL, NULL, error_code, message);
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
		inst = elem->create_element (p, elem);

		if (!inst)
			return;

		if (inst->element_type == XamlElementInstance::X_CODE_DIRECTIVE) {
			process_x_code_directive (p, attr);
			inst->parent = p->current_element;
			p->current_element = inst;
			return;
		}

		if (!inst->item)
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

		elem->set_attributes (p, inst, attr);

		// Setting the attributes can kill the item
		if (!inst->item)
			return;

		if (p->current_element && p->current_element->info)
			p->current_element->info->add_child (p, p->current_element, inst);

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

			if (p->current_element)
				inst->info = p->current_element->info;
			else
				inst->info = NULL;
			inst->element_name = g_strdup (el);
			inst->element_type = XamlElementInstance::PROPERTY;
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
flush_char_data (XamlParserInfo *p, bool start)
{
	DependencyProperty *content;
	const char *prop_name = NULL;
	Type::Kind prop_type;
	
	if (!p->has_cdata || !p->current_element)
		return;

	if (p->current_element->info)
		prop_name = p->current_element->info->content_property;
	
	if (!prop_name && p->cdata) {
		char *err = g_strdup_printf ("%s does not support text content.", p->current_element->element_name);
		parser_error (p, NULL, NULL, 2011, err);
		goto done;
	} else if (!prop_name) {
		goto done;
	}
	
	prop_type = p->current_element->info->dependency_type;
	content = DependencyObject::GetDependencyProperty (prop_type, prop_name);
	
	// TODO: There might be other types that can be specified here,
	// but string is all i have found so far.  If you can specify other
	// types, i should pull the property setting out of set_attributes
	// and use that code
	
	if ((content->value_type) == Type::STRING && p->cdata) {
		g_strchomp (p->cdata->str);
		p->current_element->item->SetValue (content, Value (p->cdata->str));
	} else if (p->current_element->item && is_instance_of (p->current_element, Type::TEXTBLOCK)) {
		Inlines *inlines = text_block_get_inlines ((TextBlock *) p->current_element->item);
		
		if (!p->cdata && inlines && !inlines->list->IsEmpty () && start) {
			// LWSP between <Run> elements is to be treated as a single-SPACE <Run> element
			p->cdata = g_string_new (" ");
		} else if (p->cdata) {
			// This is the normal case
			g_strchomp (p->cdata->str);
		} else {
			// This is either LWSP before the first <Run> element or after the last <Run> element
			goto done;
		}
		
		Run *run = new Run ();
		run_set_text (run, p->cdata->str);
		
		if (!inlines) {
			inlines = new Inlines ();
			text_block_set_inlines ((TextBlock *) p->current_element->item, inlines);
			inlines->unref ();
		}
		
		inlines->Add (run);
		run->unref ();
	}
	
done:
	
	if (p->cdata) {
		g_string_free (p->cdata, FALSE);
		p->cdata = NULL;
	}
	
	p->has_cdata = false;
}

static void
start_element_handler (void *data, const char *el, const char **attr)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (p->error_args)
		return;

	char **name = g_strsplit (el, "|",  -1);
	char *element = NULL;

	flush_char_data (p, true);

	p->current_namespace = NULL;
	if (g_strv_length (name) == 2) {
		// Find the proper namespace
		p->current_namespace = (XamlNamespace *) g_hash_table_lookup (p->namespace_map, name [0]);
		element = name [1];
	}

	if (!p->current_namespace && p->implicit_default_namespace) {
		p->current_namespace = default_namespace;
		element = name [0];
	}

	if (!p->current_namespace) {
		if (name [1])
			parser_error (p, name [1], NULL, -1, g_strdup_printf ("No handlers available for namespace: '%s' (%s)\n", name [0], el));
		else
			parser_error (p, el, NULL, -1, g_strdup_printf ("No namespace mapping available for element: '%s'\n", el));
		
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
		flush_char_data (info, false);
		break;
	case XamlElementInstance::PROPERTY: {
		List::Node *walk = info->current_element->children->First ();
		while (walk) {
			XamlElementInstance *child = (XamlElementInstance *) walk;
			if (info->current_element->parent)
				info->current_element->parent->info->set_property (info, info->current_element->parent,	info->current_element, child);
			walk = walk->next;
		}
		break;
	}
	case XamlElementInstance::X_CODE_DIRECTIVE:
		/* Nothing to do */
		break;
	}

	info->current_element = info->current_element->parent;
}

static void
char_data_handler (void *data, const char *in, int inlen)
{
	XamlParserInfo *p = (XamlParserInfo *) data;
	const char *inend = in + inlen;
	const char *inptr = in;
	bool lwsp = false;
	size_t len = 0;
	char *s, *d;
	
	if (p->error_args)
		return;
	
	if (!p->cdata) {
		p->has_cdata = true;
		
		// unless we already have significant char data, ignore lwsp
		while (inptr < inend && g_ascii_isspace (*inptr))
			inptr++;
		
		if (inptr == inend)
			return;
		
		p->cdata = g_string_new_len (inptr, inend - inptr);
	} else {
		len = p->cdata->len;
		g_string_append_len (p->cdata, in, inlen);
		lwsp = g_ascii_isspace (p->cdata->str[len - 1]);
	}
	
	// Condense multi-lwsp blocks into a single space (and make all lwsp chars a literal space, not '\n', etc)
	s = d = p->cdata->str + len;
	while (*s != '\0') {
		if (g_ascii_isspace (*s)) {
			if (!lwsp)
				*d++ = ' ';
			lwsp = true;
			s++;
		} else {
			lwsp = false;
			*d++ = *s++;
		}
	}
	
	g_string_truncate (p->cdata, d - p->cdata->str);
}

static void
start_namespace_handler (void *data, const char *prefix, const char *uri)
{
	XamlParserInfo *p = (XamlParserInfo *) data;

	if (p->error_args)
		return;

	if (!strcmp ("http://schemas.microsoft.com/winfx/2006/xaml/presentation", uri) ||
			!strcmp ("http://schemas.microsoft.com/client/2007", uri)) {

		if (prefix)
			return parser_error (p, (p->current_element ? p->current_element->element_name : NULL), prefix, -1,
					g_strdup_printf  ("It is illegal to add a prefix (xmlns:%s) to the default namespace.\n", prefix));

		g_hash_table_insert (p->namespace_map, g_strdup (uri), default_namespace);
	} else if (!strcmp ("http://schemas.microsoft.com/winfx/2006/xaml", uri)) {

		g_hash_table_insert (p->namespace_map, g_strdup (uri), x_namespace);
	} else {
		if (!p->loader) {
			return parser_error (p, (p->current_element ? p->current_element->element_name : NULL), prefix, -1,
					g_strdup_printf ("No custom element callback installed to handle %s", uri));
		}

		if (!prefix) {
			parser_error (p, (p->current_element ? p->current_element->element_name : NULL), NULL, 2262, g_strdup ("AG_E_PARSER_NAMESPACE_NOT_SUPPORTED"));
			return;
		}


		CustomNamespace *c = new CustomNamespace (g_strdup (uri));
		g_hash_table_insert (p->namespace_map, c->xmlns, c);
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
	g_hash_table_insert (p->namespace_map, (char *) "http://schemas.microsoft.com/winfx/2006/xaml/presentation", default_namespace);
	g_hash_table_insert (p->namespace_map, (char *) "http://schemas.microsoft.com/winfx/2006/xaml", x_namespace);
}

static void
print_tree (XamlElementInstance *el, int depth)
{
	for (int i = 0; i < depth; i++)
		printf ("\t");
	
	Value *v = NULL;

	if (el->element_type == XamlElementInstance::ELEMENT)
		v = el->item->GetValue (DependencyObject::NameProperty);
	printf ("%s  (%s)\n", el->element_name, v ? v->AsString () : "-no name-");
	
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

	done = 0;
	while (!done) {
		len = fread (buffer, 1, READ_BUFFER, fp);
		done = feof (fp);
		if (!XML_Parse (p, buffer, len, done)) {
			expat_parser_error (parser_info, XML_GetErrorCode (p));
			goto cleanup_and_return;
		}
	}

#ifdef DEBUG_XAML
	print_tree (parser_info->top_element, 0);
#endif

	if (parser_info->top_element) {
	
		res = parser_info->top_element->item;
		if (element_type)
			*element_type = parser_info->top_element->info->dependency_type;

		if (parser_info->error_args) {
			*element_type = Type::INVALID;
			goto cleanup_and_return;
		}
		
		res->ref ();
	}

 cleanup_and_return:
	if (parser_info->error_args) {
		printf ("setting error args\n");
		loader->error_args = parser_info->error_args;
	}

	if (fp)
		fclose (fp);
	if (p)
		XML_ParserFree (p);
	if (parser_info)
		delete parser_info;

	return res;
}

DependencyObject *
xaml_create_from_str (XamlLoader* loader, const char *xaml, bool create_namescope,
		      Type::Kind *element_type)
{
	XML_Parser p = XML_ParserCreateNS ("utf-8", '|');
	XamlParserInfo *parser_info = NULL;
	DependencyObject *res = NULL;
	char *start = (char*)xaml;

	if (!p) {
#ifdef DEBUG_XAML
		printf ("can not create parser\n");
#endif
		goto cleanup_and_return;
	}

	parser_info = new XamlParserInfo (p, NULL);

	parser_info->namescope->SetTemporary (!create_namescope);

	parser_info->loader = loader;
	
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
	for (; *start && isspace (*start); start++);

	if (!XML_Parse (p, start, strlen (start), TRUE)) {
		expat_parser_error (parser_info, XML_GetErrorCode (p));
		printf ("error parsing:  %s\n\n", xaml);
		goto cleanup_and_return;
	}

#ifdef DEBUG_XAML
	print_tree (parser_info->top_element, 0);
#endif

	if (parser_info->top_element) {
		res = parser_info->top_element->item;
		if (element_type)
			*element_type = parser_info->top_element->info->dependency_type;

		if (parser_info->error_args) {
			res = NULL;
			*element_type = Type::INVALID;
			goto cleanup_and_return;
		}
		
		res->ref ();
	}

 cleanup_and_return:
	
	if (parser_info->error_args) {
		printf ("2. setting error args\n");
		loader->error_args = parser_info->error_args;
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
	//int s; // FOr starting expression markers
	Point cp = Point (0, 0);
	Point cp1, cp2, cp3;
	char *end;

	PathFigure *pf = NULL;
	PathSegment *prev = NULL;
	PathSegmentCollection *psc = NULL;
	
	PathGeometry *pg = new PathGeometry ();
	PathFigureCollection *pfc = new PathFigureCollection ();
	pg->SetValue (PathGeometry::FiguresProperty, pfc);
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
				geometry_set_fill_rule (pg, FillRuleEvenOdd);
			else if (*inptr == '1')
				geometry_set_fill_rule (pg, FillRuleNonzero);
			// FIXME: else it's a bad value and nothing should be rendered
			inptr = g_utf8_next_char (inptr);
			break;

		case 'm':
			relative = true;
		case 'M':
			if (!get_point (&cp1, &inptr))
				break;
			
			if (relative) make_relative (&cp, &cp1);

			if (pf) {
				pfc->Add (pf);
				pf->unref ();
			}

			pf = new PathFigure ();
			psc = new PathSegmentCollection ();
			pf->SetValue (PathFigure::SegmentsProperty, psc);
			psc->unref ();

			pf->SetValue (PathFigure::StartPointProperty, Value (cp1));

			prev = NULL;

			cp.x = cp1.x;
			cp.y = cp1.y;

			advance (&inptr);
			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;
				
				if (relative) make_relative (&cp, &cp1);
				
				LineSegment* ls = new LineSegment ();
				ls->SetValue (LineSegment::PointProperty, Value (cp1));

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
			while (more_points_available (&inptr)) {
				if (!get_point (&cp1, &inptr))
					break;
				
				if (relative) make_relative (&cp, &cp1);

				LineSegment* ls = new LineSegment ();
				ls->SetValue (LineSegment::PointProperty, Value (cp1));

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
			double x = g_ascii_strtod (inptr, &end);
			if (end == inptr)
				break;
			
			inptr = end;
			
			if (relative)
				x += cp.x;
			cp = Point (x, cp.y);

			LineSegment* ls = new LineSegment ();
			ls->SetValue (LineSegment::PointProperty, Value (cp));

			psc->Add (ls);
			ls->unref ();
			prev = ls;

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

			LineSegment* ls = new LineSegment ();
			ls->SetValue (LineSegment::PointProperty, Value (cp));

			psc->Add (ls);
			ls->unref ();
			prev = ls;

			break;
		}
		
		case 'c':
			relative = true;
		case 'C':
		{
			if (!get_point (&cp1, &inptr))
				break;
			
			if (relative) make_relative (&cp, &cp1);
			
			advance (&inptr);
			
			if (!get_point (&cp2, &inptr))
				break;
			
			if (relative) make_relative (&cp, &cp2);
			
			advance (&inptr);
			
			if (!get_point (&cp3, &inptr))
				break;
			
			if (relative) make_relative (&cp, &cp3);
			
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
				pbs->SetValue (PolyBezierSegment::PointsProperty, Value (pts, count));

				psc->Add (pbs);
				pbs->unref ();
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
			while (more_points_available (&inptr)) {
				if (!get_point (&cp2, &inptr))
					break;
				
				if (relative) make_relative (&cp, &cp2);

				advance (&inptr);
				
				if (!get_point (&cp3, &inptr))
					break;
				
				if (relative) make_relative (&cp, &cp3);

				if (prev && prev->GetObjectType () == Type::BEZIERSEGMENT) {
					Point *p = prev->GetValue (BezierSegment::Point2Property)->AsPoint ();
					cp1.x = 2 * cp.x - p->x;
					cp1.y = 2 * cp.y - p->y;
				} else
					cp1 = cp;

				BezierSegment *bs = new BezierSegment ();
				bs->SetValue (BezierSegment::Point1Property, Value (cp1));
				bs->SetValue (BezierSegment::Point2Property, Value (cp2));
				bs->SetValue (BezierSegment::Point3Property, Value (cp3));

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
			if (!get_point (&cp1, &inptr))
				break;
			
			if (relative) make_relative (&cp, &cp1);

			advance (&inptr);
			
			if (!get_point (&cp2, &inptr))
				break;
			
			if (relative) make_relative (&cp, &cp2);
			
			advance (&inptr);
			if (more_points_available (&inptr)) {
				GSList *pl = NULL;
				int count = 2;

				pl = g_slist_append (pl, &cp1);
				pl = g_slist_append (pl, &cp2);

				Point last;
				Point *pts = get_point_array (inptr, pl, &count, relative, &cp, &last);
				PolyQuadraticBezierSegment *pqbs = new PolyQuadraticBezierSegment ();
				pqbs->SetValue (PolyQuadraticBezierSegment::PointsProperty, Value (pts, count));

				psc->Add (pqbs);
				pqbs->unref ();
				prev = pqbs;

				cp.x = last.x;
				cp.y = last.y;

				g_slist_free (pl);
			} else {
				QuadraticBezierSegment *qbs = new QuadraticBezierSegment ();
				qbs->SetValue (QuadraticBezierSegment::Point1Property, Value (cp1));
				qbs->SetValue (QuadraticBezierSegment::Point2Property, Value (cp2));

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
			while (more_points_available (&inptr)) {
				if (!get_point (&cp2, &inptr))
					break;
				
				if (relative) make_relative (&cp, &cp2);

				if (prev && prev->GetObjectType () == Type::QUADRATICBEZIERSEGMENT) {
					Point *p = prev->GetValue (QuadraticBezierSegment::Point1Property)->AsPoint ();
					cp1.x = 2 * cp.x - p->x;
					cp1.y = 2 * cp.y - p->y;
				} else
					cp1 = cp;

				QuadraticBezierSegment *qbs = new QuadraticBezierSegment ();
				qbs->SetValue (QuadraticBezierSegment::Point1Property, Value (cp1));
				qbs->SetValue (QuadraticBezierSegment::Point2Property, Value (cp2));

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
				
				if (relative) make_relative (&cp, &cp2);

				ArcSegment *arc = new ArcSegment ();
				arc->SetValue (ArcSegment::SizeProperty, cp1);
				arc->SetValue (ArcSegment::RotationAngleProperty, angle);
				arc->SetValue (ArcSegment::IsLargeArcProperty, (bool) is_large);
				arc->SetValue (ArcSegment::SweepDirectionProperty, sweep);
				arc->SetValue (ArcSegment::PointProperty, cp2);

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
			Point *p = path_figure_get_start_point (pf);

			if (!p)
				p = new Point (0, 0);

			LineSegment* ls = new LineSegment ();
			ls->SetValue (LineSegment::PointProperty, Value (*p));

			psc->Add (ls);
			ls->unref ();

			prev = NULL;
			path_figure_set_is_closed (pf, true);

			cp.x = p->x;
			cp.y = p->y;
			
			if (pf) {
				pfc->Add (pf);
				pf->unref ();
			}

			pf = new PathFigure ();
			psc = new PathSegmentCollection ();
			pf->SetValue (PathFigure::SegmentsProperty, psc);
			psc->unref ();

			path_figure_set_start_point (pf, p);

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
	{ "UltraLight", 200 },  /* deprecated as of July 2007 */
	{ "Light",      300 },
	{ "Normal",     400 },
	{ "Regular",    400 },  /* deprecated as of July 2007 */
	{ "Medium",     500 },
	{ "SemiBold",   600 },
	{ "DemiBold",   600 },  /* deprecated as of July 2007 */
	{ "Bold",       700 },
	{ "ExtraBold",  800 },
	{ "UltraBold",  800 },  /* deprecated as of July 2007 */
 	{ "Black",      900 },
	{ "Heavy",      900 },  /* deprecated as of July 2007 */
	{ "ExtraBlack", 950 },
	{ "UltraBlack", 950 },  /* deprecated as of July 2007 */
	{ NULL, 0 },
};

enum_map_t style_simulations_map [] = {
	{ "BoldItalicSimulation", 0 },
	{ "BoldSimulation",       1 },
	{ "ItalicSimulation",     2 },
	{ "None",                 3 },
	{ NULL,                   0 },
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
	{ NULL, 0 },
};

int
enum_from_str (const enum_map_t *emu, const char *str)
{
	for (int i = 0; emu [i].name; i++) {
		if (!g_strcasecmp (emu [i].name, str))
			return emu [i].value;
	}

	return (int) strtol (str, NULL, 10);
}

Value *
value_from_str_with_typename (const char *type_name, const char *prop_name, const char *str)
{
	Type *t = Type::Find (type_name);
	if (!t)
		return NULL;

	return value_from_str (t->type, prop_name, str);
}

		
// Will return NULL if there are any errors
Value *
value_from_str (Type::Kind type, const char *prop_name, const char *str)
{
	Value *v;
	char *endptr;

	switch (type) {
	case Type::BOOL: {
		bool b;
		if (!g_strcasecmp ("true", str))
			b = true;
		else if (!g_strcasecmp ("false", str))
			b = false;
		else
			return NULL;

		v = new Value (b);
		break;
	}
	case Type::DOUBLE: {
		double d;

		errno = 0;
		d = g_ascii_strtod (str, &endptr);

		if (errno || endptr == str || *endptr)
			return NULL;

		v = new Value (d);
		break;
	}
	case Type::INT64: {
		gint64 l;

		errno = 0;
		l = strtol (str, &endptr, 10);

		if (errno || endptr == str || *endptr)
			return NULL;

		v = new Value (l, Type::INT64);
		break;
	}
	case Type::TIMESPAN: {
		TimeSpan ts;

		if (!time_span_from_str (str, &ts))
			return NULL;

		v = new Value (ts, Type::TIMESPAN);
		break;
	}
	case Type::INT32: {
		int i;

		if (isalpha (str [0]) && prop_name) {
			enum_map_t *emu = (enum_map_t *) g_hash_table_lookup (enum_map, prop_name);

			if (emu)
				i = enum_from_str (emu, str);
			else
				return NULL;
		} else {
			errno = 0;
			long l = strtol (str, &endptr, 10);

			if (errno || endptr == str || *endptr)
				return NULL;

			i = (int) l;
		}

		v = new Value (i);
		break;
	}
	case Type::STRING: {
		//v = new Value (str);
		return new Value (str);
		break;
	}
	case Type::COLOR: {
		Color *c = color_from_str (str);
		v = new Value (*c);
		delete c;
		break;
	}
	case Type::REPEATBEHAVIOR: {
		RepeatBehavior rb = RepeatBehavior::Forever;

		if (!repeat_behavior_from_str (str, &rb))
			return NULL;

		v = new Value (rb);
		break;
	}
	case Type::DURATION: {
		Duration d = Duration::Forever;

		if (!duration_from_str (str, &d))
			return NULL;

		v = new Value (d);
		break;
	}
	case Type::KEYTIME: {
		KeyTime kt = KeyTime::Paced;

		if (!keytime_from_str (str, &kt))
			return NULL;

		v = new Value (kt);
		break;
	}
	case Type::KEYSPLINE: {
		KeySpline *ks;

		if (!key_spline_from_str (str, &ks))
			return NULL;

		v = Value::CreateUnrefPtr (ks);
		break;
	}
	case Type::BRUSH:
	case Type::SOLIDCOLORBRUSH: {
		// Only solid color brushes can be specified using attribute syntax
		SolidColorBrush *scb = solid_color_brush_new ();
		Color *c = color_from_str (str);
		solid_color_brush_set_color (scb, c); // copies c
		delete c;
		v = new Value (scb);
		scb->unref ();
		break;
	}
	case Type::POINT: {
		Point p;

		if (!point_from_str (str, &p))
			return NULL;

		v = new Value (p);
		break;
	}
	case Type::RECT: {
		Rect rect;

		if (!rect_from_str (str, &rect))
			return NULL;

		v = new Value (rect);
		break;
	}
	case Type::DOUBLE_ARRAY: {
		int count = 0;
		double *doubles = double_array_from_str (str, &count);

		if (!doubles)
			return NULL;

		v = new Value (doubles, count);
		break;
	}
	case Type::POINT_ARRAY:	{
		int count = 0;
		Point *points = point_array_from_str (str, &count);

		v = new Value (points, count);
		break;
	}
	case Type::TRANSFORM: {
		Matrix *mv = matrix_from_str (str);

		if (!mv)
			return NULL;

		MatrixTransform *t = new MatrixTransform ();
		t->SetValue (MatrixTransform::MatrixProperty, Value (mv));

		v = new Value (t);
		t->unref ();
		break;
	}
	case Type::MATRIX: {
		Matrix *matrix = matrix_from_str (str);
		if (!matrix)
			return NULL;

		v = new Value (matrix);
		matrix->unref ();
		break;
	}
	case Type::GEOMETRY: {
		Geometry *geometry = geometry_from_str (str);

		if (!geometry)
			return NULL;

		v = new Value (geometry);
		geometry->unref ();
		break;
	}
	default:
		return NULL;
	}

	return v;
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
		} else if (walk && walk->info->GetContentProperty ()) {
			dep = DependencyObject::GetDependencyProperty (walk->info->dependency_type,
					(char *) walk->info->GetContentProperty ());			
		}

		if (dep && dep->value_type == i->dependency_type) {
			Value *v = ((DependencyObject * ) walk->item)->GetValue (dep);
			if (v)
				inst->item = v->AsCollection ();
		}
	}

	if (!inst->item) {
		inst->item = i->create_item ();
		if (p->loader)
			inst->item->SetSurface (p->loader->GetSurface ());
		p->AddCreatedElement (inst->item);
	}

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
		Type *owner = Type::Find (prop_name [0]);
		DependencyProperty *dep = DependencyObject::GetDependencyProperty (owner->type, prop_name [1]);

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

	
	if (parent->info->GetContentProperty ()) {
		DependencyProperty *dep = DependencyObject::GetDependencyProperty (parent->info->dependency_type,
				(char *) parent->info->GetContentProperty ());

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
			if (value->item) {
				if (item->IsPropertySet (prop->name)) {
					parser_error (p, item->element_name, NULL, 2033,
						g_strdup_printf ("Cannot specify the value multiple times for property: %s.", prop->name));
				} else {
					dep->SetValue (prop, Value (value->item));
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

///
/// set attributes funcs
///

bool
xaml_set_property_from_str (DependencyObject *obj, DependencyProperty *prop, const char *value)
{
	Value *v = value_from_str (prop->value_type, prop->name, value);

	if (!v)
		return false;

	obj->SetValue (prop, v);
	delete v;
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
		if (!p->loader) {
			parser_error (p, item->element_name, name, -1,
					g_strdup_printf ("No hookup event callback handler installed '%s' event will not be hooked up\n", name));
			return true;
		}

		if (p->loader)
			p->loader->HookupEvent (item->item, name, value);
	}

	return false;
}


void
dependency_object_set_attributes (XamlParserInfo *p, XamlElementInstance *item, const char **attr)
{
	int skip_attribute = -1;

start_parse:
	for (int i = 0; attr [i]; i += 2) {

		if (i == skip_attribute)
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

			if (item->IsPropertySet (prop->name)) {
				parser_error (p, item->element_name, attr [i], 2033,
						g_strdup_printf ("Cannot specify the value multiple times for property: %s.", prop->name));
				return;
			}

			Value *v = value_from_str (prop->value_type, prop->name, attr [i + 1]);

			if (!v) {
				parser_error (p, item->element_name, attr [i], 2024, g_strdup_printf ("Invalid attribute value %s for property %s.",
						attr [i + 1], attr [i]));
				if (atchname)
					g_free (atchname);
				return;
			}

			dep->SetValue (prop, v);
			delete v;
			item->MarkPropertyAsSet (prop->name);
		} else {

			// This might be a property of a managed object
			if (p->loader && p->loader->SetAttribute (item->item, attr [i], attr [i + 1])) {
				if (atchname)
					g_free (atchname);
				continue;
			}

			if (dependency_object_hookup_event (p, item, pname, attr [i + 1]))
				parser_error (p, item->element_name, attr [i], 2012, g_strdup_printf ("Unknown attribute %s on element %s.",
						attr [i], p->current_element->element_name));
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

XamlElementInfo *
register_x_code_element (GHashTable *table)
{
	XamlElementInfo *res = new XamlElementInfo ("Code", NULL, Type::INVALID);

	res->create_element = create_x_code_directive_element;

	g_hash_table_insert (table, (char *) "Code", res);

	return res;
}

void
xaml_init (void)
{
	GHashTable *dem = g_hash_table_new (g_str_hash, g_str_equal); // default element map
	GHashTable *x_dem = g_hash_table_new (g_str_hash, g_str_equal); // x element map
	enum_map = g_hash_table_new (g_str_hash, g_str_equal);

	XamlElementInfo *col = register_ghost_element ("Collection", NULL, Type::COLLECTION);

#define rdoe register_dependency_object_element

	//
	// ui element ->
	//
	XamlElementInfo *ui = register_ghost_element ("UIElement", NULL, Type::UIELEMENT);
	XamlElementInfo *fw = register_ghost_element ("FrameworkElement", ui, Type::FRAMEWORKELEMENT);
	XamlElementInfo *shape = register_ghost_element ("Shape", fw, Type::SHAPE);

	rdoe (dem, "ResourceDictionary", col, Type::RESOURCE_DICTIONARY, (create_item_func) resource_dictionary_new);
	rdoe (dem, "VisualCollection", col, Type::VISUAL_COLLECTION, (create_item_func) visual_collection_new);

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


	/*XamlElementInfo *gc = */ rdoe (dem, "GeometryCollection", col, Type::GEOMETRY_COLLECTION, (create_item_func) geometry_collection_new);
	XamlElementInfo *pg = rdoe (dem, "PathGeometry", geo, Type::PATHGEOMETRY, (create_item_func) path_geometry_new);
	pg->content_property = "Figures";

	/*XamlElementInfo *pfc = */ rdoe (dem, "PathFigureCollection", col, Type::PATHFIGURE_COLLECTION, (create_item_func) NULL);

	XamlElementInfo *pf = rdoe (dem, "PathFigure", geo, Type::PATHFIGURE, (create_item_func) path_figure_new);
	pf->content_property = "Segments";

	/*XamlElementInfo *psc = */ rdoe (dem, "PathSegmentCollection", col, Type::PATHSEGMENT_COLLECTION, (create_item_func) path_segment_collection_new);

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

	XamlElementInfo *kfcol = register_ghost_element ("KeyFrameCollection", col, Type::KEYFRAME_COLLECTION);

	rdoe (dem, "ColorKeyFrameCollection", kfcol, Type::COLORKEYFRAME_COLLECTION, (create_item_func) color_key_frame_collection_new);
	rdoe (dem, "DoubleKeyFrameCollection", kfcol, Type::DOUBLEKEYFRAME_COLLECTION, (create_item_func) double_key_frame_collection_new);
	rdoe (dem, "PointKeyFrameCollection", kfcol, Type::POINTKEYFRAME_COLLECTION, (create_item_func) point_key_frame_collection_new);

	XamlElementInfo *keyfrm = register_ghost_element ("KeyFrame", NULL, Type::KEYFRAME);

	XamlElementInfo *ckf = register_ghost_element ("ColorKeyFrame", keyfrm, Type::COLORKEYFRAME);
	rdoe (dem, "DiscreteColorKeyFrame", ckf, Type::DISCRETECOLORKEYFRAME, (create_item_func) discrete_color_key_frame_new);
	rdoe (dem, "LinearColorKeyFrame", ckf, Type::LINEARCOLORKEYFRAME, (create_item_func) linear_color_key_frame_new);
	rdoe (dem, "SplineColorKeyFrame", ckf, Type::SPLINECOLORKEYFRAME, (create_item_func) spline_color_key_frame_new);

	XamlElementInfo *dkf = register_ghost_element ("DoubleKeyFrame", keyfrm, Type::DOUBLEKEYFRAME);
	rdoe (dem, "DiscreteDoubleKeyFrame", dkf, Type::DISCRETEDOUBLEKEYFRAME, (create_item_func) discrete_double_key_frame_new);
	rdoe (dem, "LinearDoubleKeyFrame", dkf, Type::LINEARDOUBLEKEYFRAME, (create_item_func) linear_double_key_frame_new);
	rdoe (dem, "SplineDoubleKeyFrame", dkf, Type::SPLINEDOUBLEKEYFRAME, (create_item_func) spline_double_key_frame_new);

	XamlElementInfo *pkf = register_ghost_element ("PointKeyFrame", keyfrm, Type::POINTKEYFRAME);
	rdoe (dem, "DiscretePointKeyFrame", pkf, Type::DISCRETEPOINTKEYFRAME, (create_item_func) discrete_point_key_frame_new);
	rdoe (dem, "LinearPointKeyFrame", pkf, Type::LINEARPOINTKEYFRAME, (create_item_func) linear_point_key_frame_new);
	rdoe (dem, "SplinePointKeyFrame", pkf, Type::SPLINEPOINTKEYFRAME, (create_item_func) spline_point_key_frame_new);

	rdoe (dem, "KeySpline", NULL, Type::KEYSPLINE, (create_item_func) key_spline_new);

	rdoe (dem, "TimelineCollection", col, Type::TIMELINE_COLLECTION, (create_item_func) timeline_collection_new);

	XamlElementInfo *timel = register_ghost_element ("Timeline", NULL, Type::TIMELINE);
	XamlElementInfo *tlg = rdoe (dem, "TimelineGroup", timel, Type::TIMELINEGROUP, (create_item_func) timeline_group_new);
	XamlElementInfo *prltl = register_ghost_element ("ParallelTimeline", tlg, Type::PARALLELTIMELINE);

	XamlElementInfo *sb = rdoe (dem, "Storyboard", prltl, Type::STORYBOARD, (create_item_func) storyboard_new);
	sb->content_property = "Children";

	rdoe (dem, "TimelineMarkerCollection", col, Type::TIMELINEMARKER_COLLECTION, (create_item_func) timeline_marker_collection_new);
	rdoe (dem, "TimelineMarker", NULL, Type::TIMELINEMARKER, (create_item_func) timeline_marker_new);

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
	
	/// Matrix

	rdoe (dem, "Matrix", NULL, Type::MATRIX, (create_item_func) matrix_new);

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
	
	rdoe (dem, "TransformCollection", col, Type::TRANSFORM_COLLECTION, (create_item_func) transform_collection_new);


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
	rdoe (dem, "VisualBrush", brush, Type::VISUALBRUSH, (create_item_func) visual_brush_new);

	///
	/// Media
	///

	XamlElementInfo *mb = register_ghost_element ("MediaBase", NULL, Type::MEDIABASE);

	rdoe (dem, "Image", mb, Type::IMAGE, (create_item_func) image_new);
	rdoe (dem, "MediaElement", mb, Type::MEDIAELEMENT, (create_item_func) media_element_new);
	rdoe (dem, "MediaAttribute", NULL, Type::MEDIAATTRIBUTE, (create_item_func) media_attribute_new);
	rdoe (dem, "MediaAttributeCollection", col, Type::MEDIAATTRIBUTE_COLLECTION, (create_item_func) media_attribute_collection_new);
	
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

	//
	// Stylus
	//

	rdoe (dem, "StylusPoint", NULL, Type::STYLUSPOINT, (create_item_func) stylus_point_new);
	rdoe (dem, "Stroke", NULL, Type::STROKE, (create_item_func) stroke_new);
	rdoe (dem, "DrawingAttributes", NULL, Type::DRAWINGATTRIBUTES, (create_item_func) drawing_attributes_new);
	XamlElementInfo *inkpresenter = rdoe (dem, "InkPresenter", canvas, Type::INKPRESENTER, (create_item_func) ink_presenter_new);
	inkpresenter->add_child = panel_add_child;
	rdoe (dem, "StrokeCollection", col, Type::STROKE_COLLECTION, (create_item_func) stroke_collection_new);
	rdoe (dem, "StylusPointCollection", col, Type::STYLUSPOINT_COLLECTION, (create_item_func) stylus_point_collection_new);

	//
	// Code
	//
	// FIXME: Make this v1.1 only
	register_x_code_element (x_dem);
	
#undef rdoe
	
	default_namespace = new DefaultNamespace (dem);
	x_namespace = new XNamespace (x_dem);
	
	///
	/// ENUMS
	///

	g_hash_table_insert (enum_map, (char *) "AlignmentX", GINT_TO_POINTER (alignment_x_map));
	g_hash_table_insert (enum_map, (char *) "AlignmentY", GINT_TO_POINTER (&alignment_y_map));
	g_hash_table_insert (enum_map, (char *) "MappingMode", GINT_TO_POINTER (brush_mapping_mode_map));
	g_hash_table_insert (enum_map, (char *) "ColorInterpolationMode", GINT_TO_POINTER (color_interpolation_mode_map));
	g_hash_table_insert (enum_map, (char *) "Cursor", GINT_TO_POINTER (cursors_map));
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
	g_hash_table_insert (enum_map, (char *) "StyleSimulations", GINT_TO_POINTER (style_simulations_map));
	g_hash_table_insert (enum_map, (char *) "SweepDirection", GINT_TO_POINTER (sweep_direction_map));
	g_hash_table_insert (enum_map, (char *) "DeviceType", GINT_TO_POINTER (tablet_device_type_map));
	g_hash_table_insert (enum_map, (char *) "TextDecorations", GINT_TO_POINTER (text_decorations_map));
	g_hash_table_insert (enum_map, (char *) "TextWrapping", GINT_TO_POINTER (text_wrapping_map));
	g_hash_table_insert (enum_map, (char *) "Visibility", GINT_TO_POINTER (visibility_map));
}
