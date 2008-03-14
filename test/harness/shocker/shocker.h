
#ifndef __SHOCKER_H__
#define __SHOCKER_H__

#include "input.h"
#include "image-capture.h"
#include "logging.h"
#include "netscape.h"


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

	InputProvider* GetInputProvider () { return input_provider; }
	ImageCaptureProvider* GetImageCaptureProvider () { return image_capture; }
	LogProvider* GetLogProvider () { return log_provider; }

        //
	// Wrappers around some JS functions
	//
	char*         GetTestPath ();

private:
	NPP instance;
	char* test_path;

	InputProvider* input_provider;
	ImageCaptureProvider* image_capture;
	LogProvider* log_provider;
};



bool Shocker_Initialize (void);
void Shocker_Shutdown (void);



#endif // __SHOCKER_H__


