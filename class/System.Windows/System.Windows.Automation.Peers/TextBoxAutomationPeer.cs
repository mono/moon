/*
 * TextBoxAutomationPeer.cs.
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
using System.Windows.Automation;
using System.Windows.Automation.Provider;
using System.Windows.Controls;

namespace System.Windows.Automation.Peers
{	
	public class TextBoxAutomationPeer : FrameworkElementAutomationPeer, IValueProvider
	{
		public TextBoxAutomationPeer (TextBox owner)
			: base (owner)
		{
			throw new NotImplementedException ();
		}
		
		protected override string GetNameCore ()
		{
			throw new NotImplementedException ();
		}
		
		public override object GetPattern (PatternInterface patternInterface)
		{
			throw new NotImplementedException ();
		}
		
		void IValueProvider.SetValue (string value)
		{
			throw new NotImplementedException ();
		}
	
		bool IValueProvider.IsReadOnly { 
			get { throw new NotImplementedException (); } 
		}
		
		string IValueProvider.Value { 
			get { throw new NotImplementedException (); } 
		}
	}
}
