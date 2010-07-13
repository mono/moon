/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * pal-dbus-network.h: dbus networkmanager network notifications
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#ifndef MOON_PAL_DBUS_NETWORK_H
#define MOON_PAL_DBUS_NETWORK_H

#include "pal.h"
#include "dbus/dbus-glib.h"

namespace Moonlight {

/* @Version=2 */
class MoonNetworkServiceDbus : public MoonNetworkService {
public:
	MoonNetworkServiceDbus ();
	virtual ~MoonNetworkServiceDbus ();

	virtual void SetNetworkStateChangedCallback (MoonCallback callback, gpointer data);

	virtual bool GetIsNetworkAvailable ();

private:
	MoonCallback callback;
	gpointer callback_data;

	DBusGConnection *dbus_connection;
	DBusGProxy *nm_proxy;
	DBusGProxy *prop_proxy;

	static void state_changed_handler (gpointer sender, guint state, gpointer data);
};

};
#endif /* MOON_PAL_DBUS_NETWORK_H */
