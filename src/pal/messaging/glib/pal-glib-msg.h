/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-glib-msg.h: giochannel + unix domain socket based local messaging
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_PAL_GLIB_MSG_H
#define MOON_PAL_GLIB_MSG_H

#include "pal.h"

namespace Moonlight {

/* @Version=2 */
class MoonMessagingServiceGlib : public MoonMessagingService {
public:
	MoonMessagingServiceGlib ();
	virtual ~MoonMessagingServiceGlib ();

	virtual MoonMessageListener* CreateMessagingListener (const char *domain, const char *listenerName, MoonError *error);
	virtual MoonMessageSender* CreateMessagingSender (const char *listenerName, const char *listenerDomain, const char *domain, MoonError *error);
};

};
#endif /* MOON_PAL_GLIB_MSG_H */
