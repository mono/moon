//
// System.Windows.Automation.Peers.ComboBoxAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Olivier Dufour olivier(dot)duff(at)gmail(dot)com
// Copyright (c) 2009 Novell, Inc. (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//


using System;
using System.Collections.Generic;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

namespace System.Windows.Automation.Peers {
	public class ComboBoxAutomationPeer : SelectorAutomationPeer, IValueProvider, IExpandCollapseProvider {

		public ComboBoxAutomationPeer (ComboBox owner) : base (owner)
		{
			// ExpandCollapse Pattern Automation Events
			// raised by ComboBox.IsDropDownOpenChanged()
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return AutomationControlType.ComboBox;
		}

		public override object GetPattern (PatternInterface pattern)
		{
			if (pattern == PatternInterface.ExpandCollapse)
				return this;
			else if (pattern == PatternInterface.Value)
				return null;
			return base.GetPattern (pattern);
		}

		protected override string GetClassNameCore ()
		{
			return "ComboBox";
		}

		#region IExpandCollapseProvider

		ExpandCollapseState IExpandCollapseProvider.ExpandCollapseState {
			get {
				if (this.OwningComboBox.IsDropDownOpen)
					return ExpandCollapseState.Expanded;
				else
					return ExpandCollapseState.Collapsed;
			}
		}

		void IExpandCollapseProvider.Collapse ()
		{
			SetFocus ();
			this.OwningComboBox.IsDropDownOpen = false;
		}

		void IExpandCollapseProvider.Expand ()
		{
			SetFocus ();
			this.OwningComboBox.IsDropDownOpen = true;
		}

		#endregion IExpandCollapseProvider

		#region IValueProvider

		bool IValueProvider.IsReadOnly { get { return !this.OwningComboBox.IsEditable; } }

		string IValueProvider.Value { get { return this.OwningComboBox.SelectedItem.ToString (); } }

		void IValueProvider.SetValue (string value)
		{
			throw new InvalidOperationException ();
		}

		#endregion IValueProvider

		#region Private Properties

		private ComboBox OwningComboBox {
			get { return this.Owner as ComboBox; }
		}

		#endregion Private Properties

	}
}
