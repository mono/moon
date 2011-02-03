#ifndef FF_COMMON
#define FF_COMMON

// define this here so that protypes.h isn't included (and doesn't
// muck with our npapi.h)
#define NO_NSPR_10_SUPPORT

#include "moonbuild.h"
#include "plugin-class.h"

#include <nsCOMPtr.h>
#include <nsIDOMElement.h>
#include <nsIDOMRange.h>
#include <nsIDOMDocumentRange.h>
#include <nsIDOMDocument.h>
#include <nsIDOMWindow.h>
#include <nsStringAPI.h>

// Events
#include <nsIDOMEvent.h>
#include <nsIDOMMouseEvent.h>
#include <nsIDOMEventTarget.h>
#include <nsIDOMEventListener.h>


#ifdef DEBUG
#define DEBUG_WARN_NOTIMPLEMENTED(x) printf ("not implemented: (%s)\n" G_STRLOC, x)
#define d(x) x
#else
#define DEBUG_WARN_NOTIMPLEMENTED(x)
#define d(x)
#endif

// debug helper
#define ds(x)

#define STR_FROM_VARIANT(v) ((char *) NPVARIANT_TO_STRING (v).utf8characters)

static const MoonNameIdMapping
dom_event_mapping[] = {
	{ "stoppropagation", MoonId_StopPropagation },
	{ "preventdefault", MoonId_PreventDefault },
	{ "detail", MoonId_Detail}
};

struct FFDomEvent : MoonlightObject {
	FFDomEvent (NPP instance) : MoonlightObject (instance) { }

	virtual bool GetProperty (int id, NPIdentifier unmapped, NPVariant *result);
	virtual bool Invoke (int id, NPIdentifier name,
			     const NPVariant *args, uint32_t argCount, NPVariant *result);

	nsIDOMEvent * event;
};

struct FFDomEventType : MoonlightObjectType {
	FFDomEventType ();
};

extern FFDomEventType *FFDomEventClass;

#endif
