/*
 * MessageBox.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using Mono;

namespace System.Windows {

	public sealed class MessageBox {

		private MessageBox()
		{
		}

		public static MessageBoxResult Show (string messageBoxText, string caption, MessageBoxButton button)
		{
			if (messageBoxText == null)
				throw new ArgumentNullException ("messageBoxText");
			if (caption == null)
				throw new ArgumentNullException ("caption");
			if ((button < MessageBoxButton.OK) || (button > MessageBoxButton.OKCancel))
				throw new ArgumentException ("button");

			return (MessageBoxResult) NativeMethods.message_box_show (caption, messageBoxText, (int) button);
		}

		public static MessageBoxResult Show (string messageBoxText)
		{
			return Show (messageBoxText, "Moonlight", MessageBoxButton.OK);
		}
	}
}
