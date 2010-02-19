//
// System.Windows.Automation.Peers.ButtonBaseAutomationPeer
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
using System.Collections.Generic;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers {

	abstract public class ButtonBaseAutomationPeer : FrameworkElementAutomationPeer {

		protected ButtonBaseAutomationPeer (ButtonBase owner)
			: base (owner)
		{
			textBlock = owner.Content as TextBlock;
			if (textBlock != null)
				textBlock.UIATextChanged += TextBlock_TextChanged;
		}

		protected override string GetNameCore ()
		{
			ButtonBase buttonBase = Owner as ButtonBase;

			// Only when Content is TextBlock TextBlock.Name is returned
			string name = buttonBase.Content as string;
			if (name == null) {
				TextBlock textblock = buttonBase.Content as TextBlock;
				if (textblock == null)
					name = string.Empty;
				else {
					AutomationPeer peer 
						= FrameworkElementAutomationPeer.CreatePeerForElement (textblock);
					name = peer.GetName ();
				}
			}

			return buttonBase.GetValue (AutomationProperties.NameProperty) as string ?? name;
		}

		internal override List<AutomationPeer> ChildrenCore {
			get {
				ButtonBase buttonBase = Owner as ButtonBase;
				string contentAsString = buttonBase.Content as string;
				if (contentAsString != null)
					return null;
				else
					return base.ChildrenCore;
			}
		}

		internal override void OnContentChanged (object oldContent, object newContent)
		{
			base.OnContentChanged (oldContent, newContent);

			if (textBlock != null)
				textBlock.UIATextChanged -= TextBlock_TextChanged;

			textBlock = newContent as TextBlock;
			if (textBlock != null)
				textBlock.UIATextChanged += TextBlock_TextChanged;
		}

		private void TextBlock_TextChanged (object sender, DependencyPropertyChangedEventArgs args)
		{
			RaiseNameChanged ();
		}

		private TextBlock textBlock;
	}
}
