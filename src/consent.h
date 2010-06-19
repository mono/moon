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

#ifndef __MOON_CONSENT_H__
#define __MOON_CONSENT_H__

#include <glib.h>

typedef enum {
	MOON_CONSENT_CLIPBOARD,
	MOON_CONSENT_FULLSCREEN_PINNING,
	MOON_CONSENT_VIDEO_CAPTURE,
	MOON_CONSENT_AUDIO_CAPTURE,
	// FIXME should this be here too?  MOON_CONSENT_ISOSTOR_QUOTA_INCREASE
} MoonConsentType;

G_BEGIN_DECLS

class Consent {
public:
	/* @GeneratePInvoke */
	static bool PromptUserFor (MoonConsentType consent);
	static bool PromptUserFor (MoonConsentType consent, const char *question, const char *detail, const char *website);
};

G_END_DECLS

#endif /* __MOON_CONSENT_H__ */
