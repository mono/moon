#include <stdio.h>
#include <glib.h>

#include <mono/mini/jit.h>
#include <mono/metadata/appdomain.h>
#include <mono/metadata/mono-config.h>
#include <mono/metadata/debug-helpers.h>
G_BEGIN_DECLS
/* because this header sucks */
#include <mono/metadata/mono-debug.h>
G_END_DECLS

#define INCLUDED_MONO_HEADERS 1

#include "runtime.h"
#include "deployment.h"
#include "downloader.h"
#include "uri.h"
#include "pal.h"
#include "window.h"
#include "debug.h"

using namespace Moonlight;

const char *mono_config =
"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
"<configuration>"
"<dllmap dll=\"moon\" target=\"/opt/moonlight-osx/lib/libmoon.dylib\" />"
"<dllmap dll=\"libmoon\" target=\"/opt/moonlight-osx/lib/libmoon.dylib\" />"
"<dllmap dll=\"libmoon.so.0\" target=\"/opt/moonlight-osx/lib/libmoon.dylib\" />"
"<dllmap dll=\"moonplugin\" target=\"/opt/moonlight-osx/lib/libmoon.dylib\" />"
"</configuration>";

static void
error_handler (EventObject *sender, EventArgs *args, gpointer user_data) {
	ErrorEventArgs *err = (ErrorEventArgs *) args;

    g_debug ("LunarLauncher Error: %s\n", err->GetErrorMessage ());
}

int
main (int argc, char **argv) {
	MonoDomain *domain;
	Deployment *deployment;
	MoonWindow *window;
	MoonWindowingSystem *winsys;
	Surface *surface;
	char *data_dir;

//	setenv ("MONO_PATH", "/opt/mono/lib/mono/2.0:/opt/moonlight-osx/lib/mono/moonlight/", 1);
	setenv ("MONO_PATH", "/Users/plasma/Work/darwoon/moon/class/lib/2.1", 1);
	//setenv ("MOONLIGHT_DEBUG", "mediaplayer", 1);
	setenv ("MOONLIGHT_OVERRIDES", "curlbridge=yes", 1);
	setenv ("MOONLIGHT_DISABLE_INCOMPLETE_MESSAGE", "1", 1);
	setenv ("XAP_URI", "file://Users/plasma/tmp/xap", 1);

	mono_config_parse_memory(mono_config);
	mono_debug_init (MONO_DEBUG_FORMAT_MONO);
	mono_set_signal_chaining (true);

	domain = mono_jit_init_version ("Moonlight for OSX Root Domain", "v2.0.50727");

	mono_jit_set_trace_options ("E:all");

	Runtime::InitDesktop ();
	winsys = Runtime::GetWindowingSystem ();
	winsys->SetPlatformWindowingSystemData (NULL);

	deployment = Deployment::GetCurrent ();

	window = winsys->CreateWindow (MoonWindowType_Desktop, 640, 480);
	surface = new Surface (window);

	deployment->SetSurface (surface);
	window->SetSurface (surface);

	surface->SetSourceLocation (Uri::Create ("file:///Users/toshok/tmp/xap"));
	deployment->SetXapFilename (argv[1]);
	deployment->SetXapLocation (Uri::Create ("file:///Users/toshok/src/moonlight-android/moonlight-activity/DemoApp.xap"));

	window->SetTitle (argv[1]);

	surface->AddXamlHandler (Surface::ErrorEvent, error_handler, NULL);
	surface->unref ();

	if (!deployment->InitializeManagedDeployment (NULL, NULL, NULL)) {
		surface->unref ();
		g_debug ("Could not initialize managed deployment");
		return 0;
	}

	g_warning ("pid: %i\n", getpid ());

	winsys->RunMainLoop (window);

	// Shutdown ungracefully for now
	
	return 1;
}
