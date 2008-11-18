/*
 * VisualStateChangedEventArgs.cs.
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
using System.Windows.Controls;

namespace System.Windows
{
	public class VisualStateChangedEventArgs : EventArgs
	{
		VisualState old_state;
		VisualState new_state;
		Control control;
		
		internal VisualStateChangedEventArgs (IntPtr raw)
		{
			throw new NotImplementedException ();
		}

		internal VisualStateChangedEventArgs (VisualState oldState, VisualState newState, Control control)
		{
			old_state = oldState;
			new_state = newState;
			this.control = control;
		}

		public VisualState OldState {
			get { return old_state; }
		}

		public VisualState NewState {
			get { return new_state; }
		}

		public Control Control {
			get { return control; }
		}
	}
}
