/*
 * InputMethod.cs.
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

namespace System.Windows.Input
{	
	public sealed partial class InputMethod : DependencyObject
	{
		public static bool GetIsInputMethodEnabled (DependencyObject target)
		{
			return (bool) target.GetValue (InputMethod.IsInputMethodEnabledProperty);
		}

		public static void SetIsInputMethodEnabled (DependencyObject target, bool value)
		{
			target.SetValue (InputMethod.IsInputMethodEnabledProperty, value);
		}
	}
}
