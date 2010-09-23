/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */

#include "config.h"

#include "pal-dbus-network.h"

using namespace Moonlight;

MoonNetworkServiceDbus::MoonNetworkServiceDbus ()
{
	this->nm_proxy = NULL;
	this->prop_proxy = NULL;

	this->callback = NULL;
	this->callback_data = NULL;

	dbus_connection = dbus_g_bus_get (DBUS_BUS_SYSTEM, NULL);
	if (!dbus_connection) {
		g_warning ("failed to get system dbus");
		return;
	}
}

MoonNetworkServiceDbus::~MoonNetworkServiceDbus ()
{
	if (nm_proxy)
		g_object_unref (nm_proxy);
	if (prop_proxy)
		g_object_unref (prop_proxy);
	if (dbus_connection)
		dbus_g_connection_unref (dbus_connection);
}

static bool
network_manager_get_state (DBusGProxy *prop_proxy, GValue *state)
{
	GError *error = NULL;
	
	return dbus_g_proxy_call (prop_proxy, "Get", &error,
				  G_TYPE_STRING, "org.freedesktop.NetworkManager",
				  G_TYPE_STRING, "State",
				  G_TYPE_INVALID,
				  G_TYPE_VALUE, state,
				  G_TYPE_INVALID);
}

bool
MoonNetworkServiceDbus::SetNetworkStateChangedCallback (MoonCallback callback, gpointer data)
{
	GValue state = { 0, };
	GError *error = NULL;
	
	if (!dbus_connection)
		return false;
	
	if (nm_proxy == NULL) {
		nm_proxy = dbus_g_proxy_new_for_name (dbus_connection,
						      "org.freedesktop.NetworkManager",
						      "/org/freedesktop/NetworkManager",
						      "org.freedesktop.NetworkManager");

		if (!nm_proxy) {
			g_warning ("failed to get proxy for network manager");
			return false;
		}

		prop_proxy = dbus_g_proxy_new_from_proxy (nm_proxy,
							  "org.freedesktop.DBus.Properties",
							  "/org/freedesktop/NetworkManager");

		if (!prop_proxy) {
			g_warning ("failed to get proxy for network manager properties");
			return false;
		}
		
		// make sure we can get the state from network manager
		if (!network_manager_get_state (prop_proxy, &state))
			return false;
		
		// attach to dbus
		dbus_g_proxy_add_signal (nm_proxy, "StateChanged", G_TYPE_UINT, G_TYPE_INVALID);
		
		dbus_g_proxy_connect_signal (nm_proxy, "StateChanged",
					     G_CALLBACK (state_changed_handler),
					     this, NULL);
	}
	
	this->callback = callback;
	this->callback_data = data;
	
	return prop_proxy != NULL;
}

void
MoonNetworkServiceDbus::state_changed_handler (gpointer sender, guint state, gpointer data)
{
	MoonNetworkServiceDbus* dbus = (MoonNetworkServiceDbus*)data;
	
	if (dbus->callback)
		dbus->callback (dbus, dbus->callback_data);
}

bool
MoonNetworkServiceDbus::GetIsNetworkAvailable ()
{
	GValue state = { 0, };
	
	if (!network_manager_get_state (prop_proxy, &state))
		return false;
	
	return g_value_get_uint (&state) == 3; /* NM_STATE_CONNECTED */
}
