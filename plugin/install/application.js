let nsISupports = Components.interfaces.nsISupports;
let nsISupportsArray = Components.interfaces.nsISupportsArray;
let nsISupportsString = Components.interfaces.nsISupportsString;

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function ApplicationStartup()
{
	var uriToLoad = null;
	var iframe = null;
	var src = null;
	
	// window.arguments[0]: URI to load (string), or an nsISupportsArray of
	//                      nsISupportsStrings to load, or a xul:tab of
	//                      a tabbrowser, which will be replaced by this
	//                      window (for this case, all other arguments are
	//                      ignored).
	//                 [1]: character set (string)
	//                 [2]: referrer (nsIURI)
	//                 [3]: postData (nsIInputStream)
	//                 [4]: allowThirdPartyFixup (bool)
	if ("arguments" in window && window.arguments[0])
		uriToLoad = window.arguments[0];
	
	iframe = document.getElementById ("MoonlightIFrame");
	
	if (uriToLoad) {
		if (uriToLoad instanceof nsISupportsArray) {
			let count = uriToLoad.Count ();
			
			alert ("loading " + count + " uris");
			for (let i = 0; i < count; i++) {
				let str = uriToLoad.GetElementAt (i).QueryInterface (nsISupportsString);
				alert ("urilist[" + i + "] = " + str);
			}
		} else if (uriToLoad instanceof XULElement) {
			alert ("loading a XULElement dingus");
		} else if (uriToLoad instanceof nsISupports) {
			alert ("loading a nsISupports dingus");
		} else {
			alert ("loading single uri " + uriToLoad);
			src = uriToLoad;
		}
	} else {
		alert ("no uris??");
	}
	
	iframe.src = src;
}
