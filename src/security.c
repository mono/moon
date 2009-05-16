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
	"System.Xml.dll",
	"Microsoft.VisualBasic.dll",
#if DEBUG
	"jtr.dll",
#endif
};

static gboolean
determine_platform_image (const char *image_name)
{
	struct stat info;
	gchar *dir, *name;
	int i;

	if (!image_name)
		return FALSE;

	/* all platform code resides in the same directory */
	dir = g_path_get_dirname (image_name);
	if (!dir || stat (dir, &info) != 0) {
		g_free (dir);
		return FALSE;
	}

	/* we avoid comparing strings, e.g. /opt/mono/lib/moon versus /opt/mono//lib/moon */
	if ((platform_stat.st_mode != info.st_mode) ||
		(platform_stat.st_ino != info.st_ino) ||
		(platform_stat.st_dev != info.st_dev)) {
		g_free (dir);
		return FALSE;
	}
	g_free (dir);

	/* we know the names of every platform assembly, because we ship them */
	name = g_path_get_basename (image_name);
	if (!name)
		return FALSE;

	for (i = 0; i < G_N_ELEMENTS (platform_code_assemblies); i++) {
		if (g_ascii_strcasecmp (name, platform_code_assemblies [i]) == 0) {
			g_free (name);
			return TRUE;
		}
	}
	g_free (name);
	return FALSE;
}

void
security_enable_coreclr (const char *platform_dir)
{
	if (g_getenv ("MOON_DISABLE_SECURITY_PREVIEW_04") != NULL) {
		g_warning ("CORECLR was DISABLED using MOON_DISABLE_SECURITY_PREVIEW_01 override");
		g_warning ("this disables both code verification and metadata verification on code\n"
			   "downloaded from untrusted sources, and therefore opens up your machine\n"
			   "to a wide variety of attack vectors. Don't do this unless you know what\n"
			   "you're doing!");
	} else if (g_path_is_absolute (platform_dir)) {
		memset (&platform_stat, 0, sizeof (platform_stat));
		if (stat (platform_dir, &platform_stat) == 0) {
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

