/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PAL_H
#define MOON_PAL_H

#if defined _WIN32 || defined __CYGWIN__
#define MOON_DLL_EXPORT __declspec(dllexport)
#define MOON_DLL_IMPORT __declspec(dllimport)
#define MOON_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define MOON_DLL_EXPORT __attribute__ ((visibility("default")))
#define MOON_DLL_IMPORT __attribute__ ((visibility("default")))
#define MOON_DLL_LOCAL  __attribute__ ((visibility("hidden")))
#else
#define MOON_DLL_EXPORT
#define MOON_DLL_IMPORT
#define MOON_DLL_LOCAL
#endif
#endif

#if BUILDING_LIBMOON
#define MOON_API MOON_DLL_EXPORT
#define MOON_LOCAL MOON_DLL_LOCAL
#else
#define MOON_API MOON_DLL_IMPORT
#define MOON_LOCAL
#endif

#include <glib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <float.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "font-utils.h"
#include "enums.h"
#include "cairo.h"
#include "list.h"
#include "gchandle.h"

// I hate X11
#ifdef FocusIn
#undef FocusIn
#endif
#ifdef FocusOut
#undef FocusOut
#endif

#ifndef HAVE_POSIX_MEMALIGN
extern "C" {
	int posix_memalign (void **ptr, size_t alignment, size_t size);
}
#endif

#if SANITY
#define glError() { \
	GLenum err = glGetError(); \
	if (err != GL_NO_ERROR) { \
		g_warning ("glError: %i caught at %s:%u\n", err, __FILE__, __LINE__); \
	} \
}
#else
#define glError()
#endif

// the default for MoonWindowingSystem::GetCursorBlinkTimeout
#define CURSOR_BLINK_TIMEOUT_DEFAULT  900

namespace Moonlight {

class Surface;
class UIElement;
class Deployment;
class HttpRequest;
class EventObject;
class EventArgs;
class PluginInstance;
class Uri;
struct AudioFormat;
struct VideoFormat;
class AudioFormatCollection;
class VideoFormatCollection;
class CaptureDevice;
class AudioCaptureDevice;
class VideoCaptureDevice;
class AudioCaptureDeviceCollection;
class VideoCaptureDeviceCollection;

class MoonEvent;
class MoonWindow;
class MoonClipboard;
class MoonError;

struct Point;
struct Rect;
struct Color;

enum MoonModifier {
	MoonModifier_Shift    = 1 << 0,
	MoonModifier_Lock     = 1 << 1,
	MoonModifier_Control  = 1 << 2,

	MoonModifier_Mod1	    = 1 << 3,
	MoonModifier_Mod2	    = 1 << 4,
	MoonModifier_Mod3	    = 1 << 5,
	MoonModifier_Mod4	    = 1 << 6,
	MoonModifier_Mod5	    = 1 << 7,

	MoonModifier_Super    = 1 << 26,
	MoonModifier_Hyper    = 1 << 27,
	MoonModifier_Meta     = 1 << 28,
};


#define MOON_PRIORITY_HIGH         -100
#define MOON_PRIORITY_DEFAULT      0
#define MOON_PRIORITY_HIGH_IDLE    100
#define MOON_PRIORITY_DEFAULT_IDLE 200

enum MoonWindowType {
	MoonWindowType_FullScreen,
	MoonWindowType_Desktop,
	MoonWindowType_Plugin
};

// useful abstractions for porting moonlight to other platforms.

// returns true if the timeout/idle should be removed
typedef bool (*MoonSourceFunc) (gpointer data);

typedef bool (*MoonCallback) (gpointer sender, gpointer data);

typedef void (*MoonClipboardGetTextCallback) (MoonClipboard *clipboard, const char *text, gpointer data);

enum MoonEventStatus {
	MoonEventNotSupported = -1,
	MoonEventNotHandled,
	MoonEventHandled
};

class MoonEvent {
public:
	virtual ~MoonEvent () {}
	virtual MoonEvent *Clone () = 0;

	// returns a platform event so that other
	// platform interfaces which consume events can get at the actual data.
	virtual gpointer GetPlatformEvent() = 0;
	
	virtual MoonEventStatus DispatchToWindow (MoonWindow *window) = 0;

	virtual bool HasModifiers () { return false; }
	
	// FIXME: should this be separate bool getters instead (like IsShiftDown, IsCtrlDown, IsAltDown)?
	virtual MoonModifier GetModifiers () { return (MoonModifier) 0; };
};

class MoonKeyEvent : public MoonEvent {
public:
	virtual Key GetSilverlightKey () = 0; // returns the enum value.  this requires platform specific mapping

	virtual int GetPlatformKeycode () = 0; // FIXME: do we really need both of these?
	virtual int GetPlatformKeyval () = 0;

