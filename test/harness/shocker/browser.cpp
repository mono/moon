/*
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *
 * Author:
 *   Jackson Harper (jackson@ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 *
 * browser.cpp: A wrapper around some of Mozilla browser functions,
 *              ie functions that the plugin calls on the browser.
 *
 */

#include "browser.h"



Browser* Browser::instance = NULL;


Browser::Browser (NPNetscapeFuncs *mozilla_funcs)
{
	browser_funcs.size                    = mozilla_funcs->size;
	browser_funcs.version                 = mozilla_funcs->version;
	browser_funcs.geturlnotify            = mozilla_funcs->geturlnotify;
	browser_funcs.geturl                  = mozilla_funcs->geturl;
	browser_funcs.posturlnotify           = mozilla_funcs->posturlnotify;
	browser_funcs.posturl                 = mozilla_funcs->posturl;
	browser_funcs.requestread             = mozilla_funcs->requestread;
	browser_funcs.newstream               = mozilla_funcs->newstream;
	browser_funcs.write                   = mozilla_funcs->write;
	browser_funcs.destroystream           = mozilla_funcs->destroystream;
	browser_funcs.status                  = mozilla_funcs->status;
	browser_funcs.uagent                  = mozilla_funcs->uagent;
	browser_funcs.memalloc                = mozilla_funcs->memalloc;
	browser_funcs.memfree                 = mozilla_funcs->memfree;
	browser_funcs.memflush                = mozilla_funcs->memflush;
	browser_funcs.reloadplugins           = mozilla_funcs->reloadplugins;
	browser_funcs.getJavaEnv              = mozilla_funcs->getJavaEnv;
	browser_funcs.getJavaPeer             = mozilla_funcs->getJavaPeer;
	browser_funcs.getvalue                = mozilla_funcs->getvalue;
	browser_funcs.setvalue                = mozilla_funcs->setvalue;
	browser_funcs.invalidaterect          = mozilla_funcs->invalidaterect;
	browser_funcs.invalidateregion        = mozilla_funcs->invalidateregion;
	browser_funcs.forceredraw             = mozilla_funcs->forceredraw;

	if (mozilla_funcs->version >= NP_VERSION_HAS_RUNTIME) {
		browser_funcs.getstringidentifier    = mozilla_funcs->getstringidentifier;
		browser_funcs.getstringidentifiers   = mozilla_funcs->getstringidentifiers;
		browser_funcs.getintidentifier       = mozilla_funcs->getintidentifier;
		browser_funcs.identifierisstring     = mozilla_funcs->identifierisstring;
		browser_funcs.utf8fromidentifier     = mozilla_funcs->utf8fromidentifier;
		browser_funcs.intfromidentifier      = mozilla_funcs->intfromidentifier;
		browser_funcs.createobject           = mozilla_funcs->createobject;
		browser_funcs.retainobject           = mozilla_funcs->retainobject;
		browser_funcs.releaseobject          = mozilla_funcs->releaseobject;
		browser_funcs.invoke                 = mozilla_funcs->invoke;
		browser_funcs.invokeDefault          = mozilla_funcs->invokeDefault;
		browser_funcs.evaluate               = mozilla_funcs->evaluate;
		browser_funcs.getproperty            = mozilla_funcs->getproperty;
		browser_funcs.setproperty            = mozilla_funcs->setproperty;
		browser_funcs.removeproperty         = mozilla_funcs->removeproperty;
		browser_funcs.hasproperty            = mozilla_funcs->hasproperty;
		browser_funcs.hasmethod              = mozilla_funcs->hasmethod;
		browser_funcs.releasevariantvalue    = mozilla_funcs->releasevariantvalue;
		browser_funcs.setexception           = mozilla_funcs->setexception;
	}
}

void
Browser::Initialize (NPNetscapeFuncs *browser_funcs)
{
	Browser::instance = new Browser (browser_funcs);
}

Browser*
Browser::Instance ()
{
	if (!Browser::instance)
		printf ("Warning, attempting to access the browser instance before it has been initialized. Things are going to crash\n");

	return Browser::instance;
}

NPObject*
Browser::CreateObject (NPP npp, NPClass *klass)
{
	return browser_funcs.createobject (npp, klass);
}

NPObject*
Browser::RetainObject (NPObject* npobj)
{
	return browser_funcs.retainobject (npobj);
}

void
Browser::ReleaseObject (NPObject* npobj)
{
	return browser_funcs.releaseobject (npobj);
}

NPUTF8 *
Browser::UTF8FromIdentifier (NPIdentifier identifier)
{
	return browser_funcs.utf8fromidentifier (identifier);
}

NPIdentifier
Browser::GetStringIdentifier (const char* name)
{
	return browser_funcs.getstringidentifier (name);
}

NPError
Browser::GetValue(NPP instance, NPNVariable variable, void *value)
{
	NPError rv = browser_funcs.getvalue (instance, variable, value);
	return rv;
}

bool
Browser::GetProperty (NPP npp, NPObject* obj, NPIdentifier propertyName, NPVariant *result)
{
	return browser_funcs.getproperty (npp, obj, propertyName, result);
}

bool
Browser::SetProperty (NPP npp, NPObject* obj, NPIdentifier propertyName, NPVariant *value)
{
	return browser_funcs.setproperty (npp, obj, propertyName, value);
}

bool
Browser::Invoke (NPP npp, NPObject* obj, NPIdentifier method_name, const NPVariant *args, uint32_t arg_count, NPVariant *result)
{
	return browser_funcs.invoke (npp, obj, method_name, args, arg_count, result);
}


void
Browser_Initialize (NPNetscapeFuncs *browser_funcs)
{
	Browser::Initialize (browser_funcs);
}

