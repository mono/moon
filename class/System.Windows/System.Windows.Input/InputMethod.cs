/*
 * InputMethod.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008-2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System.Windows.Controls;

namespace System.Windows.Input
{	
	public sealed partial class InputMethod : DependencyObject
	{
		static void CheckTarget (DependencyObject target)
		{
			if (target == null)
				throw new ArgumentNullException ("target");
			if (!(target is TextBox))
				throw new ArgumentException ("target is not a TextBox", "target");
		}

		public static bool GetIsInputMethodEnabled (DependencyObject target)
		{
			CheckTarget (target);
			return (bool) target.GetValue (InputMethod.IsInputMethodEnabledProperty);
		}

		public static void SetIsInputMethodEnabled (DependencyObject target, bool value)
		{
			CheckTarget (target);
			target.SetValue (InputMethod.IsInputMethodEnabledProperty, value);
		}
	}
}
