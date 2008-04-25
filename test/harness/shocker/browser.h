
#ifndef __BROWSER_H__
#define __BROWSER_H__

#include "netscape.h"


//
// Browser is a somewhat evil Singleton.  It is completely useless before Initialize has
// been called on it, so if you try to get an Instance before that, you'll get an error
// message and I am sure things will crash real soon after that.
//

class Browser {

public:
	static void Initialize (NPNetscapeFuncs *browser_funcs);
	static Browser* Instance ();

	//
	// Wrappers around Netscape API calls
	//
	NPObject*     CreateObject (NPP npp, NPClass *klass);
	NPObject*     RetainObject (NPObject* obj);
	void          ReleaseObject (NPObject* obj);
	NPUTF8*       UTF8FromIdentifier (NPIdentifier identifier);
	NPIdentifier  GetStringIdentifier (const char* name);
	NPError       GetValue (NPP instance, NPNVariable variable, void *value);
	bool          GetProperty (NPP npp, NPObject* obj, NPIdentifier propertyName, NPVariant *result);
	bool          SetProperty (NPP npp, NPObject* obj, NPIdentifier propertyName, NPVariant *value);
	bool          Invoke (NPP npp, NPObject* obj, NPIdentifier method_name, const NPVariant *args, uint32_t arg_count, NPVariant *result);

private:
	Browser (NPNetscapeFuncs *browser_funcs);
	NPNetscapeFuncs browser_funcs;

	static Browser* instance;
};

void Browser_Initialize (NPNetscapeFuncs *browser_funcs);


#endif // __BROWSER_H__

