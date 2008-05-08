// define this here so that protypes.h isn't included (and doesn't
// muck with our npapi.h)
#define NO_NSPR_10_SUPPORT

#include "plugin.h"

#include "ff3-bridge.h"

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

#define STR_FROM_VARIANT(v) ((char *) NPVARIANT_TO_STRING (v).utf8characters)

BrowserBridge* CreateBrowserBridge ()
{
	return new FF3BrowserBridge ();
}

#define GECKO_SYM(x) CONCAT(FF3,x)
#include "../ff2/firefox-browserhttp.inc"

class FF3DomEventWrapper : public nsIDOMEventListener {

	NS_DECL_ISUPPORTS
	NS_DECL_NSIDOMEVENTLISTENER

 public:

	FF3DomEventWrapper () {
		callback = NULL;

		NS_INIT_ISUPPORTS ();
	}

	callback_dom_event *callback;
	nsCOMPtr<nsIDOMEventTarget> target;
};

NS_IMPL_ISUPPORTS1(FF3DomEventWrapper, nsIDOMEventListener)

NS_IMETHODIMP
FF3DomEventWrapper::HandleEvent (nsIDOMEvent *aDOMEvent)
{
	int client_x, client_y, offset_x, offset_y, mouse_button;
	gboolean alt_key, ctrl_key, shift_key;
	nsString str_event;
	
	aDOMEvent->GetType (str_event);

	client_x = client_y = offset_x = offset_y = mouse_button = 0;
	alt_key = ctrl_key = shift_key = FALSE;

	nsCOMPtr<nsIDOMMouseEvent> mouse_event = do_QueryInterface (aDOMEvent);
	if (mouse_event != nsnull) {
		int screen_x, screen_y;

		mouse_event->GetScreenX (&screen_x);
		mouse_event->GetScreenY (&screen_y);

		mouse_event->GetClientX (&client_x);
		mouse_event->GetClientY (&client_y);

		offset_x = screen_x - client_x;
		offset_y = screen_y - client_y;

		mouse_event->GetAltKey (&alt_key);
		mouse_event->GetCtrlKey (&ctrl_key);
		mouse_event->GetShiftKey (&shift_key);

		PRUint16 umouse_button;
		mouse_event->GetButton (&umouse_button);
		mouse_button = umouse_button;
	}

	callback (strdup (NS_ConvertUTF16toUTF8 (str_event).get ()), client_x, client_y, offset_x, offset_y,
			alt_key, ctrl_key, shift_key, mouse_button);

	return NS_OK;
}

static nsCOMPtr<nsIDOMDocument>
ff3_get_dom_document (NPP npp)
{
	nsCOMPtr<nsIDOMWindow> dom_window;
	NPN_GetValue (npp, NPNVDOMWindow, static_cast<nsIDOMWindow **>(getter_AddRefs(dom_window)));
	if (!dom_window) {
		d (printf ("No DOM window available\n"));
		return NULL;
	}

	nsCOMPtr<nsIDOMDocument> dom_document;
	dom_window->GetDocument (getter_AddRefs (dom_document));
	if (dom_document == nsnull) {
		d(printf ("No DOM document available\n"));
		return NULL;
	}

	return dom_document;
}

const char*
FF3BrowserBridge::HtmlElementGetText (NPP npp, const char *element_id)
{
	nsresult rv = NS_OK;

	nsCOMPtr<nsIDOMDocument> document;
	document = ff3_get_dom_document (npp);
	if (!document)
		return NULL;

	nsString ns_id = NS_ConvertUTF8toUTF16 (element_id, strlen (element_id));
	nsCOMPtr<nsIDOMElement> element;
	rv = document->GetElementById (ns_id, getter_AddRefs (element));
	if (NS_FAILED (rv) || element == NULL)
		return NULL;

	nsCOMPtr<nsIDOMDocument> owner;
	element->GetOwnerDocument (getter_AddRefs (owner));
	
	nsCOMPtr<nsIDOMDocumentRange> doc_range = do_QueryInterface (owner);
	if (!doc_range)
		return NULL;

	nsCOMPtr<nsIDOMRange> range;
	doc_range->CreateRange (getter_AddRefs (range));
	if (!range)
		return NULL;

	range->SelectNodeContents (element);

	nsString text;
	range->ToString (text);
	return g_strdup (NS_ConvertUTF16toUTF8 (text).get ());
}

