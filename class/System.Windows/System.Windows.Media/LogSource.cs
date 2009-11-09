/*
 * LogSource.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

namespace System.Windows.Media
{
	public enum LogSource
	{
		RequestLog,
		Stop,
		Seek,
		Pause,
		SourceChanged,
		EndOfStream,
		MediaElementShutdown,
		RuntimeShutdown,
	}
}
