/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * shocker.h
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __SHOCKER_H__
#define __SHOCKER_H__

struct ShockerScriptableObjectMethod;
struct ShockerScriptableControlMethod;
struct ShockerScriptableControlObject;
struct RenderDataCapturerObject;
struct RenderCapturerDataMethod;

#include "logging.h"
#include "image-capture.h"
#include "input.h"

/*
 * ShockerScriptableObject
 */

struct ShockerScriptableObjectType : NPClass {
	ShockerScriptableObjectType ();
};

struct ShockerScriptableObject : public NPObject {
	ShockerScriptableObject (NPP instance)
	{
		this->instance = instance;
	}
	virtual ~ShockerScriptableObject () {}
	virtual void Invalidate () {}
	virtual bool HasProperty (NPIdentifier name) = 0;
	virtual bool GetProperty (NPIdentifier name, NPVariant *result) = 0;
	virtual bool SetProperty (NPIdentifier name, const NPVariant *value) { return false; }
	virtual bool RemoveProperty (NPIdentifier name) { return false; }
	bool HasMethod (NPIdentifier name);
	bool Invoke (NPIdentifier name, const NPVariant *args, guint32 argCount, NPVariant *result);
	virtual bool InvokeDefault (const NPVariant *args, uint32_t argCount, NPVariant *result) { return false; }

	virtual ShockerScriptableObjectMethod *GetMethods () = 0;
	virtual const char *GetTypeName () = 0;

	NPP instance;
};

typedef void (ShockerScriptableObject::*scriptable_method) (const NPVariant* args, uint32_t arg_count, NPVariant *result);
struct ShockerScriptableObjectMethod {
    const char* name;
    scriptable_method Invoke;
};

/*
 * ShockerScriptableControl
 */

struct ShockerScriptableControlType : ShockerScriptableObjectType {
	ShockerScriptableControlType ();
};

extern ShockerScriptableControlType* ShockerScriptableControlClass;

struct ShockerScriptableControlObject : public ShockerScriptableObject
{
	ShockerScriptableControlObject (NPP instance);
	virtual ~ShockerScriptableControlObject ();
	virtual bool HasProperty (NPIdentifier id);
	virtual bool GetProperty (NPIdentifier id, NPVariant *result);

	virtual ShockerScriptableObjectMethod *GetMethods () { return (ShockerScriptableObjectMethod *) methods; }
	virtual const char *GetTypeName () { return "ShockerScriptableControlObject"; }

	void Connect                    (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void SignalShutdown             (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void LogError                   (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void LogDebug                   (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void LogResult                  (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void LogMessage                 (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void LogWarning                 (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void LogHelp                    (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MoveMouseLogarithmic       (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MoveMouse                  (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MouseIsAtPosition          (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MouseDoubleClick           (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MouseLeftClick             (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MouseRightClick            (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MouseLeftButtonDown        (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MouseLeftButtonUp          (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void SendKeyInput               (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void SetKeyboardInputSpeed      (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void CompareImages              (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetActiveInputLocaleId     (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void ActivateKeyboardLayout     (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void IsInputLocaleInstalled     (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetTestDirectory           (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetTestDefinition          (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetRuntimePropertyValue    (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void SetRuntimePropertyValue    (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void CleanDRM                   (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void SwitchToHighContrastScheme (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void StartLog                   (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void EndLog                     (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void MouseWheel                 (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void EnablePrivacyPrompts       (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetRenderDataCapturer      (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void LogPerfEvent               (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void InitializePerfProvider     (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void UninitializePerfProvider   (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetPlatformName            (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetPlatformVersion         (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void StartWebCamWriter          (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void StopWebCamWriter           (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void CaptureSingleImage         (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void CaptureMultipleImages      (const NPVariant *args, uint32_t arg_count, NPVariant *result);

	InputProvider *       GetInputProvider ();
	ImageCaptureProvider *GetImageCaptureProvider ();
	LogProvider *         GetLogProvider ();
	PluginObject *        GetPluginObject () { return (PluginObject *) instance->pdata; }
	const char *         GetTestPath ();

private:
	char *test_path;
	ImageCaptureProvider *image_capture;
	RenderDataCapturerObject *render_data_capturer;
	static ShockerScriptableControlMethod methods [];
};

typedef void (ShockerScriptableControlObject::*scriptable_control_method) (const NPVariant* args, uint32_t arg_count, NPVariant *result);
struct ShockerScriptableControlMethod {
    const char* name;
    scriptable_control_method Invoke;
};

/*
 * RenderDataCapturer
 */

struct RenderDataCapturerType : ShockerScriptableObjectType {
	RenderDataCapturerType ();
};

extern RenderDataCapturerType *RenderDataCapturerClass;

struct RenderDataCapturerObject : public ShockerScriptableObject {
private:

public:
	RenderDataCapturerObject (NPP instance);
	virtual ~RenderDataCapturerObject () {}

	virtual bool HasProperty (NPIdentifier id);
	virtual bool GetProperty (NPIdentifier id, NPVariant *result);
	
	virtual ShockerScriptableObjectMethod *GetMethods () { return (ShockerScriptableObjectMethod *) methods; }
	virtual const char *GetTypeName () { return "RenderDataCapturerObject"; }

	void CaptureSingleImage              (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void RegisterGetNextFrameDataRequest (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void IsFrameDataAvailable            (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetFrameData                    (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void IsEnabled                       (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void SetFailMode                     (const NPVariant *args, uint32_t arg_count, NPVariant *result);
	void GetFailMode                     (const NPVariant *args, uint32_t arg_count, NPVariant *result);

	static RenderCapturerDataMethod methods [];
};

typedef void (RenderDataCapturerObject::*render_data_capturer_method) (const NPVariant* args, uint32_t arg_count, NPVariant *result);
struct RenderCapturerDataMethod {
    const char* name;
    render_data_capturer_method Invoke;
};

bool Shocker_Initialize ();
void Shocker_Shutdown ();
void Shocker_FailTestFast (const char *msg);
#endif // __SHOCKER_H__