gpointer
FF3BrowserBridge::HtmlObjectAttachEvent (NPP npp, NPObject *npobj, const char *name, callback_dom_event cb)
{
	nsresult rv;
	NPVariant npresult;
	NPIdentifier id_identifier = NPN_GetStringIdentifier ("id");
	nsCOMPtr<nsISupports> item;

	NPN_GetProperty (npp, npobj, id_identifier, &npresult);

	if (NPVARIANT_IS_STRING (npresult) && strlen (STR_FROM_VARIANT (npresult)) > 0) {
		NPString np_id = NPVARIANT_TO_STRING (npresult);

		nsString ns_id = NS_ConvertUTF8toUTF16 (np_id.utf8characters, strlen (np_id.utf8characters));
		nsCOMPtr<nsIDOMDocument> dom_document = ff3_get_dom_document (npp);

		nsCOMPtr<nsIDOMElement> element;
		rv = dom_document->GetElementById (ns_id, getter_AddRefs (element));
		if (NS_FAILED (rv) || element == nsnull) {
			return NULL;
		}

		item = element;
	} else {
		NPObject *window = NULL;
		NPIdentifier document_identifier = NPN_GetStringIdentifier ("document");

		NPN_GetValue (npp, NPNVWindowNPObject, &window);

		if (npobj == window) {
			NPN_GetValue (npp, NPNVDOMWindow, static_cast<nsISupports **>(getter_AddRefs (item)));
		} else {
			NPVariant docresult;
			NPN_GetProperty (npp, window, document_identifier, &docresult);

			if (npobj == NPVARIANT_TO_OBJECT (docresult)) {
				item = ff3_get_dom_document (npp);
			} else {
				const char *temp_id = "__moonlight_temp_id";
				NPVariant npvalue;

				string_to_npvariant (temp_id, &npvalue);
				NPN_SetProperty (npp, npobj, id_identifier, &npvalue);
				NPN_ReleaseVariantValue (&npvalue);

				nsString ns_id = NS_ConvertUTF8toUTF16 (temp_id, strlen (temp_id));
				nsCOMPtr<nsIDOMDocument> dom_document = ff3_get_dom_document (npp);

				nsCOMPtr<nsIDOMElement> element;
				dom_document->GetElementById (ns_id, getter_AddRefs (element));
				if (element == nsnull) {
					d(printf ("Unable to find temp_id element\n"));
					return NULL;
				}

				item = element;

				// reset to it's original empty value
				NPN_SetProperty (npp, npobj, id_identifier, &npresult);
			}
		}
	}

	nsCOMPtr<nsIDOMEventTarget> target = do_QueryInterface (item);

	FF3DomEventWrapper *wrapper = new FF3DomEventWrapper ();
	wrapper->callback = cb;
	wrapper->target = target;

	rv = target->AddEventListener (NS_ConvertUTF8toUTF16 (name, strlen (name)), wrapper, PR_TRUE);

	return wrapper;
}

void
FF3BrowserBridge::HtmlObjectDetachEvent (NPP instance, const char *name, gpointer listener_ptr)
{
	FF3DomEventWrapper *wrapper = (FF3DomEventWrapper *) listener_ptr;

	wrapper->target->RemoveEventListener (NS_ConvertUTF8toUTF16 (name, strlen (name)), wrapper, PR_TRUE);
}

BrowserHttpRequest*
FF3BrowserBridge::CreateBrowserHttpRequest (const char *method, const char *uri)
{
	return new FF3BrowserHttpRequest (method, uri);
}
