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
		private LogSource? log_source;
		
		public LogReadyRoutedEventArgs ()
		{
		}
		
		internal LogReadyRoutedEventArgs (IntPtr calldata, bool dropref) : base (calldata, dropref)
		{
		}
		
		public string Log {
			get {
				if (log == null) {
					if (this.NativeHandle == IntPtr.Zero)
						return null;
					log = NativeMethods.log_ready_routed_event_args_get_log (NativeHandle);
					if (log == null)
						log = string.Empty;
				}
				return log;
			}
		}
		
		public LogSource LogSource {
			get {
				if (!log_source.HasValue) {
					if (this.NativeHandle == IntPtr.Zero)
						return LogSource.RequestLog;
					log_source = (LogSource) NativeMethods.log_ready_routed_event_args_get_log_source (NativeHandle);
				}
				return log_source.Value;
			}
		}
	}
}
