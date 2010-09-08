/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>

#include "pal-linux-network.h"
#include "dbus/pal-dbus-network.h"

using namespace Moonlight;

MoonNetworkServiceLinux::MoonNetworkServiceLinux ()
{
#if PAL_DBUS_NETWORKAVAILABILITY
	dbus_network_service = new MoonNetworkServiceDbus ();
#else
	dbus_network_service = NULL;
#endif

	using_dbus = false;
}

MoonNetworkServiceLinux::~MoonNetworkServiceLinux ()
{
	delete dbus_network_service;
}

bool
MoonNetworkServiceLinux::SetNetworkStateChangedCallback (MoonCallback callback, gpointer data)
{
	if (dbus_network_service && dbus_network_service->SetNetworkStateChangedCallback (callback, data)) {
		using_dbus = true;
		return true;
	}

	g_warning ("add fallback for poll networking initialization");

	return true;
}

bool
MoonNetworkServiceLinux::GetIsNetworkAvailable ()
{
	if (using_dbus && dbus_network_service->GetIsNetworkAvailable ())
		return true;

	g_warning ("add fallback for poll networking property getter");

	return true;
}
