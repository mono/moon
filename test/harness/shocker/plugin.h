


#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#define Visual _XVisual
#include <X11/X.h>
#include <X11/Xlib.h>
#undef Visual

class PluginObject;

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

	int GetX ();
	int GetY ();
	
	static Window browser_app_context;
	
private:	  
	NPP instance;
	NPWindow *window;
	AutoCapture* auto_capture;
	int x, y;
	
	ShockerScriptableControlObject* shocker_control;
	ShockerScriptableControlObject* GetShockerControl ();
	void UpdateXY ();
};


void Plugin_Initialize (NPPluginFuncs* npp_funcs);


//
// Make some plugin metadata available to the world
//
char* Plugin_GetMIMEDescription (void);
NPError Plugin_GetValue (NPP instance, NPPVariable variable, void *value);

#endif  // __PLUGIN_H__


