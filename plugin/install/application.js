function ApplicationStartup()
{
	var uriToLoad = null;
	var iframe = null;
	
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
	iframe.src = uriToLoad;
}
