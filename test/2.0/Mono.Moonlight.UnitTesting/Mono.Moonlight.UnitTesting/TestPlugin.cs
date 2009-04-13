/*
 * TestPlugin.cs
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;
using System.ComponentModel;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Threading;

namespace Mono.Moonlight.UnitTesting
{
	public static class TestPlugin
	{		
		static EventHandler TestPluginReadyEvent;
		static bool IsTestPluginReady;
		
		static TestPlugin ()
		{
			DispatcherTimer timer = new DispatcherTimer ();
			timer.Tick += delegate (object sender, EventArgs e) {
				IsTestPluginReady = (bool) HtmlPage.Window.GetProperty ("TestPluginReady"); 
				Console.WriteLine ("Ticked: {0}", IsTestPluginReady);
				if (IsTestPluginReady) {
					timer.Stop ();
					TestPluginReadyEvent (null, null);
				}
			};
			timer.Interval = TimeSpan.FromMilliseconds (100);
			timer.Start ();
		}
		
		/// <summary>
		/// Shuts down the browser.
		/// </summary>
		public static void SignalShutdown ()
		{
			HtmlPage.Window.Eval ("TestHost.SignalShutdown ()");
		}
		
		/// <summary>
		/// Captures a screenshot at the specified position (in screen coordinates)
		/// </summary>
		public static void CaptureSingleImage (string file_name, int x, int y, int width, int height, int delay)
		{
			HtmlPage.Window.Eval ("TestHost.CaptureSingleImage ('', '" + file_name + "', " + x + ", " + y + ", " + width + ", " + height + ")");
		}
		
		/// <summary>
		/// Captures a screenshot at the position reported by GetPluginPosition
		/// </summary>
		public static void CaptureSingleImage (string file_name, int width, int height, int delay)
		{
			int [] pos = GetPluginPosition ();
			CaptureSingleImage (file_name, pos [0], pos [1], width, height, delay);
		}
		
		public static void MoveMouseLogarithmic (int x, int y)
		{
			HtmlPage.Window.Eval ("TestHost.MoveMouseLogarithmic (" + x + ", " + y + ")");
		}
		
		public static void MouseLeftClick ()
		{
			HtmlPage.Window.Eval ("TestHost.MouseLeftClick ()");
		}
		
		public static int [] GetPluginPosition ()
		{
			int x, y;
			
			x = (int) (double) HtmlPage.Window.Eval ("TestHost.GetPluginPosition ().x");
			y = (int) (double) HtmlPage.Window.Eval ("TestHost.GetPluginPosition ().y");     

			return new int [] { x, y };
		}
		
		public static event EventHandler TestPluginReady {
			add {
				TestPluginReadyEvent += value;
				
				if (IsTestPluginReady)
					value (null, null);
			}
			remove {
				TestPluginReadyEvent -= value;
			}
		}
    }
}