	virtual gunichar GetUnicode () = 0;

	virtual bool IsModifier () = 0;
};

class MoonMouseEvent : public MoonEvent {
public:
	virtual Point GetPosition () = 0;

	virtual double GetPressure () = 0;

	virtual void GetStylusInfo (TabletDeviceType *type, bool *is_inverted) = 0;
};

class MoonButtonEvent : public MoonMouseEvent {
public:
	virtual bool IsRelease () = 0;

	virtual int GetButton () = 0;

	// the number of clicks.  gdk provides them as event->type ==
	// GDK_3BUTTON_PRESS/GDK_2BUTTON_PRESS/GDK_BUTTON_PRESS
	virtual int GetNumberOfClicks () = 0; // FIXME: will this api work?
};

class MoonMotionEvent : public MoonMouseEvent {
};

class MoonCrossingEvent : public MoonMouseEvent {
public:
	virtual bool IsEnter () = 0;
};

class MoonScrollWheelEvent : public MoonMouseEvent {
public:
	virtual int GetWheelDelta () = 0;
};

class MoonFocusEvent : public MoonEvent {
public:
	virtual bool IsIn() = 0;
};


class MoonIMContext {
public:
	virtual void SetUsePreedit (bool flag) = 0;
	virtual void SetClientWindow (MoonWindow*  window) = 0;
	virtual void SetSurroundingText (const char *text, int offset, int length) = 0;
	virtual void Reset () = 0;

	virtual void FocusIn () = 0;
	virtual void FocusOut () = 0;

	virtual void SetCursorLocation (Rect r) = 0;

	virtual bool FilterKeyPress (MoonKeyEvent* event) = 0;

	virtual void SetRetrieveSurroundingCallback (MoonCallback cb, gpointer data) = 0;
	virtual void SetDeleteSurroundingCallback (MoonCallback cb, gpointer data) = 0;
	virtual void SetCommitCallback (MoonCallback cb, gpointer data) = 0;

	virtual gpointer GetPlatformIMContext () = 0;
	virtual ~MoonIMContext () {}
};

enum MoonClipboardType {
	MoonClipboard_Clipboard,
	MoonClipboard_Primary
};

class MoonClipboard {
public:
	/* @GeneratePInvoke */
	virtual bool ContainsText () = 0;
	/* @GeneratePInvoke */
	virtual void SetText (const char *text) = 0;
	virtual void AsyncGetText (MoonClipboardGetTextCallback cb, gpointer data) = 0;
	/* @GeneratePInvoke */
	virtual char* GetText () = 0;
	virtual ~MoonClipboard () {}
};

class MoonPixbuf {
public:
	virtual gint GetWidth () = 0;
	virtual gint GetHeight () = 0;
	virtual gint GetRowStride () = 0;
	virtual gint GetNumChannels () = 0;
	virtual guchar *GetPixels () = 0;
	virtual gboolean IsPremultiplied () = 0;
	virtual gpointer GetPlatformPixbuf () = 0;
	virtual ~MoonPixbuf () {}
};

class MoonPixbufLoader {
public:
	virtual void Write (const guchar *buffer, int buflen, MoonError **error = NULL) = 0;
	virtual void Close (MoonError **error = NULL) = 0;
	virtual MoonPixbuf *GetPixbuf () = 0;
	virtual ~MoonPixbufLoader () {}
};

enum MoonMessageBoxType {
	MessageBoxTypeInfo,
	MessageBoxTypeQuestion,
	MessageBoxTypeWarning
};

// must match values from System.Windows.MessageBoxButtons
enum MoonMessageBoxButton {
	MessageBoxButtonOk,
	MessageBoxButtonOkCancel,
	MessageBoxButtonYesNo  // extra value just in native
};

// must match values from System.Windows.MessageBoxResult
enum MoonMessageBoxResult {
	MessageBoxResultNone = 0,
	MessageBoxResultOk = 1,
	MessageBoxResultCancel = 2,
	MessageBoxResultYes = 6,
	MessageBoxResultNo = 7
};

typedef MoonWindow* (*MoonWindowlessCtor)(int width, int height, PluginInstance *forPlugin);

class MoonWindowingSystem {
public:
	MoonWindowingSystem() { windowless_ctor = NULL; }

	virtual ~MoonWindowingSystem () {}

	// creates a platform/windowing system specific surface
	virtual cairo_surface_t *CreateSurface () = 0;
	virtual void ExitApplication () = 0;

	/* @GeneratePInvoke */
	virtual MoonWindow *CreateWindow (MoonWindowType windowType, int width = -1, int height = -1, MoonWindow *parentWindow = NULL, Surface* surface = NULL) = 0;
	virtual MoonWindow *CreateWindowless (int width, int height, PluginInstance *forPlugin);

