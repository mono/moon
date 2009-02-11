// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Copyright (c) 2009 Novell, Inc.
//
// Authors:
//	Rolf Bjarne Kvinge <RKvinge@novell.com>
//


using System;

namespace MoonlightTests {
	
	/// <summary>
	/// This class is for logging the harness
	/// </summary>
	public static class Log
	{
		private static bool? enabled;
		
		public static bool IsEnabled {
			get {
				if (!enabled.HasValue)
					enabled = !string.IsNullOrEmpty (Environment.GetEnvironmentVariable ("MOONLIGHT_LOG_HARNESS"));
				
				return enabled.Value;
			}
		}
		
		public static void WriteLine (string message)
		{
			if (IsEnabled)
				Console.WriteLine ("Moonlight harness: " + message);
		}
		
		public static void WriteLine (string message, params object [] args)
		{
			if (IsEnabled)
				Console.WriteLine ("Moonlight harness: " + message, args);
		}
	}
}
