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

#include "plugin-domevents.h"

#ifdef DEBUG
#define d(x) x
#else
#define d(x)
#endif

#define STR_FROM_VARIANT(v) ((char *) NPVARIANT_TO_STRING (v).utf8characters)

NPClass DomEventListener::DomEventClass = {
	0,
	DomEventListener::Allocate,
	DomEventListener::_Deallocate, //Deallocate
	NULL, //Invalidate
	DomEventListener::_HasMethod, //HasMethod
	DomEventListener::_Invoke, //Invoke
	NULL, //DomEventListener::_InvokeDefault, //InvokeDefault
	DomEventListener::_HasProperty, //hasProperty
	DomEventListener::_GetProperty, //getProperty
	NULL, //setProperty
	NULL, //removeProperty
	NULL, //enumerate
	NULL, //construct
};

DomEventListener*
DomEventListener::Create(NPP npp, PluginInstance *plugin, const char *name, callback_dom_event* cb, gpointer ctx, NPObject *npobj) {
	DomEventListener *listener = reinterpret_cast<DomEventListener *> (MOON_NPN_CreateObject(npp, &DomEventClass));
	listener->parent = plugin;
	listener->callback = cb;
	listener->context = ctx;
	listener->target = npobj;
	listener->name = name;
	return listener;
}

void
DomEventListener::Attach ()
{
	NPVariant args[3];
	string_to_npvariant(name, &args[0]);
	OBJECT_TO_NPVARIANT(this, args[1]);
	BOOLEAN_TO_NPVARIANT(false, args[2]);

	NPVariant result;
	if (!MOON_NPN_Invoke(npp, target, NPID("addEventListener"), args, 3, &result))
		d(printf ("Error attaching event\n"));
}

void
DomEventListener::Detach ()
{
	NPVariant args[3];
	string_to_npvariant(name, &args[0]);
	OBJECT_TO_NPVARIANT(this, args[1]);
	BOOLEAN_TO_NPVARIANT(true, args[2]);
	NPVariant result;
	if (!MOON_NPN_Invoke(npp, target, NPID("removeEventListener"), args, 3, &result))
		d(printf ("Error detaching event\n"));

	callback = NULL;
	if (eventTarget)
		MOON_NPN_ReleaseObject (eventTarget);
	eventTarget = NULL;
}


NPObject*
DomEventListener::Allocate(NPP npp, NPClass *klass) {
	return new DomEventListener (npp);
}

void
DomEventListener::_Deallocate(NPObject *obj) {
	delete ((DomEventListener*)obj);
}

bool
DomEventListener::_HasMethod(NPObject *obj, NPIdentifier name) {
	return ((DomEventListener*)obj)->HasMethod (name);
}

bool
DomEventListener::_Invoke(NPObject *obj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result) {
	return ((DomEventListener*)obj)->Invoke (name, args, argCount, result);
}

bool
DomEventListener::_HasProperty(NPObject * obj, NPIdentifier name) {
	return ((DomEventListener*)obj)->HasProperty (name);
}

bool
DomEventListener::_GetProperty(NPObject *obj, NPIdentifier name, NPVariant *result) {
	return ((DomEventListener*)obj)->GetProperty (name, result);
}


int
DomEventListener::GetScreenX () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("screenX"), &res);
	int x = NPVARIANT_TO_INT32 (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

int
DomEventListener::GetScreenY () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("screenY"), &res);
	int x = NPVARIANT_TO_INT32 (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

int
DomEventListener::GetClientX () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("clientX"), &res);
	int x = NPVARIANT_TO_INT32 (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

int
DomEventListener::GetClientY () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("clientY"), &res);
	int x = NPVARIANT_TO_INT32 (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

bool
DomEventListener::GetAltKey () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("altKey"), &res);
	bool x = NPVARIANT_TO_BOOLEAN (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

bool
DomEventListener::GetCtrlKey () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("ctrlKey"), &res);
	bool x = NPVARIANT_TO_BOOLEAN (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

bool
DomEventListener::GetShiftKey () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("shiftKey"), &res);
	bool x = NPVARIANT_TO_BOOLEAN (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

int
DomEventListener::GetButton () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("button"), &res);
	int x = NPVARIANT_TO_INT32 (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

int
DomEventListener::GetKeyCode () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("keyCode"), &res);
	int x = NPVARIANT_TO_INT32 (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

int
DomEventListener::GetCharCode () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("charCode"), &res);
	int x = NPVARIANT_TO_INT32 (res);
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

int
DomEventListener::GetType () {
	NPVariant res;
	MOON_NPN_GetProperty (npp, eventObj, NPID("type"), &res);

	int x = Other;
	char* type = STR_FROM_VARIANT (res);
	if (!strncmp (type, "click", strlen("click")) ||
	    !strncmp (type, "dblclick", strlen("dblclick")) ||
	    !strncmp (type, "mouse", strlen("mouse")))
		x = Mouse;
	else if (!strncmp (type, "key", strlen("key")))
		x = Key;
	MOON_NPN_ReleaseVariantValue (&res);
	return x;
}

bool
DomEventListener::HasMethod (NPIdentifier name) {
	if (name == NPID ("handleEvent"))
		return true;
	return false;
}

bool
DomEventListener::HasProperty (NPIdentifier name) {
	return false;
}

bool
DomEventListener::GetProperty (NPIdentifier name, NPVariant *result) {
	return false;
}

bool
DomEventListener::Invoke (NPIdentifier name,
			const NPVariant *args,
			guint32 argCount,
			NPVariant *result)
{
	if (name != NPID ("handleEvent"))
		return false;

	if (callback == NULL)
		return true;

	NPVariant res;

	eventObj = NPVARIANT_TO_OBJECT(args[0]);
	MOON_NPN_RetainObject (eventObj);

	MOON_NPN_GetProperty (npp, eventObj, NPID("target"), &res);
	eventTarget = NPVARIANT_TO_OBJECT (res);
	MOON_NPN_RetainObject (eventTarget);
	MOON_NPN_ReleaseVariantValue (&res);

	int client_x, client_y, offset_x, offset_y, mouse_button, key_code, char_code;
	gboolean alt_key, ctrl_key, shift_key;
	client_x = client_y = offset_x = offset_y = mouse_button = 0;
	alt_key = ctrl_key = shift_key = FALSE;
	key_code = char_code = 0;

	int type = GetType ();

	if (type == Mouse) {
		client_x = GetClientX ();
		client_y = GetClientY ();
		offset_x = GetScreenX ();
		offset_y = GetScreenY ();
		offset_x -= client_x;
		offset_y -= client_y;
		mouse_button = GetButton ();
		alt_key = GetAltKey ();
		ctrl_key = GetCtrlKey ();
		shift_key = GetShiftKey ();
	} else if (type == Key) {
		key_code = GetKeyCode ();
		char_code = GetCharCode ();
		if (char_code == 0 && key_code != 0)
			char_code = key_code;
		alt_key = GetAltKey ();
		ctrl_key = GetCtrlKey ();
		shift_key = GetShiftKey ();
	}

	MOON_NPN_GetProperty (npp, eventObj, NPID("type"), &res);
	char *t = STRDUP_FROM_VARIANT(res);
	MOON_NPN_ReleaseVariantValue (&res);

	callback (context, t, client_x, client_y, offset_x, offset_y,
		alt_key, ctrl_key, shift_key, mouse_button, key_code, char_code, eventTarget);


	return true;
}
