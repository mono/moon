
var tests = [];
var results = [];

var xaml = 
"<Canvas  xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\" Width=\"120\" Height=\"80\">" +
"	<MediaElement MarkerReached=\"onMarkerReached\" " +
"		MediaEnded=\"onMediaEnded\" " +
"		MediaOpened=\"onMediaOpened\" " +
"		MediaFailed=\"onMediaFailed\" " + 
"		x:Name=\"TestVideo\" Source=\"%TEST%.wmv\" Opacity=\"0.0\" IsHitTestVisible=\"False\"/> " +
"	<Rectangle Width=\"1280\" Height=\"720\"  >" +
"		<Rectangle.Fill>" +
"			<VideoBrush SourceName=\"TestVideo\" Stretch=\"None\"/>" +
"		</Rectangle.Fill>" +
"	</Rectangle>" +
"</Canvas>";
// Width=\"120\" Height=\"80\"
var current_test;
var currently_expected;
var current_result = null;
var marker_counter = 0;
var log_counter = 0;
var accumulative_result = true;
var current_finished = false;

function log_result (msg) {
	var result = "";
	
	if (currently_expected != null && currently_expected.length > 0) {
		if (currently_expected [log_counter] == msg) {
			result = " OK";
		} else {
			result = " FAILED (expected: '" + currently_expected [log_counter] + "')";
			current_result = false;
		}
	}

	msg = msg + result;

	if (current_result == false) {
		msg = "<div style='color:#FF0000;'>" + msg + "</div>";
	} else {
		msg = "<div style='color:#00BB00;'>" + msg + "</div>";
	}

	log_msg (msg)
	log_counter++;
}

function log_msg (msg) {
	document.getElementById("testlog").innerHTML += msg;// + "<br>";
}

function onBrokenPluginError (sender, args) {
	log_result ("BrokenPlugin");
	current_result = false;
}

function onPluginLoad (sender, args) {
	log_result ("PluginLoad");
	runTests ();
}
 style='foreground-color: red'
function onMarkerReached (sender, markerEventArgs) {
	marker_counter++;
	log_result (marker_counter.toString () + " MarkerReached: ms = " + Math.round(markerEventArgs.marker.time.seconds*1000).toString () + ", type = " + markerEventArgs.marker.type + ", text = " + markerEventArgs.marker.text);
}

function onMediaOpened (sender, args) {
	log_result ("MediaOpened");
}

function onMediaEnded (sender, args) {
	log_result ("MediaEnded");
	currently_expected = null;
	if (current_result == null) {
		current_result = true;
	}
	current_finished = true;
}

function onMediaFailed (sender, args) {
	log_result ("MediaFailed");
	if (current_result == null) {
		current_result = true;
	}
	current_finished = true;
}

function runTest (test_name) {
	var media;
	var SL = document.getElementById ("MoonlightControl");
	var content = SL.content;
	var container = content.findName ("Container");

	current_finished = false;
	marker_counter = 0;
	log_counter = 0;
	current_result = null;
	current_test = test_name;
	currently_expected = results [test_name];
	
	log_msg ("<br>Running " + test_name);

	var test_xaml = xaml;

	test_xaml = test_xaml.replace ("%TEST%", test_name);

	media = content.createFromXaml (test_xaml);
	while (container.Children.Count > 0)
		container.Children.RemoveAt (0);
	container.Children.Add (media);
	
	
}

function checkTests () {

	if (!current_finished) {
		setTimeout ("checkTests ()", 1000);
		return;
	}

	accumulative_result &= current_result;

	var next = -1;
	for (i = 0; i < tests.length - 1; i++) {
		//log_result ("Checking " + tests [i] + " against " + current_test);
		if (tests [i] == current_test) {
			next = i+1;
			break;
		}
	}

	if (next > -1) {
		runTest (tests [next]);
		setTimeout ("checkTests ()", 1000);
	} else {
		log_msg ("");
		msg = "Running tests: DONE, result = ";
	 	msg += ((accumulative_result != 0));// ? "OK" : "FAIL")); 
		if (accumulative_result) {
			msg = "<div style='color:#00BB00;'>" + msg + "</div>";
		} else { 
			msg = "<div style='color:#FF0000;'>" + msg + "</div>";
		}
		log_msg (msg);
		document.getElementById("Moonlight").removeChild (document.getElementById ("MoonlightHost"));
	}
}

function runTests () {
	log_msg ("Running " + tests.length + " tests...");

	if (tests.length > 0) {
		runTest (tests [0]);
		setTimeout ("checkTests ()", 1000);
	} else {
		log_msg ("No tests to run.");
	}

}




