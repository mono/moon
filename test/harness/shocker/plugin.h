


#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "netscape.h"
#include "shocker.h"


class PluginObject {

public:
	PluginObject (NPP instance, int argc, char* argn[], char* argv[]);
	virtual ~PluginObject ();

	void Shutdown ();

	NPError GetValue (NPPVariable variable, void *value);
	NPError SetValue (NPNVariable variable, void *value);

	NPError SetWindow (NPWindow* window);

private:	  
	NPP instance;
	AutoCapture* auto_capture;

	ShockerScriptableControlObject* shocker_control;
	ShockerScriptableControlObject* GetShockerControl ();

};


void Plugin_Initialize (NPPluginFuncs* npp_funcs);


//
// Make some plugin metadata available to the world
//
char* Plugin_GetMIMEDescription (void);
NPError Plugin_GetValue (NPP instance, NPPVariable variable, void *value);

#endif  // __PLUGIN_H__


