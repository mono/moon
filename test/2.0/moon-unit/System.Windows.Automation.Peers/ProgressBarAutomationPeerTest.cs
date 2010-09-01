//
// Unit tests for ProgressBarAutomationPeer
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 olivier dufour olivier(dot)duff(at)gmail(dot)com
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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Automation;
using System.Collections.Generic;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls.Primitives;
using MoonTest.System.Windows.Automation.Peers;

using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ___ProgressBarAutomationPeerTest : RangeBaseAutomationPeerTest {

		public class ProgressBarAutomationPeerPoker : ProgressBarAutomationPeer, FrameworkElementAutomationPeerContract {

			public ProgressBarAutomationPeerPoker (ProgressBarConcrete owner)
				: base (owner)
			{
			}

			#region Overriden methods

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public object GetPattern_ (PatternInterface iface)
			{
				return base.GetPattern (iface);
			}

			#endregion

			#region Wrapper methods

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			public bool IsControlElementCore_ ()
			{
				return base.IsControlElementCore ();
			}

			public AutomationPeer GetLabeledByCore_ ()
			{
				return base.GetLabeledByCore ();
			}

			public bool IsContentElementCore_ ()
			{
				return base.IsContentElementCore ();
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

			public Rect GetBoundingRectangleCore_ ()
			{
				return base.GetBoundingRectangleCore ();
			}

			public List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
			}

			public Point GetClickablePointCore_ ()
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

		public class ProgressBarConcrete : ProgressBar {

			public ProgressBarConcrete () : base ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new ProgressBarAutomationPeerPoker (this);
			}
		}

		[TestMethod]
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("ProgressBar", feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("ProgressBar", feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.ProgressBar, feap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.ProgressBar, feap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void GetPattern ()
		{
			ProgressBarConcrete concrete = new ProgressBarConcrete ();
			FrameworkElementAutomationPeer peer
				= CreateConcreteFrameworkElementAutomationPeer (concrete)
					as FrameworkElementAutomationPeer;

			Assert.IsNull (peer.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (peer.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (peer.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (peer.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (peer.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (peer.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (peer.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (peer.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (peer.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (peer.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (peer.GetPattern (PatternInterface.Window), "Window");
			Assert.IsNull (peer.GetPattern (PatternInterface.Value), "Value");

			concrete.IsIndeterminate = false;
			Assert.IsNotNull (peer.GetPattern (PatternInterface.RangeValue), "RangeValue #0");
			Assert.IsTrue (Object.ReferenceEquals (peer, peer.GetPattern (PatternInterface.RangeValue)), "RangeValue #1");

			concrete.IsIndeterminate = true;
			Assert.IsNull (peer.GetPattern (PatternInterface.RangeValue), "RangeValue #2");
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable ()
		{
			TestIsNotKeyboardFocusable ();
		}

		[TestMethod]
		[Asynchronous]
		public override void IsKeyboardFocusable_Event ()
		{
			TestIsNotKeyboardFocusableEvent ();
		}

		[TestMethod]
		public void IsReadOnly ()
		{
			ProgressBarConcrete concrete = new ProgressBarConcrete ();
			FrameworkElementAutomationPeer peer
				= CreateConcreteFrameworkElementAutomationPeer (concrete)
					as FrameworkElementAutomationPeer;
			var rangeValue = peer.GetPattern (PatternInterface.RangeValue)
				as IRangeValueProvider;

			Assert.IsTrue (concrete.IsEnabled);
			Assert.IsTrue (peer.IsEnabled ());
			Assert.IsTrue (rangeValue.IsReadOnly);

			concrete.IsEnabled = false;
			Assert.IsFalse (peer.IsEnabled ());
			Assert.IsTrue (rangeValue.IsReadOnly);
		}

		[TestMethod]
		[ExpectedException (typeof (InvalidOperationException))]
		public void SetValue ()
		{
			ProgressBarConcrete concrete = new ProgressBarConcrete ();
			FrameworkElementAutomationPeer peer
				= CreateConcreteFrameworkElementAutomationPeer (concrete)
					as FrameworkElementAutomationPeer;
			var rangeValue = peer.GetPattern (PatternInterface.RangeValue)
				as IRangeValueProvider;
			rangeValue.SetValue (0);
		}

		[TestMethod]
		[Asynchronous]
		public override void TestHasKeyboardFocusAfterPattern ()
		{
			ProgressBar fe = CreateConcreteFrameworkElement () as ProgressBar;

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			IRangeValueProvider provider = null;

			CreateAsyncTest (fe,
			() => {
				provider = (IRangeValueProvider) peer.GetPattern (PatternInterface.RangeValue);
				Assert.IsNotNull (provider, "#0");
			},
			() => {
				Assert.Throws<InvalidOperationException> (() => {
				global::System.Console.WriteLine (">>>>>>>>>>>>>>>>>>>provider is {0}", provider.GetType ());
					provider.SetValue (.5);
				});
			},
			() => Assert.IsFalse (peer.HasKeyboardFocus (), "#1"));
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ProgressBarConcrete ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ProgressBarAutomationPeerPoker (element as ProgressBarConcrete);
		}
	}
}

