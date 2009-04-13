/*
 * TestLogger.cs
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
using System.Runtime.InteropServices;
using System.Security;
using System.Windows.Browser;

namespace Mono.Moonlight.UnitTesting
{
	public static class TestLogger
	{
		public static void LogMessage (string message)
		{
			HtmlPage.Window.Eval ("TestHost.LogMessage ('" + message + "')");
		}

		public static void LogWarning (string message)
		{
			HtmlPage.Window.Eval ("TestHost.LogWarning ('" + message + "')");
		}

		public static void LogDebug (string message)
		{
			HtmlPage.Window.Eval ("TestHost.LogDebug ('" + message + "')");
		}
        
		public static void LogError (string message)
		{
			HtmlPage.Window.Eval ("TestHost.LogError ('" + message + "')");
		}

		public static void LogResult (int result)
		{
			HtmlPage.Window.Eval ("TestHost.LogResult (" + result + ")");
		}       
	}
}
