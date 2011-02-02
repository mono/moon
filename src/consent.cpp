/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include <config.h>

#include "consent.h"
#include "uri.h"
#include "pal.h"
#include "runtime.h"
#include "deployment.h"
#include "moonlightconfiguration.h"

namespace Moonlight {

static const char *consent_name[] = {
	"clipboard",
	"fullscreen",
	"capture"
};

static const char *consent_description[] = {
	"Clipboard",
	"Full-screen: stay full-screen when unfocused",
	"Webcam and Microphone"
};

char*
Consent::GeneratePermissionConfigurationKey (MoonConsentType consent, const char *website)
{
	const char *name = GetConsentName (consent);
	if (name == NULL)
		return NULL;
	return g_strdup_printf ("%s-%s", website, name);
}

const char *
Consent::GetConsentName (MoonConsentType consent)
{
	if (consent < 0 || consent >= MOON_CONSENT_LAST)
		return NULL;

	return consent_name[consent];
}

const char *
Consent::GetConsentDescription (MoonConsentType consent)
{
	if (consent < 0 || consent >= MOON_CONSENT_LAST)
		return NULL;

	return consent_description[consent];
}

MoonConsentType
Consent::GetConsentType (const char *name)
{
	for (int i = 0; i < MOON_CONSENT_LAST; i ++)
		if (!g_ascii_strcasecmp (name, consent_name[i]))
			return (MoonConsentType)i;

	return (MoonConsentType)-1;
}

bool
Consent::PromptUserFor (MoonConsentType consent, bool *asked_user)
{
	bool result;
	Surface *surface = Deployment::GetCurrent ()->GetSurface ();
	const Uri *uri = surface->GetSourceLocation ();
	char *website;
	const char *question, *detail;

	if (asked_user)
		*asked_user = false;

	if (uri->GetPort () > 0) {
		website = g_strdup_printf ("%s://%s:%d", uri->GetScheme(), uri->GetHost(), uri->GetPort());
	} else {
		/* file:// uri */
		website = g_strdup (uri->ToString ());
		/* We need to strip off the filename */
		char *last_sep = strrchr (website, '/');
		if (last_sep != NULL) {
			*(last_sep + 1) = 0;
		}
	}

	switch (consent) {
	case MOON_CONSENT_CLIPBOARD:
		question = "Do you want to allow this application to access your clipboard?";
		detail = "If you allow this, the application can copy data to and from the Clipboard as long as the application is running.";
		break;
	case MOON_CONSENT_FULLSCREEN_PINNING:
		question = "Do you want to allow this application to stay in full-screen mode?";
		detail = "If you you choose No, the application will still enter full-screen mode, but will exit full-screen mode when it loses focus";
		break;
	case MOON_CONSENT_CAPTURE:
		question = "Do you want to allow this application to access your webcam and microphone?";
		detail = "If you allow this, the application can capture video and audio as long as the application is running.";
		break;
	case MOON_CONSENT_LAST:
		// should't be reached...
		question = detail = NULL;
		break;
	}

	result = PromptUserFor (consent, question, detail, website, asked_user);

	g_free (website);

	return result;
}

bool
Consent::PromptUserFor (MoonConsentType consent, const char *question, const char *detail, const char *website, bool *asked_user)
{
	bool remember = false;
	bool rv = false; 

	MoonlightConfiguration configuration;

	char *config_key = GeneratePermissionConfigurationKey (consent, website);
	if (configuration.HasKey ("Permissions", config_key)) {
		rv = configuration.GetBooleanValue ("Permissions", config_key);
		g_free (config_key);
		return rv;
	}


	MoonWindowingSystem *windowing_system = Runtime::GetWindowingSystem ();

	rv = windowing_system->ShowConsentDialog (question,
						  detail,
						  website,
						  &remember);

	if (asked_user)
		*asked_user = true;

	if (remember) {
		configuration.SetBooleanValue ("Permissions", config_key, rv);
		configuration.Save ();
	}

	g_free (config_key);
	return rv;
}

};
