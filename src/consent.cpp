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

static const char *consent_name[] = {
	"clipboard", "fullscreen", "videocapture", "audiocapture"
};

static char*
generate_key (MoonConsentType consent, const char *website)
{
	return g_strdup_printf ("%s-%s", website, consent_name[consent]);
}

bool
Consent::PromptUserFor (MoonConsentType consent)
{
	Surface *surface = Deployment::GetCurrent ()->GetSurface ();
	const char *source = surface->GetSourceLocation ();

	Uri *uri = new Uri ();
	uri->Parse (source);

	char *website = ((uri->GetPort () > 0)
			 ? g_strdup_printf ("%s://%s:%d",
					    uri->GetScheme(),
					    uri->GetHost(),
					    uri->GetPort())
			 : g_strdup_printf ("%s://%s",
					    uri->GetScheme(),
					    uri->GetHost()));

	const char *question, *detail;

	switch (consent) {
	case MOON_CONSENT_CLIPBOARD:
		question = "Do you want to allow this application to access your clipboard?";
		detail = "If you allow this, the application can copy data to and from the Clipboard as long as the application is running.";
		break;
	case MOON_CONSENT_FULLSCREEN_PINNING:
		question = "[fullscreen question here]"; // FIXME
		detail = "[fullscreen detail here]"; // FIXME
		break;
	case MOON_CONSENT_VIDEO_CAPTURE:
		question = "Do you want to allow this application to access your webcam?";
		detail = "If you allow this, the application can capture still frames and video from your webcam as long as the application is running.";
		break;
	case MOON_CONSENT_AUDIO_CAPTURE:
		question = "Do you want to allow this application to access your microphone?";
		detail = "If you allow this, the application can capture audio from your microphone as long as the application is running.";
		break;
	}

	return PromptUserFor (consent, question, detail, website);
}

bool
Consent::PromptUserFor (MoonConsentType consent, const char *question, const char *detail, const char *website)
{
	bool remember = false;
	bool rv = false; 

	MoonlightConfiguration configuration;

	char *config_key = generate_key (consent, website);
	if (configuration.HasKey ("Permissions", config_key)) {
		rv = configuration.GetBooleanValue ("Permissions", config_key);
		g_free (config_key);
		return rv;
	}


	MoonWindowingSystem *windowing_system = runtime_get_windowing_system ();

	rv = windowing_system->ShowConsentDialog (question,
						  detail,
						  website,
						  &remember);

	if (remember) {
		configuration.SetBooleanValue ("Permissions", config_key, rv);
		configuration.Save ();
	}

	g_free (config_key);
	return rv;
}
