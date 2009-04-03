/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 *
 * netscape.cpp: Mozilla plugin entry point functions
 *
 */
 
#include <dlfcn.h>

#include "netscape.h"
#include "shocker.h"
#include "plugin.h"
#include "browser.h"
#include "shutdown-manager.h"

//
// These are the functions that mozilla looks up and is going to call to figure out
// what kind of plugin we are and to instantiate/destroy us
//

NPError
NP_Initialize (NPNetscapeFuncs* mozilla_funcs, NPPluginFuncs* plugin_funcs)
{
#ifdef SHOCKER_DEBUG
	printf ("NP_Initialize\n");
#endif

	Dl_info dl_info;
	// Prevent firefox from unloading us
	if (dladdr ((void *) &NP_Initialize, &dl_info) != 0) {
		void *handle = dlopen (dl_info.dli_fname, RTLD_LAZY | RTLD_NOLOAD);
		if (handle == NULL)
			printf ("[shocker] tried to open a handle to libshocker.so, but: '%s' (rare crashes might occur).\n", dlerror ());
	} else {
		printf ("[shocker] could not get path of libshocker.so: '%s' (rare crashes might occur).\n", dlerror ());
	}
	
	Browser_Initialize (mozilla_funcs);
	Plugin_Initialize (plugin_funcs);
	Shocker_Initialize ();

	return NPERR_NO_ERROR;
}

NPError
NP_Shutdown (void)
{
#ifdef SHOCKER_DEBUG
    printf ("NP_Shutdown\n");
#endif

    Shocker_Shutdown ();
    
    return NPERR_NO_ERROR;
}

char *
NP_GetMIMEDescription (void)
{
#ifdef SHOCKER_DEBUG
	printf ("NP_GetMIMEDescription\n");
#endif

	return Plugin_GetMIMEDescription ();
}

NPError
NP_GetValue (void *future, NPPVariable variable, void *value)
{
#ifdef SHOCKER_DEBUG
	printf ("NP_GetValue\n");
#endif

	return Plugin_GetValue ((NPP) future, variable, value);
}

