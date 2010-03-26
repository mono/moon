const nsIURI = Components.interfaces.nsIURI;
const nsISupports = Components.interfaces.nsISupports;
const nsISupportsArray = Components.interfaces.nsISupportsArray;
const nsISupportsString = Components.interfaces.nsISupportsString;

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function ApplicationStartup()
{
	var uriToLoad = null;
	var iframe = null;
	var src = null;
	
	if ("arguments" in window && window.arguments[0])
		uriToLoad = window.arguments[0];
	
	iframe = document.getElementById ("MoonlightIFrame");
	
	if (uriToLoad) {
		if (uriToLoad instanceof nsISupports) {
			let uri = uriToLoad.QueryInterface (nsIURI);
			src = uri.path;
		} else {
			src = uriToLoad;
		}
	}
	
	if (iframe)
		iframe.src = src;
}
