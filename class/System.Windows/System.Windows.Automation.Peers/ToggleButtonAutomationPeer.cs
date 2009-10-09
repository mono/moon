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

			toggleState = ToggleState;
			owner.Checked += ToggleButton_ToggleStateChanged;
			owner.Indeterminate += ToggleButton_ToggleStateChanged;
			owner.Unchecked += ToggleButton_ToggleStateChanged;
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

		private void ToggleButton_ToggleStateChanged (object sender, RoutedEventArgs args)
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
