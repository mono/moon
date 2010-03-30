const nsIArray = Components.interfaces.nsIArray;
const nsIVariant = Components.interfaces.nsIVariant;
const nsIURI = Components.interfaces.nsIURI;
const nsISupports = Components.interfaces.nsISupports;
const nsISupportsArray = Components.interfaces.nsISupportsArray;
const nsISupportsString = Components.interfaces.nsISupportsString;

Components.utils.import("resource://gre/modules/XPCOMUtils.jsm");

function myDump(aMessage) {
  var consoleService = Components.classes["@mozilla.org/consoleservice;1"]
                                 .getService(Components.interfaces.nsIConsoleService);
  consoleService.logStringMessage("Moonlight: " + aMessage);
}

function ApplicationStartup()
{
  try {
    if (!window.arguments)
      return;

    myDump ("window.arguments = " + window.arguments);

    uriToLoad = window.arguments[0];
    windowTitle = window.arguments[1];
    windowWidth = window.arguments[2];
    windowHeight = window.arguments[3];

    myDump ("creating window for app " + uriToLoad + ", sizing to " + windowWidth + " x " + windowHeight);

    win = document.getElementById ("MoonlightWindow");
    iframe = document.getElementById ("MoonlightIFrame");

    if (uriToLoad instanceof nsISupports) {
      src = uriToLoad.QueryInterface (nsIURI).spec;
    } else {
      src = uriToLoad;
    }

    iframe.setAttribute ('src', src);

    win.width = windowWidth;
    win.height = windowHeight;

    win.setAttribute ("title", windowTitle);
  }
  catch (e) {
    Components.utils.reportError(e);
  }
}
