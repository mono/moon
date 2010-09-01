//
// Unit tests for RangeBaseAutomationPeer
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
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class RangeBaseAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class RangeBaseConcrete : RangeBase {
			public RangeBaseConcrete ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new RangeBaseAutomationPeerPoker (this);
			}

			// Overriding these methods shouldn't affect raising UIA events

			protected override void OnMaximumChanged (double oldMaximum, double newMaximum)
			{
			}

			protected override void OnMinimumChanged (double oldMinimum, double newMinimum)
			{
			}
		}

		public class RangeBaseAutomationPeerPoker : RangeBaseAutomationPeer, FrameworkElementAutomationPeerContract {
			public RangeBaseAutomationPeerPoker (RangeBaseConcrete rangebase)
				: base (rangebase)
			{
			}

			#region FrameworkElementAutomationPeerContract Members


			public AutomationPeer GetLabeledByCore_ ()
			{
				return base.GetLabeledByCore ();
			}

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
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

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			public string GetAutomationIdCore_ ()
			{
				return base.GetAutomationIdCore ();
			}

			public Rect GetBoundingRectangleCore_ ()
			{
				return base.GetBoundingRectangleCore ();
			}

			public global::System.Collections.Generic.List<AutomationPeer> GetChildrenCore_ ()
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

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			#endregion
		}

		[TestMethod]
		public override void GetPattern ()
		{
			FrameworkElementAutomationPeer peer
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ())
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

			Assert.IsNotNull (peer.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsTrue (Object.ReferenceEquals (peer, peer.GetPattern (PatternInterface.RangeValue)), "RangeValue");
		}

		[TestMethod]
		[Asynchronous]
		public virtual void TestHasKeyboardFocusAfterPattern ()
		{
			RangeBase fe = CreateConcreteFrameworkElement () as RangeBase;

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			IRangeValueProvider provider = null;

			CreateAsyncTest (fe,
			() => {
				provider = (IRangeValueProvider) peer.GetPattern (PatternInterface.RangeValue);
				Assert.IsNotNull (provider, "#0");
			}, 
			() => provider.SetValue (.5),
			() => Assert.IsTrue (peer.HasKeyboardFocus (), "#1"));
		}

		#region IRangeValueProvider tests

		[TestMethod]
		public void RangeValueProvider_Methods ()
		{
			RangeBaseConcrete s = new RangeBaseConcrete ();
			IRangeValueProvider rbap = new RangeBaseAutomationPeer (s);

			Assert.AreEqual (s.LargeChange, rbap.LargeChange, "LargeChange #0");
			s.LargeChange = 5;
			Assert.AreEqual (s.LargeChange, rbap.LargeChange, "LargeChange #1");

			Assert.AreEqual (s.Maximum, rbap.Maximum, "Maximum #0");
			s.Maximum = 100;
			Assert.AreEqual (s.Maximum, rbap.Maximum, "Maximum #1");

			Assert.AreEqual (s.Minimum, rbap.Minimum, "Minimum #0");
			s.Minimum = 0.1;
			Assert.AreEqual (s.Minimum, rbap.Minimum, "Minimum #1");

			Assert.AreEqual (s.SmallChange, rbap.SmallChange, "SmallChange #0");
			s.SmallChange = 3;
			Assert.AreEqual (s.SmallChange, rbap.SmallChange, "SmallChange #1");

			Assert.AreEqual (s.Value, rbap.Value, "Value #0");
			s.Value = 50;
			Assert.AreEqual (s.Value, rbap.Value, "Value #1");

			rbap.SetValue (0.5);
			Assert.AreEqual (0.5, s.Value, "RangeBaseConcrete.Value");
			Assert.AreEqual (0.5, rbap.Value, "IRangeValueProvider.Value");

			s.LargeChange = 0.9;
			Assert.AreEqual (0.9, rbap.LargeChange, "LargeChange #3");
			s.Maximum = 0.9;
			Assert.AreEqual (0.9, rbap.Maximum, "Maximum #3");
			s.Minimum = 0.1;
			Assert.AreEqual (0.1, rbap.Minimum, "Minimum #3");
			s.SmallChange = 0.1;
			Assert.AreEqual (0.1, rbap.SmallChange, "SmallChange #3");

			s.IsEnabled = true;
			Assert.IsFalse (rbap.IsReadOnly, "IsReadOnly #0");
			s.IsEnabled = false;
			Assert.IsTrue (rbap.IsReadOnly, "IsReadOnly #1");

			Assert.Throws<ElementNotEnabledException> (delegate {
				rbap.SetValue (.8);
			});
			s.IsEnabled = true;
			rbap.SetValue (.5);
		}

		[TestMethod]
		public void RangeValueProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			RangeBaseConcrete rangeBase = new RangeBaseConcrete ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (rangeBase);

			IRangeValueProvider provider 
				= (IRangeValueProvider) peer.GetPattern (PatternInterface.RangeValue);
			Assert.IsNotNull (provider, "GetPattern #0");

			provider.SetValue (0); // Setting old value
			EventsManager.Instance.Reset ();
			AutomationPropertyEventTuple tuple 
				= EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.ValueProperty);
			Assert.IsNull (tuple, "GetAutomationEventFrom #0");
			
			provider.SetValue (0.9);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.ValueProperty);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #1");
			Assert.AreEqual ((double) tuple.OldValue, 0,  "OldValue #1");
			Assert.AreEqual ((double) tuple.NewValue, 0.9, "NewValue #1");

			EventsManager.Instance.Reset ();
			provider.SetValue (0.5);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.ValueProperty);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #2");
			Assert.AreEqual ((double) tuple.OldValue, 0.9, "OldValue #2");
			Assert.AreEqual ((double) tuple.NewValue, 0.5,  "NewValue #2");

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.IsReadOnlyProperty);
			Assert.IsNull (tuple, "GetAutomationEventFrom #3");

			rangeBase.IsEnabled = false;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.IsReadOnlyProperty);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #4");
			Assert.IsTrue  ((bool) tuple.OldValue,  "OldValue #4");
			Assert.IsFalse ((bool) tuple.NewValue, "NewValue #4");

			EventsManager.Instance.Reset ();
			rangeBase.IsEnabled = true;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.IsReadOnlyProperty);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #5");
			Assert.IsFalse ((bool) tuple.OldValue, "OldValue #5");
			Assert.IsTrue  ((bool) tuple.NewValue,  "NewValue #5");

			EventsManager.Instance.Reset ();
			rangeBase.Maximum = 10.0;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.MaximumProperty);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #6");
			Assert.AreEqual (1d,  (double) tuple.OldValue, "OldValue #6");
			Assert.AreEqual (10d, (double) tuple.NewValue, "NewValue #6");

			EventsManager.Instance.Reset ();
			rangeBase.Minimum = 5.0;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.MinimumProperty);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #7");
			Assert.AreEqual (0d, (double) tuple.OldValue, "OldValue #7");
			Assert.AreEqual (5d, (double) tuple.NewValue, "NewValue #7");

			EventsManager.Instance.Reset ();
			rangeBase.LargeChange = 2.0;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.LargeChangeProperty);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #8");
			Assert.AreEqual (1d, (double) tuple.OldValue, "OldValue #8");
			Assert.AreEqual (2d, (double) tuple.NewValue, "NewValue #8");

			EventsManager.Instance.Reset ();
			rangeBase.SmallChange = 1.0;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, RangeValuePatternIdentifiers.SmallChangeProperty);
			Assert.IsNotNull (tuple, "GetAutomationEventFrom #9");
			Assert.AreEqual (0.1d, (double) tuple.OldValue, "OldValue #9");
			Assert.AreEqual (1d, (double) tuple.NewValue, "NewValue #9");

		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new RangeBaseConcrete ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new RangeBaseAutomationPeerPoker (element as RangeBaseConcrete);
		}
	}
}
