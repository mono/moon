var me = null;
var current = -1;
var file = "";
var plugin = null;
var top = null;
var timeout = null;

var printf_counter = 0;
var complete_log = [];

var test_plugin = null;
var finished = false;

function mergeNullandUndef (arg)
{
	if (!arg)
		return "null/undefined";
	return arg;
}

function printf (message, action) {
	if (!Plugin.Silverlight) {
		if (action != "create")
			return;
		action = "printf"
		message = "Not creating any test results, we're executing with Moonlight.";
	}
		
	var client = new XMLHttpRequest();

	client.open("POST", "printf.aspx?file=" + file + ".js&action=" + action, false);
	client.setRequestHeader("Content-Type", "text/plain;charset=UTF-8");
	client.send(message);
}

function printf_formatted (message, action) {
	printf ("    \"" + message + "\", \n", action);
}

function log_to_div (msg)
{
	try {
		test_plugin.LogMessage (msg);
	} catch (ex) {
	}
	document.getElementById ("log").innerHTML += msg + "<BR/>";
}

function log (msg)
{
	if (finished)
		return;

	log_to_div (msg);
	printf_formatted (msg, "append");
	complete_log.push (msg);
}

function logAttributes (media)
{
	log (" Attributes: " + media.Attributes + " Attributes.Count: " + media.Attributes.Count);

	// Sort the array of attributes by the name
	var names = [];

	for (var i = 0; i < media.Attributes.Count; i++) {
		var attribute = media.Attributes.GetItem (i);
		names.push (attribute.Name);
	}

	names.sort ();

	//	for (var i = 0; i < media.Attributes.Count; i++) {
	for (var i = 0; i < names.length; i++) {
		var attribute = null;
		for (var k = 0; k < media.Attributes.Count; k++) {
			attribute = media.Attributes.GetItem (k);
			if (attribute.Name == names [i]) {
				break;
			} else {
				attribute = null;
			}
		}
		log (" #" + (i + 1).toString ( ) + ": Name = '" + attribute.Name + "', Value = '" + attribute.Value + "'");
	}
	log (" Source: " + media.Source);
	log (" NaturalDuration: " + Math.round (media.NaturalDuration.Seconds * 100) / 100);
	log ("<hr />");
}

function OnTestFinished ()
{
	if (finished)
		return;
	finished = true;

	printf ("    \"\"\n", "append");
	printf ("];\n", "append");

	var failed = true;

	complete_log.push ("");

	log_to_div ("");
	log_to_div ("");

	if (result.length != complete_log.length) {
		log_to_div ("Test failed, result.length (" + result.length + ") != complete_log.length (" + complete_log.length + ")");
	} else {
		failed = false;
		for (i = 0; i < result.length; i++) {
			if (result [i].length != complete_log [i].length) {
				log_to_div ("Test failed: at index " + i + " expected length " + result [i].length + " and got length " + complete_log [i].length + "");
				log_to_div ("    " + result [i]);
				log_to_div ("    " + complete_log [i]);
				failed = true;
			} else if (result [i] != complete_log [i]) {
				log_to_div ("Test failed: at index " + i + " expected [" + result [i] + "] and got [" + complete_log [i] + "]");
				failed = true;
			}
		}
	}

	if (failed) {
		log_to_div ("expected output:");
		for (i = 0; i < result.length; i++)
			log_to_div (result [i]);
		log_to_div ("actual output:");
		for (i = 0; i < complete_log.length; i++)
			log_to_div (complete_log [i]);
	}

	if (!failed)
		log_to_div ("Test succeeded");

	if (window.opener)
		window.opener.document.getElementById ("result").innerHTML = failed ? "Failed" : "Success";

	try {
		if (failed) {
			test_plugin.LogResult (-1);
		} else {
			test_plugin.LogResult (1);
		}

		test_plugin.SignalShutdown ();
	} catch (ex) {
	}
}

function OnMediaOpened (e, args)
{
	clearTimeout (timeout);
	log ("OnMediaOpened (" + e + " [" + e.Source + "], " + mergeNullandUndef(args));
	logAttributes (e);
	OnTestFinished ();
}

function OnMediaFailed (e, args)
{
	clearTimeout (timeout);
	log ("OnMediaFailed (" + e + " [" + e.Source + "], " + mergeNullandUndef(args) + " [" + ErrorEventArgsToOneLineString (args) + "])");
	OnTestFinished ();
}

function OnPluginError (e, args)
{
	clearTimeout (timeout);
	log ("OnError (" + e + ", " + mergeNullandUndef(args) + " [" + ErrorEventArgsToOneLineString (args) + "])");
	OnTestFinished ();
}

function OnCurrentStateChanged (e, args)
{
	//log ("OnCurrentStateChanged (" + e + " [" + e.Source + "], " + args + "): state: " + e.CurrentState);
}

function callback ()
{
	log ("Timedout: [" + me.Source + "]");
	OnTestFinished ();
}

function NextASX ()
{
	current++;

	clearTimeout (timeout);

	if (file != "")
		return;

	file = asx_file;

	printf ("", "create");
//	printf ("// Moonlight: " + Plugin.Moonlight + "\n", "append");
//	printf ("// Silverlight: " + Plugin.Silverlight + "\n", "append");
	printf ("var result = [\n", "append");
	log ("Opening: " + asx_file);

	timeout = setTimeout ("callback ()", 2000);
	top.Children.Clear ();
	me = plugin.content.createFromXaml ("<MediaElement Name=\"MediaPlayer\" Width=\"300\" Height=\"300\" MediaFailed=\"OnMediaFailed\" MediaOpened=\"OnMediaOpened\" CurrentStateChanged=\"OnCurrentStateChanged\"/>");
	top.Children.Add (me);
	me.Source = file;
}

function OnPluginLoaded (e, args)
{
	test_plugin = document.getElementById ("_TestPlugin");
	e = document.getElementById ("_MoonlightControl");

	plugin = e;
	top = e.content.findName ("Top");
	check_can_start ();
}

function check_can_start ()
{
	if (!TestPluginReady) {
		setTimeout (check_can_start, 50);
		return;
	}
	NextASX ();
}

function createPlugin ()
{
	document.write (
	'<div>' +
	'	<embed type="application/x-silverlight" width="10" height="10"' +
	'		id="_MoonlightControl" Source="all.xaml" OnError="OnPluginError" OnLoad="OnPluginLoaded"' +
	'		style="position:absolute; left:0px; top:0px" background="green">' +
	'	</embed>' +
	'</div>'
	);
}
createPlugin ();

/*
function createTestPlugin () {
	var found = false;
	for (var i = 0; i < navigator.plugins.length; i++) {
		var pl = navigator.plugins [i];
		for (var j = 0; j < pl.length; j++) {
			if (pl.item (j).type == "application/x-jolttest") {
				found = true;
				break;
			}
		}
		if (found)
			break;
	}

	if (found) {
		document.write (
			"<div>"+
				"<embed id=\"_TestPlugin\" width=\"1\" height=\"0\" type=\"application/x-jolttest\"></embed>" +
			"</div>");
	}
}
*/
//createTestPlugin ();

