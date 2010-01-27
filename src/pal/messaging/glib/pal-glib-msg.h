/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#ifndef MOON_PAL_GLIB_MSG_H
#define MOON_PAL_GLIB_MSG_H

#include "pal.h"

/* @Version=2 */
class MoonMessagingServiceGlib : public MoonMessagingService {
public:
	MoonMessagingServiceGlib ();
	virtual ~MoonMessagingServiceGlib ();

	virtual MoonMessageListener* CreateMessagingListener (const char *domain, const char *listenerName, MoonError *error);
	virtual MoonMessageSender* CreateMessagingSender (const char *listenerName, const char *listenerDomain, const char *domain, MoonError *error);
};

#endif /* MOON_PAL_GLIB_MSG_H */
