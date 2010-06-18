//
// System.Windows.Automation.Peers.RichTextBoxAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
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
using System.Windows.Automation;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers {
	public class RichTextBoxAutomationPeer : FrameworkElementAutomationPeer {
		public RichTextBoxAutomationPeer (RichTextBox owner)
			: base (owner)
		{
			Console.WriteLine ("NIEX: System.Windows.Automation.Peers.RichTextBoxAutomationPeer:.ctor");
			throw new NotImplementedException ();
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			Console.WriteLine ("NIEX: System.Windows.Automation.Peers.RichTextBoxAutomationPeer:GetAutomationControlTypeCore");
			throw new NotImplementedException ();
		}

		protected override List<AutomationPeer> GetChildrenCore ()
		{
			Console.WriteLine ("NIEX: System.Windows.Automation.Peers.RichTextBoxAutomationPeer:GetChildrenCore");
			throw new NotImplementedException ();
		}

		protected override string GetClassNameCore ()
		{
			Console.WriteLine ("NIEX: System.Windows.Automation.Peers.RichTextBoxAutomationPeer:GetClassNameCore");
			throw new NotImplementedException ();
		}

		public override object GetPattern (PatternInterface patternInterface)
		{
			Console.WriteLine ("NIEX: System.Windows.Automation.Peers.RichTextBoxAutomationPeer:GetPattern");
			throw new NotImplementedException ();
		}
	}
}

