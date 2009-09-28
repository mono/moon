//
// Unit tests for MediaElementAutomationPeer
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
using System.Windows;
using System.Windows.Automation.Peers;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class MediaElementAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class MediaElementAutomationPeerPoker : MediaElementAutomationPeer, FrameworkElementAutomationPeerContract {

			public MediaElementAutomationPeerPoker (MediaElement owner)
				: base (owner)
			{
			}

			#region Overridden methods

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			#endregion

			#region Wrapper Methods

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public AutomationPeer GetLabeledByCore_ ()
			{
				return base.GetLabeledByCore ();
			}

			public bool IsContentElementCore_ ()
			{
				return base.IsContentElementCore ();
			}

			public bool IsControlElementCore_ ()
			{
				return base.IsControlElementCore ();
			}

			public string GetAcceleratorKeyCore_ ()
			{
				return base.GetAcceleratorKeyCore ();
			}

			public string GetAccessKeyCore_ ()
			{
				return base.GetAccessKeyCore ();
			}

			public string GetAutomationIdCore_ ()
			{
				return base.GetAutomationIdCore ();
			}

			public global::System.Windows.Rect GetBoundingRectangleCore_ ()
			{
				return base.GetBoundingRectangleCore ();
			}

			public global::System.Collections.Generic.List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
			}

			public global::System.Windows.Point GetClickablePointCore_ ()
			{
				return base.GetClickablePointCore ();
			}

			public string GetHelpTextCore_ ()
			{
				return base.GetHelpTextCore ();
			}

			public string GetItemStatusCore_ ()
			{
				return base.GetItemStatusCore ();
			}

			public string GetItemTypeCore_ ()
			{
				return base.GetItemTypeCore ();
			}

			public string GetLocalizedControlTypeCore_ ()
			{
				return base.GetLocalizedControlTypeCore ();
			}

			public AutomationOrientation GetOrientationCore_ ()
			{
				return base.GetOrientationCore ();
			}

			public bool HasKeyboardFocusCore_ ()
			{
				return base.HasKeyboardFocusCore ();
			}

			public bool IsEnabledCore_ ()
			{
				return base.IsEnabledCore ();
			}

			public bool IsKeyboardFocusableCore_ ()
			{
				return base.IsKeyboardFocusableCore ();
			}

			public bool IsOffscreenCore_ ()
			{
				return base.IsOffscreenCore ();
			}

			public bool IsPasswordCore_ ()
			{
				return base.IsPasswordCore ();
			}

			public bool IsRequiredForFormCore_ ()
			{
				return base.IsRequiredForFormCore ();
			}


			#endregion
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			MediaElementAutomationPeerPoker mapp = new MediaElementAutomationPeerPoker (new MediaElement ());
			Assert.AreEqual (AutomationControlType.Custom, mapp.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Custom, mapp.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract mapp
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("MediaElement", mapp.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("MediaElement", mapp.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new MediaElement ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new MediaElementAutomationPeerPoker (element as MediaElement);
		}

	}
}
