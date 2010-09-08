/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-linux-network.h: network notifications for for linux
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_PAL_LINUX_NETWORK_H
#define MOON_PAL_LINUX_NETWORK_H

#include "pal.h"

#include "network/dbus/pal-dbus-network.h"

namespace Moonlight {

/* @Version=2 */
class MoonNetworkServiceLinux : public MoonNetworkService {
public:
	MoonNetworkServiceLinux ();
	virtual ~MoonNetworkServiceLinux ();

	virtual bool SetNetworkStateChangedCallback (MoonCallback callback, gpointer data);

	virtual bool GetIsNetworkAvailable ();

private:
	bool using_dbus;

	MoonCallback callback;
	gpointer callback_data;

	static void state_changed_handler (gpointer sender, guint state, gpointer data);

	
	MoonNetworkServiceDbus *dbus_network_service;
};

};
#endif /* MOON_PAL_LINUX_NETWORK_H */
