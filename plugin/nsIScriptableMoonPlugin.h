/*
 * DO NOT EDIT.  THIS FILE IS GENERATED FROM nsIScriptableMoonPlugin.idl
 */

#ifndef __gen_nsIScriptableMoonPlugin_h__
#define __gen_nsIScriptableMoonPlugin_h__


#ifndef __gen_nsISupports_h__
#include "nsISupports.h"
#endif

/* For IDL files that don't want to include root IDL files. */
#ifndef NS_NO_VTABLE
#define NS_NO_VTABLE
#endif

/* starting interface:    nsIScriptableMoonPlugin */
#define NS_ISCRIPTABLEMOONPLUGIN_IID_STR "d2d536a0-b6fc-11d5-9d10-0060b0fbd8ac"

#define NS_ISCRIPTABLEMOONPLUGIN_IID \
  {0xd2d536a0, 0xb6fc, 0x11d5, \
    { 0x9d, 0x10, 0x00, 0x60, 0xb0, 0xfb, 0xd8, 0xac }}

class NS_NO_VTABLE nsIScriptableMoonPlugin : public nsISupports {
 public: 

  NS_DEFINE_STATIC_IID_ACCESSOR(NS_ISCRIPTABLEMOONPLUGIN_IID)

  /* readonly attribute string version; */
  NS_IMETHOD GetVersion(char * *aVersion) = 0;

  /* void showVersion (); */
  NS_IMETHOD ShowVersion(void) = 0;

  /* void clear (); */
  NS_IMETHOD Clear(void) = 0;

};

/* Use this macro when declaring classes that implement this interface. */
#define NS_DECL_NSISCRIPTABLEMOONPLUGIN \
  NS_IMETHOD GetVersion(char * *aVersion); \
  NS_IMETHOD ShowVersion(void); \
  NS_IMETHOD Clear(void); 

/* Use this macro to declare functions that forward the behavior of this interface to another object. */
#define NS_FORWARD_NSISCRIPTABLEMOONPLUGIN(_to) \
  NS_IMETHOD GetVersion(char * *aVersion) { return _to GetVersion(aVersion); } \
  NS_IMETHOD ShowVersion(void) { return _to ShowVersion(); } \
  NS_IMETHOD Clear(void) { return _to Clear(); } 

/* Use this macro to declare functions that forward the behavior of this interface to another object in a safe way. */
#define NS_FORWARD_SAFE_NSISCRIPTABLEMOONPLUGIN(_to) \
  NS_IMETHOD GetVersion(char * *aVersion) { return !_to ? NS_ERROR_NULL_POINTER : _to->GetVersion(aVersion); } \
  NS_IMETHOD ShowVersion(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->ShowVersion(); } \
  NS_IMETHOD Clear(void) { return !_to ? NS_ERROR_NULL_POINTER : _to->Clear(); } 

#if 0
/* Use the code below as a template for the implementation class for this interface. */

/* Header file */
class nsScriptableMoonPlugin : public nsIScriptableMoonPlugin
{
public:
  NS_DECL_ISUPPORTS
  NS_DECL_NSISCRIPTABLEMOONPLUGIN

  nsScriptableMoonPlugin();

private:
  ~nsScriptableMoonPlugin();

protected:
  /* additional members */
};

/* Implementation file */
NS_IMPL_ISUPPORTS1(nsScriptableMoonPlugin, nsIScriptableMoonPlugin)

nsScriptableMoonPlugin::nsScriptableMoonPlugin()
{
  /* member initializers and constructor code */
}

nsScriptableMoonPlugin::~nsScriptableMoonPlugin()
{
  /* destructor code */
}

/* readonly attribute string version; */
NS_IMETHODIMP nsScriptableMoonPlugin::GetVersion(char * *aVersion)
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void showVersion (); */
NS_IMETHODIMP nsScriptableMoonPlugin::ShowVersion()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* void clear (); */
NS_IMETHODIMP nsScriptableMoonPlugin::Clear()
{
    return NS_ERROR_NOT_IMPLEMENTED;
}

/* End of implementation class template. */
#endif


#endif /* __gen_nsIScriptableMoonPlugin_h__ */
