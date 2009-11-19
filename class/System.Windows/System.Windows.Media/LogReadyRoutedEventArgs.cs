/*
 * LogReadyRoutedEventArgs.cs.
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
using Mono;

namespace System.Windows.Media
{	
	public sealed class LogReadyRoutedEventArgs : RoutedEventArgs
	{
		private string log;
		private LogSource log_source;
		
		public LogReadyRoutedEventArgs ()
		{
		}
		
		internal LogReadyRoutedEventArgs (IntPtr calldata) : base (calldata, false)
		{
		}
		
		public string Log {
			get { return log; }
		}
		
		public LogSource LogSource {
			get { return log_source; }
		}
	}
}
