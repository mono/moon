var NAME = "Novell Moonlight";
var VERSION = "0.1";
var PLID = "@novell.com/" + NAME + ",version=" + VERSION;
var LOADER = "libmoonloader.so";
var PLUGIN = "libmoonplugin.so";
var LIB = "libmoon.so";
var AVUTIL = "libavutil.so";
var SWSCALE = "libswscale.so";
var AVCODEC = "libavcodec.so";
var AVFORMAT = "libavformat.so";

initInstall (NAME, PLID, VERSION);

/* For some reason Mozilla can only tell us the global plugin directory,
 * so we have to guess it from the user's profile. */
var plugins = getFolder("Profile", "../../plugins");
addFile (PLID, VERSION, LOADER, plugins, null);
addFile (PLID, VERSION, PLUGIN, plugins, "/moonlight/" + PLUGIN);
addFile (PLID, VERSION, LIB, plugins, "/moonlight/" + LIB);
addFile (PLID, VERSION, AVUTIL, plugins, "/moonlight/" + AVUTIL);
addFile (PLID, VERSION, SWSCALE, plugins, "/moonlight/" + SWSCALE);
addFile (PLID, VERSION, AVCODEC, plugins, "/moonlight/" + AVCODEC);
addFile (PLID, VERSION, AVFORMAT, plugins, "/moonlight/" + AVFORMAT);
performInstall ();
alert("Succesfully installed " + NAME + " " + VERSION);
