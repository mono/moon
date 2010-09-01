//
// System.Windows.Automation.Peers.PasswordBoxAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Windows.Automation;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

namespace System.Windows.Automation.Peers
{
	public class PasswordBoxAutomationPeer : FrameworkElementAutomationPeer, IValueProvider
	{
		public PasswordBoxAutomationPeer (PasswordBox owner)
			: base (owner)
		{
			this.owner = owner;
			// Not raising ValuePatternIdentifiers.IsReadOnlyProperty, never changes.
			string oldPassword = owner.Password;
			owner.PasswordChanged += (o, e) => {
				RaisePropertyChangedEvent (ValuePatternIdentifiers.ValueProperty, 
				                           oldPassword,
							   owner.Password);
				oldPassword = owner.Password;
			};
		}

		public override object GetPattern (PatternInterface patternInterface)
		{
			if (patternInterface == PatternInterface.Value)
				return this;
			return base.GetPattern (patternInterface);;
		}

		void IValueProvider.SetValue (string value)
		{
			if (!owner.IsEnabled)
				throw new ElementNotEnabledException ();

			SetFocus ();
			owner.Password = value;
		}

		bool IValueProvider.IsReadOnly {
			get { return false; }
		}

		string IValueProvider.Value {
			get { throw new InvalidOperationException (); }
		}

		internal override AutomationControlType? AutomationControlTypeCore {
			get { return AutomationControlType.Edit; }
		}

		internal override string ClassNameCore {
			get { return "PasswordBox"; }
		}

		internal override bool PasswordCore {
			get { return true; }
		}

		private PasswordBox owner;
	}
}

