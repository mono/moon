/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * ff36-dom.cpp: Firefox 3.6.x DOM wrapper
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2010 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

#ifndef __DOM_EVENTS_H__
#define __DOM_EVENTS_H__

#include "plugin-class.h"

typedef void callback_dom_event (gpointer context, char *name, int client_x, int client_y, int offset_x, int offset_y, gboolean alt_key,
				 gboolean ctrl_key, gboolean shift_key, int mouse_button,
				 int key_code, int char_code,
				 gpointer domEvent);


enum EventType {
	Mouse,
	Key,
	Other,
};

class DomEventListener : public NPObject {

 public:
	static NPClass DomEventClass;
	static DomEventListener *Create(NPP npp, PluginInstance *plugin, const char *name, callback_dom_event* cb, gpointer ctx, NPObject *npobj);
	static NPObject *Allocate(NPP npp, NPClass *klass);
	static void _Deallocate(NPObject *obj);
	static bool _HasMethod(NPObject *obj, NPIdentifier name);
	static bool _Invoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
	static bool _HasProperty(NPObject * obj, NPIdentifier name);
	static bool _GetProperty(NPObject *obj, NPIdentifier name, NPVariant *result);

	DomEventListener () : callback (0) {}

	DomEventListener (NPP instance) : npp (instance),
		parent(0), callback(0), context(0),
		target(0), eventObj(0), name(0) {}


	~DomEventListener () {}

	void Attach ();
	void Detach ();

	bool HasMethod (NPIdentifier unmapped);
	bool Invoke (NPIdentifier name,
		const NPVariant *args,
		guint32 argCount,
		NPVariant *result);
	bool HasProperty (NPIdentifier name);
	bool GetProperty (NPIdentifier name, NPVariant *result);


	int GetScreenX ();
	int GetScreenY ();
	int GetClientX ();
	int GetClientY ();
	bool GetAltKey ();
	bool GetCtrlKey ();
	bool GetShiftKey ();
	int GetButton ();
	int GetKeyCode ();
	int GetCharCode ();
	int GetType ();
	NPObject* GetTarget ();

	NPP npp;
	PluginInstance *parent;
	callback_dom_event *callback;
	gpointer context;
	NPObject *target;
	NPObject *eventObj;
	NPObject *eventTarget;
	const char *name;
};

#endif