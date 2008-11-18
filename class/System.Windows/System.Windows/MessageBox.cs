/*
 * MessageBox.cs.
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

namespace System.Windows
{
	public sealed class MessageBox
	{
		internal MessageBox()
		{
		}

		public static MessageBoxResult Show (string messageBoxText, string caption, MessageBoxButton button)
		{
			throw new NotImplementedException ();
		}

		public static MessageBoxResult Show (string messageBoxText)
		{
			throw new NotImplementedException ();
		}
	}
}
