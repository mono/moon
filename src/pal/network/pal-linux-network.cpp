/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* 
 * pal-linux-network.cpp:
 *
 * Copyright 2010 Novell, Inc.  (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include "config.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <errno.h>

#include "pal.h"
#include "runtime.h"
#include "pal-linux-network.h"
#include "dbus/pal-dbus-network.h"

using namespace Moonlight;

static bool
network_available (void)
{
	struct ifaddrs *ifaces, *iface;
	bool avail = false;
	
	if (getifaddrs (&ifaces) == -1)
		return false;
	
	for (iface = ifaces; iface && !avail; iface = iface->ifa_next) {
		struct sockaddr_in6 *sin6 = (struct sockaddr_in6 *) iface->ifa_addr;
		struct sockaddr_in *sin = (struct sockaddr_in *) iface->ifa_addr;
		//char ipaddr[128];
		
		switch (iface->ifa_addr->sa_family) {
		case AF_INET6:
			/* An ipv6 address is 16 bytes, if only one of the
			 * last 2 is set, then it is assumed to be loopback,
			 * else it is presumed not to be */
			//inet_ntop (AF_INET6, sin6->sin6_addr.s6_addr, ipaddr, sizeof (ipaddr));
			for (int i = 0; i < 14 && !avail; i++)
				avail = sin6->sin6_addr.s6_addr[i] != 0;
			break;
		case AF_INET:
			/* An ipv4 address is 4 bytes, if first byte is 127
			 * then it is assumed to be loopback, else it is
			 * presumed not to be */
			//inet_ntop (AF_INET, &sin->sin_addr.s_addr, ipaddr, sizeof (ipaddr));
			if ((sin->sin_addr.s_addr & 0xff) != 127)
				avail = true;
			break;
		default:
			break;
		}
	}
	
	freeifaddrs (ifaces);
	
	return avail;
}

static bool
poll_network_state (gpointer user_data)
{
	MoonNetworkServiceLinux *nserv = (MoonNetworkServiceLinux *) user_data;
	
	nserv->PollNetworkState ();
	
	return true;
}

MoonNetworkServiceLinux::MoonNetworkServiceLinux ()
{
#if PAL_DBUS_NETWORKAVAILABILITY
	dbus_network_service = new MoonNetworkServiceDbus ();
#else
	dbus_network_service = NULL;
#endif
	
	callback = NULL;
	user_data = NULL;
	using_dbus = false;
	is_avail = false;
	idle = 0;
}

MoonNetworkServiceLinux::~MoonNetworkServiceLinux ()
{
	MoonWindowingSystem *winsys;
	
	delete dbus_network_service;
	
	if (idle != 0 && (winsys = Runtime::GetWindowingSystem ())) {
		winsys->RemoveIdle (idle);
		idle = 0;
	}
}

bool
MoonNetworkServiceLinux::SetNetworkStateChangedCallback (MoonCallback callback, gpointer data)
{
	MoonWindowingSystem *winsys;
	
	if (dbus_network_service && dbus_network_service->SetNetworkStateChangedCallback (callback, data)) {
		using_dbus = true;
		return true;
	}
	
	if (!(winsys = Runtime::GetWindowingSystem ()))
		return false;
	
	idle = winsys->AddIdle (poll_network_state, this);
	is_avail = network_available ();
	this->callback = callback;
	this->user_data = data;
	
	return true;
}

bool
MoonNetworkServiceLinux::GetIsNetworkAvailable ()
{
	if (using_dbus)
		return dbus_network_service->GetIsNetworkAvailable ();
	
	return network_available ();
}

void
MoonNetworkServiceLinux::PollNetworkState ()
{
	if (is_avail != network_available ()) {
		is_avail = !is_avail;
		
		if (callback)
			callback (this, user_data);
	}
}
