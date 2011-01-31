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

namespace Moonlight {

enum MoonConsentType {
	MOON_CONSENT_CLIPBOARD = 0,
	MOON_CONSENT_FULLSCREEN_PINNING = 1,
	MOON_CONSENT_CAPTURE = 2,
	// FIXME should this be here too?  MOON_CONSENT_ISOSTOR_QUOTA_INCREASE
	MOON_CONSENT_LAST
};

class Consent {
public:
	static char *GeneratePermissionConfigurationKey (MoonConsentType consent, const char *website);

	static const char *GetConsentName (MoonConsentType consent);
	static const char *GetConsentDescription (MoonConsentType consent);
	static MoonConsentType GetConsentType (const char *name);

	/* @GeneratePInvoke */
	static bool PromptUserFor (/* @MarshalAs=int */ MoonConsentType consent, bool *asked_user);
	static bool PromptUserFor (MoonConsentType consent, const char *question, const char *detail, const char *website, bool *asked_user);
};

};
#endif /* __MOON_CONSENT_H__ */
