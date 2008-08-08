/*
 * Log.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;

public static class Log {
	public static bool LogEnabled;
	
	public static void Write (string text, params object [] args)
	{
		if (LogEnabled)
			Console.Write (text, args);
	}
	public static void WriteLine (string text, params object [] args)
	{
		Write (text + Environment.NewLine, args);
	}
	public static void WriteLine ()
	{
		WriteLine ("");
	}
}