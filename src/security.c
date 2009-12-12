/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * security.c:
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include "config.h"
#include "security.h"

#if MONO_ENABLE_CORECLR_SECURITY

static struct stat platform_stat;

static struct stat platform_a11y_stat;

void
a11y_stat_init (char *platform_dir)
{
	//please keep this lookup pattern in sync with the one in A11yHelper.cs (Initiailize() method)
	const char* moonlight_at_novell = g_strrstr (platform_dir, "moonlight@novell.com");
	if (moonlight_at_novell != NULL) {
		const char* after = g_strdup ("moonlight-a11y@novell.com/components");
		const char* before = g_strndup (platform_dir, 
		                                strlen (platform_dir) - strlen (moonlight_at_novell));
		const char* platform_a11y_dir = g_strconcat (before, after, NULL);

		memset (&platform_a11y_stat, 0, sizeof (platform_a11y_stat));
		stat (platform_a11y_dir, &platform_a11y_stat);
		g_free (platform_a11y_dir);
		g_free (before);
		g_free (after);
		moonlight_at_novell = NULL;
	}
}


const static char* platform_code_assemblies [] = {
	"mscorlib.dll",
	"System.dll",
	"System.Core.dll",
	"System.Net.dll",
	"System.Runtime.Serialization.dll",
	"System.ServiceModel.dll",
	"System.ServiceModel.Web.dll",
	"System.Windows.dll",
	"System.Windows.Browser.dll",
//	right now there are no [SecurityCritical] nor [SecuritySafeCritical] code inside the next two assemblies
//	so we'll treat them (at runtime) just like "application code" to reduce our attack surface
//	"System.Xml.dll",
//	"Microsoft.VisualBasic.dll",
#if DEBUG
	"jtr.dll",
#endif
};

static gboolean
determine_platform_image (const char *image_name)
{
	struct stat info;
	gchar *dir, *name;
	unsigned int i;
	struct stat the_platform_stat = platform_stat;
	gboolean a11y = FALSE;

	if (!image_name)
		return FALSE;

	/* all platform code resides in the same directory */
	dir = g_path_get_dirname (image_name);
	if (!dir || stat (dir, &info) != 0) {
		g_free (dir);
		return FALSE;
	}

	name = g_path_get_basename (image_name);
	if (!name) {
		g_free (dir);
		return FALSE;
	}
	
	if (g_ascii_strcasecmp (name, "MoonAtkBridge.dll") == 0) {
		the_platform_stat = platform_a11y_stat;
		a11y = TRUE;
	}

	/* we avoid comparing strings, e.g. /opt/mono/lib/moon versus /opt/mono//lib/moon */
	if ((the_platform_stat.st_mode != info.st_mode) ||
		(the_platform_stat.st_ino != info.st_ino) ||
		(the_platform_stat.st_dev != info.st_dev)) {
		g_free (dir);
		g_free (name);
		return FALSE;
	}
	g_free (dir);

	if (a11y == TRUE){
		g_free (name);
		return TRUE;
	}

	/* we know the names of every platform assembly, because we ship them */
	for (i = 0; i < G_N_ELEMENTS (platform_code_assemblies); i++) {
		if (g_ascii_strcasecmp (name, platform_code_assemblies [i]) == 0) {
			g_free (name);
			return TRUE;
		}
	}
	g_free (name);
	return FALSE;
}

#define DISABLE_SECURITY "MOON_DISABLE_SECURITY_DEBUG_ONLY"

void
security_enable_coreclr (const char *platform_dir)
{
#if DEBUG
	if (g_getenv (DISABLE_SECURITY) != NULL) {
		g_warning ("CORECLR was DISABLED using %s override", DISABLE_SECURITY);
	} else 
#endif
	if (g_path_is_absolute (platform_dir)) {
		memset (&platform_stat, 0, sizeof (platform_stat));

		if (stat (platform_dir, &platform_stat) == 0) {

			a11y_stat_init (platform_dir);

			mono_security_enable_core_clr ();
			mono_security_set_core_clr_platform_callback (determine_platform_image);
		}
	} else {
		g_warning ("CORECLR was DISABLED due to invalid, non-absolute, platform directory");
	}

	mono_assembly_setrootdir (platform_dir);
}

#else

void
security_enable_coreclr (const char *platform_dir)
{
}

#endif

