var NAME = "Novell Moonlight";
var VERSION = "0.1";
var PLID = "@novell.com/" + NAME + ",version=" + VERSION;
var LOADER = "libmoonloader.so";
var PLUGIN = "libmoonplugin.so";

initInstall (NAME, PLID, VERSION);

/* For some reason Mozilla can only tell us the global plugin directory,
 * so we have to guess it from the user's profile. */
var plugins = getFolder("Profile", "../../plugins");
addFile (PLID, VERSION, LOADER, plugins, null);
addFile (PLID, VERSION, PLUGIN, plugins, null);
performInstall ();
alert("Succesfully installed " + NAME + " " + VERSION);
