/*
 * CheckBoxAutomationPeer.cs.
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
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers
{
	public class CheckBoxAutomationPeer : ToggleButtonAutomationPeer
	{
		public CheckBoxAutomationPeer (CheckBox owner)
			: base (owner)
		{
		}

		protected override string GetClassNameCore ()
		{
			return "CheckBox";
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.CheckBox;
		}
	}
}
