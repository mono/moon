/*
 * ButtonAutomationPeer.cs.
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
	public class ButtonAutomationPeer : ButtonBaseAutomationPeer, IInvokeProvider
	{

		public ButtonAutomationPeer (Button owner)
			: base (owner)
		{
			owner.Click += (s, a) => { 
				RaiseAutomationEvent (AutomationEvents.InvokePatternOnInvoked); 
			};
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.Button;
		}

		protected override string GetClassNameCore ()
		{
			return "Button";
		}

		public override object GetPattern (PatternInterface patternInterface)
		{
			if (patternInterface == PatternInterface.Invoke)
				return this;
			return base.GetPattern (patternInterface);
		}

		void IInvokeProvider.Invoke ()
		{
			if (!IsEnabled ())
				throw new ElementNotEnabledException ();

			SetFocus ();
			((Button) Owner).OnClickInternal ();
		}
	}
}
