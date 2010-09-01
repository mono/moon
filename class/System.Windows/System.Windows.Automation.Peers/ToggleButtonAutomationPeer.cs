/*
 * ToggleButtonAutomationPeer.cs.
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
	public class ToggleButtonAutomationPeer : ButtonBaseAutomationPeer, IToggleProvider
	{
		public ToggleButtonAutomationPeer (ToggleButton owner)
			: base (owner)
		{
			this.owner = owner;

			// UIA Event TogglePatternIdentifiers.ToggleStateProperty
			// raised by ToggleButton.OnIsCheckedPropertyChanged().
			toggleState = ToggleState;
		}

		public override object GetPattern (PatternInterface patternInterface)
		{
			if (patternInterface == PatternInterface.Toggle)
				return this;
			return base.GetPattern (patternInterface);
		}

		protected override string GetClassNameCore ()
		{
			return "Button";
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.Button;
		}

		#region IToggleProvider implementation

		void IToggleProvider.Toggle ()
		{
			if (!owner.IsEnabled)
				throw new ElementNotEnabledException ();

			SetFocus ();
			bool? isChecked = owner.IsChecked;
			if (isChecked == true)
				owner.IsChecked = owner.IsThreeState ? null : ((bool?) false);
			else
				owner.IsChecked = (bool?) isChecked.HasValue;
		}

		ToggleState IToggleProvider.ToggleState {
			get { return ToggleState; }
		}

		#endregion

		internal virtual void RaiseToggleStateChanged ()
		{
			RaisePropertyChangedEvent (TogglePatternIdentifiers.ToggleStateProperty,
			                           toggleState,
						   ToggleState);
			toggleState = ToggleState;
		}

		private ToggleState ToggleState {
			get {
				bool? isChecked = owner.IsChecked;
				if (isChecked.HasValue)
					return (isChecked == true) ? ToggleState.On : ToggleState.Off;
				return ToggleState.Indeterminate;
			}
		}

		private ToggleButton owner;
		private ToggleState toggleState;
	}
}
