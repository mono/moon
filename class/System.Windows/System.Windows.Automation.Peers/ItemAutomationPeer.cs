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
// Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
//
// Contact:
//   Moonlight Team (moonlight-list@lists.ximian.com)
//

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace System.Windows.Automation.Peers {
	public abstract class ItemAutomationPeer : FrameworkElementAutomationPeer {

		protected ItemAutomationPeer (UIElement uielement) : base ((ContentControl) uielement)
		{
		}

		protected ItemAutomationPeer (object item, ItemsControlAutomationPeer itemsControlAutomationPeer) : base (null)
		{
			throw new NotImplementedException ();
		}

		protected override string GetNameCore ()
		{
			return ((ContentControl) Owner).Content as string ?? string.Empty;
		}

		protected override string GetItemTypeCore ()
		{
			return string.Empty;
		}

		protected ItemsControlAutomationPeer ItemsControlAutomationPeer {
			get {
				ContentControl control = Owner as ContentControl;
				if (control == null || control.Parent == null)
					return null;

				return FrameworkElementAutomationPeer.FromElement (control.Parent as UIElement) as ItemsControlAutomationPeer;
			}
		}

		protected object Item {
			get {
				ListBoxItem lbi = Owner as ListBoxItem;
				if (lbi != null && lbi.Parent is ComboBox)
					return ((ComboBox) lbi.Parent).ItemContainerGenerator.ItemFromContainer (lbi);
				else
					return Owner;
			}
		}

		protected override bool HasKeyboardFocusCore ()
		{
			return base.HasKeyboardFocusCore ();
		}

		protected override bool IsContentElementCore ()
		{
			return base.IsContentElementCore ();
		}

		protected override bool IsControlElementCore ()
		{
			return base.IsControlElementCore ();
		}

		protected override bool IsEnabledCore ()
		{
			return base.IsEnabledCore ();
		}

		protected override bool IsKeyboardFocusableCore ()
		{
			return base.IsKeyboardFocusableCore ();
		}

		protected override bool IsOffscreenCore ()
		{
			return base.IsOffscreenCore ();
		}
 
		protected override bool IsPasswordCore ()
		{
			return base.IsPasswordCore ();
		}

		protected override bool IsRequiredForFormCore ()
		{
			return base.IsRequiredForFormCore ();
		}
 
		protected override List<AutomationPeer> GetChildrenCore ()
		{
			return base.GetChildrenCore ();
		}

		public override object GetPattern (PatternInterface patternInterface)
		{
			return base.GetPattern (patternInterface);
		}

		protected override string GetAcceleratorKeyCore ()
		{
			return base.GetAcceleratorKeyCore ();
		}

		protected override string GetAccessKeyCore ()
		{
			return base.GetAcceleratorKeyCore();
		}

		protected override string GetAutomationIdCore ()
		{
			return base.GetAutomationIdCore ();
		}
 
		protected override string GetClassNameCore ()
		{
			return base.GetClassNameCore ();
		}
 
		protected override string GetHelpTextCore ()
		{
			return base.GetHelpTextCore ();
		}
 
		protected override string GetItemStatusCore ()
		{
			return base.GetItemStatusCore ();
		}

		protected override string GetLocalizedControlTypeCore ()
		{
			return base.GetLocalizedControlTypeCore ();
		}

		protected override void SetFocusCore ()
		{
			base.SetFocusCore ();
		}

		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			return base.GetAutomationControlTypeCore ();
		}

		protected override AutomationOrientation GetOrientationCore ()
		{
			return base.GetOrientationCore ();
		}

		protected override AutomationPeer GetLabeledByCore ()
		{
			return base.GetLabeledByCore ();
		}
 
		protected override Point GetClickablePointCore ()
		{
			return base.GetClickablePointCore ();
		}

		protected override Rect GetBoundingRectangleCore ()
		{
			return base.GetBoundingRectangleCore ();
		}

		internal override List<AutomationPeer> ChildrenCore {
			get {
				ContentControl owner = (ContentControl) Owner;
				if (owner.Content == null || owner.Content is string)
					return null;
				else
					return base.ChildrenCore; 
			}
		}
	}
}