	/* @GeneratePInvoke */
	virtual MoonMessageBoxResult ShowMessageBox (MoonMessageBoxType messagebox_type, const char *caption, const char *text, MoonMessageBoxButton button) = 0;

	/* @GeneratePInvoke */
	virtual gchar** ShowOpenFileDialog (const char *title, bool multsel, const char *filter, int idx) = 0;
	/* @GeneratePInvoke */
	virtual char* ShowSaveFileDialog (const char *title, const char *filter, int idx) = 0;

	/* @GeneratePInvoke */
	virtual bool ShowConsentDialog (const char *question, const char *detail, const char *website, bool *remember) = 0;

	/* @GeneratePInvoke */
	virtual Color *GetSystemColor (SystemColor id) = 0;
	
	virtual guint AddTimeout (gint priority, gint ms, MoonSourceFunc timeout, gpointer data) = 0;
	virtual void RemoveTimeout (guint timeoutId) = 0;

	virtual guint AddIdle (MoonSourceFunc idle, gpointer data) = 0;
	virtual void RemoveIdle (guint idle_id) = 0;

	virtual MoonIMContext* CreateIMContext () = 0;

	virtual MoonEvent* CreateEventFromPlatformEvent (gpointer platformEvent) = 0;

	virtual guint GetCursorBlinkTimeout (MoonWindow *window) = 0;

	virtual MoonPixbufLoader* CreatePixbufLoader (const char *imageType) = 0;

	void SetWindowlessCtor (MoonWindowlessCtor ctor);

	virtual void RunMainLoop (MoonWindow *main_window = NULL, bool quit_on_window_close = true) = 0;

	virtual void ShowCodecsUnavailableMessage () = 0;

	/* @GeneratePInvoke */
	virtual guint32 GetScreenHeight (MoonWindow *moon_window) = 0;

	/* @GeneratePInvoke */
	virtual guint32 GetScreenWidth (MoonWindow *moon_window) = 0;

	// buffer must have a size of at least buffer_stride * buffer_height
	virtual bool ConvertJPEGToBGRA (void *jpeg, guint32 jpeg_size, guint8 *buffer, guint32 buffer_stride, guint32 buffer_height) = 0;

	virtual gchar *GetTemporaryFolder () = 0;

	void SetPlatformWindowingSystemData (gpointer data) {
		system_data = data;
	}
	gpointer GetPlatformWindowingSystemData () { return system_data; }

protected:
	gpointer system_data;

private:
	MoonWindowlessCtor windowless_ctor;
};

struct MOON_API MoonAppRecord {
	char *origin;
	time_t mtime;
	char *uid;
	
	MoonAppRecord ();
	~MoonAppRecord ();
	
	bool Equal (const MoonAppRecord *app) const;
	bool Save (FILE *db) const;
};

class MoonAppDatabase {
	char *base_dir;
	char *path;
	
public:
	MoonAppDatabase (const char *base_dir);
	~MoonAppDatabase ();
	
	MoonAppRecord *CreateAppRecord (const Uri *origin);
	
	bool AddAppRecord (MoonAppRecord *record);
	bool SyncAppRecord (const MoonAppRecord *record);
	bool RemoveAppRecord (const MoonAppRecord *record);
	
	MoonAppRecord *GetAppRecordByOrigin (const Uri *origin);
	MoonAppRecord *GetAppRecordByUid (const char *uid);
};

typedef void (* UpdateCompletedCallback) (bool updated, const char *error, gpointer user_data);

class MOON_API MoonInstallerService {
	UpdateCompletedCallback completed;
	HttpRequest *request;
	MoonAppDatabase *db;
	gpointer user_data;
	MoonAppRecord *app;
	
	MoonAppRecord *GetAppRecord (Deployment *deployment);
	void CloseDownloader (bool abort);
	bool InitDatabase ();
	
	static void downloader_stopped (EventObject *sender, EventArgs *args, gpointer user_data);
	
protected:
	MoonAppRecord *CreateAppRecord (const Uri *origin);
	
public:
	MoonInstallerService ();
	virtual ~MoonInstallerService ();
	
	void CheckAndDownloadUpdateAsync (Deployment *deployment, UpdateCompletedCallback completed, gpointer user_data);
	
	virtual bool Install (Deployment *deployment, bool unattended) = 0;
	virtual bool Uninstall (Deployment *deployment);
	
	virtual const char *GetBaseInstallDir () = 0;
	
	MoonAppRecord *GetAppRecord (const char *uid);
	
	bool IsRunningOutOfBrowser (Deployment *deployment);
	bool CheckInstalled (Deployment *deployment);
	
