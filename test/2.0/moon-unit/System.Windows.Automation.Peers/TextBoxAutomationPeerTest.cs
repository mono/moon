//
// Unit tests for TextBoxAutomationPeer
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
using System.Windows.Automation.Provider;
using System.Windows.Controls;

using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class TextBoxAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class TextBoxAutomationPeerPoker : TextBoxAutomationPeer, FrameworkElementAutomationPeerContract {

			public TextBoxAutomationPeerPoker (TextBox owner)
				: base (owner)
			{
			}

			#region Overriden methods

			public string GetNameCore_()
			{
				return base.GetNameCore();
			}

			public object GetPattern_(PatternInterface iface)
			{
				return base.GetPattern(iface);
			}

			#endregion

			#region Wrapper methods

			public AutomationControlType GetAutomationControlTypeCore_()
			{
				return base.GetAutomationControlTypeCore();
			}

			public string GetClassNameCore_()
			{
				return base.GetClassNameCore();
			}

			public bool IsControlElementCore_()
			{
				return base.IsControlElementCore();
			}

			public AutomationPeer GetLabeledByCore_()
			{
				return base.GetLabeledByCore();
			}

			public bool IsContentElementCore_()
			{
				return base.IsContentElementCore();
			}

			public string GetAcceleratorKeyCore_()
			{
				return base.GetAcceleratorKeyCore();
			}

			public string GetAccessKeyCore_()
			{
				return base.GetAccessKeyCore();
			}

			public string GetAutomationIdCore_()
			{
				return base.GetAutomationIdCore();
			}

			public Rect GetBoundingRectangleCore_()
			{
				return base.GetBoundingRectangleCore();
			}

			public List<AutomationPeer> GetChildrenCore_()
			{
				return base.GetChildrenCore();
			}

			public Point GetClickablePointCore_()
			{
				return base.GetClickablePointCore();
			}

			public string GetHelpTextCore_()
			{
				return base.GetHelpTextCore();
			}

			public string GetItemStatusCore_()
			{
				return base.GetItemStatusCore();
			}

			public string GetItemTypeCore_()
			{
				return base.GetItemTypeCore();
			}

			public string GetLocalizedControlTypeCore_()
			{
				return base.GetLocalizedControlTypeCore();
			}

			public AutomationOrientation GetOrientationCore_()
			{
				return base.GetOrientationCore();
			}

			public bool HasKeyboardFocusCore_()
			{
				return base.HasKeyboardFocusCore();
			}

			public bool IsEnabledCore_()
			{
				return base.IsEnabledCore();
			}

			public bool IsKeyboardFocusableCore_()
			{
				return base.IsKeyboardFocusableCore();
			}

			public bool IsOffscreenCore_()
			{
				return base.IsOffscreenCore();
			}

			public bool IsPasswordCore_()
			{
				return base.IsPasswordCore();
			}

			public bool IsRequiredForFormCore_()
			{
				return base.IsRequiredForFormCore();
			}

			#endregion
		}

		[TestMethod]
		public override void GetName()
		{
			TextBox textbox = new TextBox ();
			TextBoxAutomationPeerPoker tbap = new TextBoxAutomationPeerPoker (textbox);
			Assert.AreEqual (String.Empty, tbap.GetName(), "GetName");
			Assert.AreEqual (String.Empty, tbap.GetNameCore_(), "GetNameCore");

			string textBlockname = "Textbox name!";
			textbox.Text = textBlockname;
			Assert.AreEqual (textBlockname, tbap.GetName (), "GetName #1");
			Assert.AreEqual (textBlockname, tbap.GetNameCore_ (), "GetNameCore #1");

			textbox.Text = string.Empty;
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #2");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #2");
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty0Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			TextBox fe = CreateConcreteFrameworkElement () as TextBox;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			CreateAsyncTest (fe,
			() => {
				EventsManager.Instance.Reset ();
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#0");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.SetValue (AutomationProperties.NameProperty, "Attached Name");
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#1");
				Assert.AreEqual ("Attached Name", (string) tuple.NewValue, "#2");
				Assert.AreEqual (string.Empty, tuple.OldValue, "#3");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.SetValue (AutomationProperties.NameProperty, "Name");
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#4");
				Assert.AreEqual ("Name", (string) tuple.NewValue, "#5");
				Assert.AreEqual ("Attached Name", (string) tuple.OldValue, "#6");
			},
			() => {
				// Even if TextBox.Name changes the value will be the same
				EventsManager.Instance.Reset ();
				fe.Text = "New value";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#7");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.SetValue (AutomationProperties.NameProperty, null);
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#8");
				Assert.AreEqual ("New value", (string) tuple.NewValue, "#9");
				Assert.AreEqual ("Name", (string) tuple.OldValue, "#10");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Text = "What's up?";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#11");
				Assert.AreEqual ("What's up?", (string) tuple.NewValue, "#12");
				Assert.AreEqual ("New value", (string) tuple.OldValue, "#13");
			});
		}

		[TestMethod]
		public override void GetPattern ()
		{
			TextBoxAutomationPeerPoker tbap = new TextBoxAutomationPeerPoker (new TextBox ());

			Assert.IsNull (tbap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (tbap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (tbap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (tbap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (tbap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (tbap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (tbap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (tbap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (tbap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (tbap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (tbap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (tbap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (tbap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (tbap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (tbap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (tbap.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (tbap.GetPattern (PatternInterface.Value), "Value");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("TextBox", feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("TextBox", feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.Edit, feap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Edit, feap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
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

		[TestMethod]
		[Asynchronous]
		public override void GetBoundingRectangle ()
		{
			base.GetBoundingRectangle ();

			TestLocationAndSize ();
		}

		[TestMethod]
		[Asynchronous]
		public void TestHasKeyboardFocusAfterPattern ()
		{
			TextBox fe = CreateConcreteFrameworkElement () as TextBox;

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			IValueProvider provider = null;

			CreateAsyncTest (fe,
			() => {
				provider = (IValueProvider) peer.GetPattern (PatternInterface.Value);
				Assert.IsNotNull (provider, "#0");
			}, 
			() => provider.SetValue ("Hello world"),
			() => Assert.IsTrue (peer.HasKeyboardFocus (), "#1"));
		}

		#region IValueProvider tests
		
		[TestMethod]
		public void ValueProviderIsReadOnly ()
		{
			TextBox textbox = CreateConcreteFrameworkElement () as TextBox;
			AutomationPeer feap
				= CreateConcreteFrameworkElementAutomationPeer (textbox) as AutomationPeer;
			IValueProvider valueProvider = feap.GetPattern (PatternInterface.Value) as IValueProvider;

			Assert.IsNotNull (valueProvider, "#0");
			Assert.IsFalse(valueProvider.IsReadOnly, "#1");

			textbox.IsReadOnly = true;
			Assert.IsTrue (valueProvider.IsReadOnly, "#2");

			textbox.IsReadOnly = false;
			textbox.IsEnabled = false;
			Assert.IsFalse (valueProvider.IsReadOnly, "#3");

			textbox.IsReadOnly = true;
			Assert.IsTrue (valueProvider.IsReadOnly, "#4");
		}

		[TestMethod]
		public void ValueProviderValue ()
		{
			TextBox textbox = CreateConcreteFrameworkElement () as TextBox;
			AutomationPeer feap
				= CreateConcreteFrameworkElementAutomationPeer (textbox) as AutomationPeer;
			IValueProvider valueProvider = feap.GetPattern (PatternInterface.Value) as IValueProvider;

			Assert.IsNotNull (valueProvider, "#0");
			Assert.IsNotNull (valueProvider.Value, "#1");

			textbox.Text = "Hello world!";
			Assert.AreEqual ("Hello world!", valueProvider.Value, "#2");

			textbox.Text = "New text";
			Assert.AreEqual ("New text", valueProvider.Value, "#3");
		}

		[TestMethod]
		public void ValueProviderSetValue ()
		{
			TextBox textbox = CreateConcreteFrameworkElement () as TextBox;
			AutomationPeer feap
				= CreateConcreteFrameworkElementAutomationPeer (textbox) as AutomationPeer;
			IValueProvider valueProvider = feap.GetPattern (PatternInterface.Value) as IValueProvider;

			Assert.IsNotNull (valueProvider, "#0");
			Assert.IsNotNull (valueProvider.Value, "#1");

			valueProvider.SetValue ("Hello world");
			Assert.AreEqual ("Hello world", textbox.Text, "#2");

			valueProvider.SetValue ("new text");
			Assert.AreEqual ("new text", textbox.Text, "#3");

			textbox.IsReadOnly = true;
			Assert.Throws<InvalidOperationException>(delegate {
				valueProvider.SetValue ("is readonly");
			}, "#4");
			Assert.AreEqual ("new text", textbox.Text, "#5");

			textbox.IsReadOnly = false;
			textbox.IsEnabled = false;
			Assert.Throws<ElementNotEnabledException>(delegate {
				valueProvider.SetValue ("is not enabled");
			}, "#6");
			Assert.AreEqual ("new text", textbox.Text, "#7");
		}

		[TestMethod]
		[Asynchronous]
		public void ValueProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			TextBox textbox = CreateConcreteFrameworkElement () as TextBox;
			AutomationPeer peer 
				= FrameworkElementAutomationPeer.CreatePeerForElement (textbox);
			AutomationPropertyEventTuple tuple = null;

			CreateAsyncTest (textbox,
			// IsReadOnly
			() => {
				EventsManager.Instance.Reset ();
				textbox.IsReadOnly = true;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, ValuePatternIdentifiers.IsReadOnlyProperty);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #0");
				Assert.IsTrue ((bool) tuple.NewValue, "GetAutomationEventFrom.NewValue #0");
				Assert.IsFalse ((bool) tuple.OldValue, "GetAutomationEventFrom.NewValue #0");
			},
			() => {
				EventsManager.Instance.Reset ();
				textbox.IsEnabled = false;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, ValuePatternIdentifiers.IsReadOnlyProperty);
				Assert.IsNull (tuple, "GetAutomationEventFrom #1");
			},
			() => {
				EventsManager.Instance.Reset ();
				textbox.IsReadOnly  = false;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, ValuePatternIdentifiers.IsReadOnlyProperty);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #2");
				Assert.IsFalse ((bool) tuple.NewValue, "GetAutomationEventFrom.NewValue #2");
				Assert.IsTrue ((bool) tuple.OldValue, "GetAutomationEventFrom.NewValue #2");
			},
			() => {
				EventsManager.Instance.Reset ();
				textbox.IsEnabled = true;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, ValuePatternIdentifiers.IsReadOnlyProperty);
				Assert.IsNull (tuple, "GetAutomationEventFrom #3");
			},
			// Value
			() => {
				textbox.Text = string.Empty;
				EventsManager.Instance.Reset ();
				textbox.Text = "hello world";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, ValuePatternIdentifiers.ValueProperty);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #4");
				Assert.AreEqual ("hello world", tuple.NewValue, "GetAutomationEventFrom.NewValue #4");
				Assert.AreEqual (string.Empty, tuple.OldValue, "GetAutomationEventFrom.NewValue #4");
			},
			() => {
				EventsManager.Instance.Reset ();
				textbox.Text = string.Empty;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, ValuePatternIdentifiers.ValueProperty);
				Assert.IsNotNull (tuple, "GetAutomationEventFrom #5");
				Assert.AreEqual (string.Empty, tuple.NewValue, "GetAutomationEventFrom.NewValue #5");
				Assert.AreEqual ("hello world", tuple.OldValue, "GetAutomationEventFrom.NewValue #5");
			});

		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new TextBox ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer(FrameworkElement element)
		{
			return new TextBoxAutomationPeerPoker (element as TextBox);
		}
	}
}

