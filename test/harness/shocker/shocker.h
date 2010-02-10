
#ifndef __SHOCKER_H__
#define __SHOCKER_H__

struct ShockerScriptableControlObject;

#include "logging.h"
#include "image-capture.h"
#include "input.h"

struct ShockerScriptableControlType : NPClass {

	ShockerScriptableControlType ();
	~ShockerScriptableControlType () {}

};
extern ShockerScriptableControlType* ShockerScriptableControlClass;



struct ShockerScriptableControlObject : public NPObject
{
  
	ShockerScriptableControlObject (NPP instance);
	virtual ~ShockerScriptableControlObject ();

	void Connect ();
	void SignalShutdown ();

	InputProvider* GetInputProvider ();
	ImageCaptureProvider* GetImageCaptureProvider ();
	LogProvider* GetLogProvider ();
	PluginObject *GetPluginObject () { return (PluginObject *) instance->pdata; }
	
        //
	// Wrappers around some JS functions
	//
	char*         GetTestPath ();
private:
	NPP instance;
	char* test_path;

	InputProvider* input_provider;
	ImageCaptureProvider* image_capture;
};



bool Shocker_Initialize (void);
void Shocker_Shutdown (void);



#endif // __SHOCKER_H__


