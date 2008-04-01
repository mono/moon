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
#include "error.h"

class XamlLoader;
struct XamlLoaderCallbacks;

typedef DependencyObject *xaml_load_managed_object_callback (const char* asm_name, const char* asm_path, const char* name, const char* type_name);
typedef bool xaml_set_custom_attribute_callback (void *target, const char *name, const char *value);
typedef bool xaml_hookup_event_callback (void *target, const char *ename, const char *evalue);
typedef void xaml_insert_mapping_callback (const char* key, const char* value); 
typedef const char* xaml_get_mapping_callback (const char* key);
typedef bool xaml_load_code_callback (const char *source, const char *type);
typedef void xaml_set_name_attribute_callback (void *target, const char *name);

G_BEGIN_DECLS

DependencyObject  *xaml_create_from_file (XamlLoader* loader, const char *xaml, bool create_namescope, Type::Kind *element_type);
DependencyObject  *xaml_create_from_str  (XamlLoader* loader, const char *xaml, bool create_namescope, Type::Kind *element_type);
bool        xaml_set_property_from_str (DependencyObject *obj, DependencyProperty *prop, const char *value);

bool        value_from_str_with_typename (const char *type_name, const char *prop_name, const char *str, Value **v);
bool        value_from_str (Type::Kind type, const char *prop_name, const char *str, Value **v);
bool        convert_property_value_to_enum_str (DependencyProperty *prop, Value *v, const char **s);

void xaml_loader_set_callbacks (XamlLoader* loader, XamlLoaderCallbacks callbacks);

void        xaml_init (void);

bool            time_span_from_str (const char *str, TimeSpan *res);
XamlLoader* xaml_loader_new (const char* filename, const char* str, Surface* surface);
void		xaml_loader_free (XamlLoader* loader);
void		xaml_parse_xmlns (const char* xmlns, char** type_name, char** ns, char** assembly);
void		xaml_loader_add_missing (XamlLoader* loader, const char* file);
G_END_DECLS

struct XamlLoaderCallbacks
{	
public:
	xaml_load_managed_object_callback *load_managed_object;
	xaml_set_custom_attribute_callback *set_custom_attribute;
	xaml_hookup_event_callback *hookup_event;
	xaml_get_mapping_callback *get_mapping;
	xaml_insert_mapping_callback *insert_mapping;
	xaml_load_code_callback *load_code;
	xaml_set_name_attribute_callback *set_name_attribute;

	XamlLoaderCallbacks () :
		load_managed_object (NULL), set_custom_attribute (NULL),
		hookup_event (NULL), get_mapping (NULL),
		insert_mapping (NULL), load_code (NULL),
		set_name_attribute (NULL) 
	{
	}
};

/*

  Plugin:
    - calls PluginXamlLoader::TryLoad to try to load some xaml.
    -  calls xaml_create_from_*
    -     calls XamlLoader::CreateManagedObject (,) if it encounters xmlns/name
    -      parses the xmlns and name
    -       calls XamlLoader::LoadVM.
    -        PluginXamlLoader::LoadVM will load the vm and create a ManagedXamlLoader (which will set the callbacks in XamlLoader)
    -       calls XamlLoader::CreateManagedObject (,,,) with the parsed xml
    -        calls the create_managed_object callback (if any).
    -          will try to load the assembly, if it fails, it's requested.
    -  if XamlLoader::CreateManagedObject failed, try to download the missing assembly (if any).
    -  if no missing assembly, the xaml load fails.

  Deskop:
    - calls System.Windows.XamlReader::Load
    -  creates a ManagedXamlLoader and a native XamlLoader (setting the callbacks).
    -  calls xaml_create_from_str
    -     calls XamlLoader::CreateManagedObject (,) if it encounters xmlns/name
    -      parses the xmlns and name
    -       calls XamlLoader::LoadVM (which does nothing).
    -       calls XamlLoader::CreateManagedObject (,,,) with the parsed xml
    -        calls the create_managed_object callback (if any).
    -          will try to load the assembly, if it fails, it's requested.
    -    destroy the native/managed XamlLoader. Any requested assemblies are ignored, no retries are done.
*/


class XamlLoader
{
public:
	enum AssemblyLoadResult {
		SUCCESS = -1,
		MissingAssembly = 1,
		LoadFailure = 2
	};

private:
	Surface* surface;
	char* filename;
	char* str;
	GHashTable* mappings;
	GHashTable* missing_assemblies;

public:
	XamlLoader (const char* filename, const char* str, Surface* surface);
	virtual ~XamlLoader ();
	virtual bool LoadVM ();
	virtual DependencyObject* CreateManagedObject (const char* xmlns, const char* name);
	virtual DependencyObject* CreateManagedObject (const char* asm_name, const char* asm_path, const char* name, const char* type_name);
	virtual bool SetAttribute (void* target, const char* name, const char* value);
	virtual void SetNameAttribute (void* target, const char* name);
	virtual bool HookupEvent (void* target, const char* name, const char* value);
	virtual void InsertMapping (const char* key, const char* value);
	const char* GetMapping (const char* key);
	bool LoadCode (const char *source, const char *type);
	
	char* GetFilename () { return filename; }
	char* GetString () { return str; }
	Surface* GetSurface () { return surface; }
	
	const char* GetMissing ();
	virtual void AddMissing (const char* assembly);
	void RemoveMissing (const char* assembly);

	bool vm_loaded;
public:
	XamlLoaderCallbacks callbacks;
	ParserErrorEventArgs *error_args;
};

#endif /* __MOON_XAML_H__ */
