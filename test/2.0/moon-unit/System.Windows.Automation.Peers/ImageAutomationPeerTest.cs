//
// Unit tests for ImageAutomationPeer
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
using System.Windows;
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Controls;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ImageAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class ImageAutomationPeerPoker : ImageAutomationPeer, FrameworkElementAutomationPeerContract {

			public ImageAutomationPeerPoker (Image owner)
				: base (owner)
			{
			}

			#region Overriden methods

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public bool IsControlElementCore_ ()
			{
				return base.IsControlElementCore  ();
			}

			#endregion

			#region Wrapper methods

			public AutomationPeer GetLabeledByCore_ ()
			{
				return base.GetLabeledByCore();
			}

			public bool IsContentElementCore_ ()
			{
				return base.IsContentElementCore();
			}

			public string GetAcceleratorKeyCore_ ()
			{
				return base.GetAcceleratorKeyCore();
			}

			public string GetAccessKeyCore_ ()
			{
				return base.GetAccessKeyCore();
			}

			public string GetAutomationIdCore_ ()
			{
				return base.GetAutomationIdCore();
			}

			public Rect GetBoundingRectangleCore_ ()
			{
				return base.GetBoundingRectangleCore();
			}

			public List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
			}

			public Point GetClickablePointCore_ ()
			{
				return base.GetClickablePointCore();
			}

			public string GetHelpTextCore_ ()
			{
				return base.GetHelpTextCore();
			}

			public string GetItemStatusCore_ ()
			{
				return base.GetItemStatusCore();
			}

			public string GetItemTypeCore_ ()
			{
				return base.GetItemTypeCore();
			}

			public string GetLocalizedControlTypeCore_ ()
			{
				return base.GetLocalizedControlTypeCore();
			}

			public AutomationOrientation GetOrientationCore_ ()
			{
				return base.GetOrientationCore();
			}

			public bool HasKeyboardFocusCore_ ()
			{
				return base.HasKeyboardFocusCore();
			}

			public bool IsEnabledCore_ ()
			{
				return base.IsEnabledCore();
			}

			public bool IsKeyboardFocusableCore_ ()
			{
				return base.IsKeyboardFocusableCore();
			}

			public bool IsOffscreenCore_ ()
			{
				return base.IsOffscreenCore();
			}

			public bool IsPasswordCore_ ()
			{
				return base.IsPasswordCore();
			}

			public bool IsRequiredForFormCore_ ()
			{
				return base.IsRequiredForFormCore();
			}

			#endregion

		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			ImageAutomationPeerPoker imagePeer = new ImageAutomationPeerPoker (new Image ());
			Assert.AreEqual (AutomationControlType.Image, imagePeer.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Image, imagePeer.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("Image", feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("Image", feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetLabeledBy_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer(fe);
			Assert.IsNull (feap.GetLabeledBy (), "GetLabeledBy");
			Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore");

			TextBlock labeledBy = new TextBlock ();
			labeledBy.Text = "LabeledBy text";
			AutomationPeer labeledByPeer = FrameworkElementAutomationPeer.CreatePeerForElement (labeledBy);

			fe.SetValue (AutomationProperties.LabeledByProperty, labeledBy);
			Assert.AreSame (labeledByPeer, feap.GetLabeledBy (), "GetLabeledBy #1");
			Assert.AreSame (labeledByPeer, feap.GetLabeledByCore_ (), "GetLabeledByCore #1");

			fe.SetValue (AutomationProperties.LabeledByProperty, null);
			Assert.IsNull (feap.GetLabeledBy (), "GetLabeledBy #2");
			Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore #2");
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new Image ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ImageAutomationPeerPoker (element as Image);
		}
	}
}
