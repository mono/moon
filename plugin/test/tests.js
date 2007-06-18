/*
 * tests.js: JavaScript unit tests for MoonLight browser plugin.
 *
 * Author:
 *   Everaldo Canuto (everaldo@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

var already = false;
var output;
var control1;
var control2;

function print (text) {
	output.value += text + "\n";
}

function AssertAreEqual (text, tvalue, expected) {
	var result = (tvalue == expected) ? "." : "F";
	print (result + " " + text + ": " + tvalue);
}

function AssertNotEqual (text, tvalue, expected) {
	var result = (tvalue == expected) ? "F" : ".";
	print (result + " " + text + ": " + tvalue);
}

function SetupTests () {
	output = document.form.outputtext;
	control1 = document.getElementById("moonControl1");
	control2 = document.getElementById("moonControl2");

	output.value = "";
	BeforeLoadTests ();
}

function BeforeLoadTests () {
	print ("=> Page started\n");

	AssertNotEqual ("control1.settings.version", control1.settings.version, "");
	AssertAreEqual ("control1.isLoaded", control1.isLoaded, false);

	AssertAreEqual ("control1.content.actualWidth", control1.content.actualWidth, 400);
	AssertAreEqual ("control2.content.actualWidth", control2.content.actualWidth, 200);

	AssertAreEqual ("control1.content.actualHeight", control1.content.actualHeight, 180);
	AssertAreEqual ("control2.content.actualHeight", control2.content.actualHeight, 190);
}

function AfterLoadTests () {
	// I dont know why but IE calls onload two times.
	if (already) return;
	already = true;

	print ("\n=> Page loaded");

	print (""); ControlTest ();
	print (""); ControlSettingsTest ();
	print (""); ControlContentTest ();
}

function ControlTest () {
	// control.isLoaded
	AssertAreEqual ("control1.isLoaded", control1.isLoaded, true);

	// control.initParams
	AssertAreEqual ("control1.initParams", control1.initParams, "");
	AssertAreEqual ("control2.initParams", control2.initParams, "paramValue1, paramValue2");

	// control.source - Cant be changed after initialization.
	AssertAreEqual ("control1.source", control1.source, "circle.xaml");
	control1.source = "emote.xaml";
	AssertAreEqual ("control1.source", control1.source, "emote.xaml");
	control1.source = "circle.xaml";
}

function ControlSettingsTest () {
	// control.settings.version
	AssertNotEqual ("control1.settings.version", control1.settings.version, "");

	// control.settings.background
	AssertAreEqual ("control1.settings.background", control1.settings.background, "#c0c0c0");
	control1.settings.background = "#b1b1b1";
	AssertAreEqual ("control1.settings.background", control1.settings.background, "#b1b1b1");
	AssertAreEqual ("control2.settings.background", control2.settings.background, "#ffebcd");

	// control.settings.enableFramerateCounter
	AssertAreEqual ("control1.settings.enableFramerateCounter", control1.settings.enableFramerateCounter, false);

	// control.settings.enableRedrawRegions
	AssertAreEqual ("control1.settings.enableRedrawRegions", control1.settings.enableRedrawRegions, false);
	control1.settings.enableRedrawRegions = true;
	AssertAreEqual ("control1.settings.enableRedrawRegions", control1.settings.enableRedrawRegions, true);
	control1.settings.enableRedrawRegions = false;

	// control.settings.enableHtmlAccess
	AssertAreEqual ("control1.settings.enableHtmlAccess", control1.settings.enableHtmlAccess, true);

	// control.settings.maxFrameRate
	AssertAreEqual ("control1.settings.maxFrameRate", control1.settings.maxFrameRate, 60);
	control1.settings.maxFrameRate = 20;
	AssertAreEqual ("control1.settings.maxFrameRate", control1.settings.maxFrameRate, 20);

	// control.settings.enableFramerateCounter
	AssertAreEqual ("control1.settings.windowless", control1.settings.windowless, false);
	AssertAreEqual ("control2.settings.windowless", control2.settings.windowless, false);
}

function ControlContentTest () {
	// control1.content.actualWidth
	AssertAreEqual ("control1.content.actualWidth", control1.content.actualWidth, 400);
	AssertAreEqual ("control2.content.actualWidth", control2.content.actualWidth, 200);

	// control.content.actualHeight
	AssertAreEqual ("control1.content.actualHeight", control1.content.actualHeight, 180);
	AssertAreEqual ("control2.content.actualHeight", control2.content.actualHeight, 190);
}
