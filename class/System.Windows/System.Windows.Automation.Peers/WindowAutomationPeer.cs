/*
 * WindowAutomationPeer.cs.
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
using System.Windows.Automation;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

namespace System.Windows.Automation.Peers
{
	internal class WindowAutomationPeer : FrameworkElementAutomationPeer {

		public WindowAutomationPeer (FrameworkElement owner)
			: base (owner)
		{
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.Window;
		}

		protected override string GetClassNameCore ()
		{
			return "Window";
		}

		protected override Rect GetBoundingRectangleCore ()
		{
			return GetBoundingRectangleFrom ((FrameworkElement) Owner);
		}
	}
}
