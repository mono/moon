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
	public class ButtonAutomationPeer : FrameworkElementAutomationPeer, IValueProvider
	{

		public void SetValue (string value)
		{
			throw new System.NotImplementedException();
		}
		
		public string Value {
			get {
				throw new System.NotImplementedException();
			}
		}
		
		public bool IsReadOnly {
			get {
				throw new System.NotImplementedException();
			}
		}
		
		public ButtonAutomationPeer (Button owner)
			: base (owner)
		{
			
		}
	}
}