	void UpdaterCompleted ();
	void UpdaterFailed (const char *msg);
};

// XXX we need to think about multitouch events/tablets/accelerometers/gtk extension events, etc.

typedef char* (*MessageReceivedCallback) (const char *message, gpointer data);
typedef void (*MessageSentCallback) (MoonError *error, const char *message, const char *response, GCHandle managedUserState, gpointer data);

class MoonMessageListener {
public:
	MoonMessageListener () {};
	virtual ~MoonMessageListener () {};
	
	virtual void AddMessageReceivedCallback (MessageReceivedCallback messageReceivedCallback, gpointer data) = 0;
	virtual void RemoveMessageReceivedCallback () = 0;
};

class MoonMessageSender {
public:
	MoonMessageSender () {};
	virtual ~MoonMessageSender () {};
	
	virtual void AddMessageSentCallback (MessageSentCallback messageSentCallback, gpointer data) = 0;
	virtual void RemoveMessageSentCallback () = 0;

	virtual void SendMessageAsync (const char *msg, GCHandle managedUserState, MoonError *error) = 0;
};

class MoonMessagingService {
public:
	MoonMessagingService () {};
	virtual ~MoonMessagingService () {};

	virtual MoonMessageListener* CreateMessagingListener (const char *domain, const char *listenerName, MoonError *error) = 0;
	virtual MoonMessageSender* CreateMessagingSender (const char *listenerName, const char *listenerDomain, const char *domain, MoonError *error) = 0;
};

class MoonCaptureDevice {
	CaptureDevice *device;

protected:
	CaptureDevice *GetDevice () { return device; }

public:
	MoonCaptureDevice ();
	virtual ~MoonCaptureDevice ();

	virtual const char* GetFriendlyName () = 0;
	void SetDevice (CaptureDevice *value) { device = value; }

	virtual void StartCapturing () = 0;
	virtual void StopCapturing () = 0;
};

class MoonVideoCaptureDevice : public MoonCaptureDevice {
protected:
	VideoCaptureDevice *GetDevice () { return (VideoCaptureDevice *) MoonCaptureDevice::GetDevice (); }

public:
	MoonVideoCaptureDevice () {};
	virtual ~MoonVideoCaptureDevice () {};

	virtual void GetSupportedFormats (VideoFormatCollection *col) = 0;
};

class MoonAudioCaptureDevice : public MoonCaptureDevice {
protected:
	AudioCaptureDevice *GetDevice () { return (AudioCaptureDevice *) MoonCaptureDevice::GetDevice (); }

public:
	MoonAudioCaptureDevice () {};
	virtual ~MoonAudioCaptureDevice () {};

	virtual void GetSupportedFormats (AudioFormatCollection *col) = 0;
};

class MoonVideoCaptureService {
public:
	MoonVideoCaptureService () {};
	virtual ~MoonVideoCaptureService () {}

	virtual void GetAvailableCaptureDevices (VideoCaptureDeviceCollection *col) = 0;
};

class MoonAudioCaptureService {
public:
	MoonAudioCaptureService () {};
	virtual ~MoonAudioCaptureService () {}

	virtual void GetAvailableCaptureDevices (AudioCaptureDeviceCollection *col) = 0;
};

class MoonCaptureService {
public:
	MoonCaptureService () {};
	virtual ~MoonCaptureService () {};

	virtual MoonVideoCaptureService *GetVideoCaptureService() = 0;
	virtual MoonAudioCaptureService *GetAudioCaptureService() = 0;
};

class MoonNetworkService {
public:
	MoonNetworkService () {};
	virtual ~MoonNetworkService () {};

	// registers a callback with the service which should be invoked any time a state change happens:
	// 1. network address changed
	// 2. network up/down
	// 3. etc...
	// return value is true for success in setting up the service, false for an error
	/* @GeneratePInvoke */
	virtual bool SetNetworkStateChangedCallback (MoonCallback callback, gpointer data) = 0;

	/* @GeneratePInvoke */
	virtual bool GetIsNetworkAvailable () = 0;
};

struct MoonFont {
	char *path;
	int index;
	
	MoonFont (const char *path, int index)
	{
		this->path = g_strdup (path);
		this->index = index;
	}
	
	~MoonFont () { g_free (path); }
};

typedef bool (* MoonForeachFontCallback) (const char *path, int index, gpointer user_data);

class MoonFontService {
public:
	FT_Library libft2;
	
	MoonFontService ();
	virtual ~MoonFontService ();
	
	virtual void ForeachFont (MoonForeachFontCallback foreach, gpointer user_data) = 0;
	virtual MoonFont *FindFont (const FontStyleInfo *pattern) = 0;
	
	virtual guint32 GetCharIndex (FT_Face face, gunichar unichar);
};

};
#endif /* MOON_PAL_H */
