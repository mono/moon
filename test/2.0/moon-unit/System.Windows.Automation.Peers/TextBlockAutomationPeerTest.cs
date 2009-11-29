//
// Unit tests for TextBlockAutomationPeer
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
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class TextBlockAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class TextBlockAutomationPeerPoker : TextBlockAutomationPeer, FrameworkElementAutomationPeerContract {

			public TextBlockAutomationPeerPoker (TextBlock owner)
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
			TextBlockAutomationPeerPoker tbap = new TextBlockAutomationPeerPoker (new TextBlock ());
			Assert.AreEqual (AutomationControlType.Text, tbap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Text, tbap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual ("TextBlock", feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("TextBlock", feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetName ()
		{
			TextBlock textblock = new TextBlock ();
			TextBlockAutomationPeerPoker tbap = new TextBlockAutomationPeerPoker (textblock);
			Assert.AreEqual (String.Empty, tbap.GetName (), "GetName");
			Assert.AreEqual (String.Empty, tbap.GetNameCore_ (), "GetNameCore");

			string textBlockname = "Textblock name!";
			textblock.Text = textBlockname;
			Assert.AreEqual (textBlockname, tbap.GetName (), "GetName #1");
			Assert.AreEqual (textBlockname, tbap.GetNameCore_ (), "GetNameCore #1");

			textblock.Text = null;
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

			TextBlock fe = CreateConcreteFrameworkElement () as TextBlock;
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
				// Even if TextBlock.Name changes the value will be the same
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
		public override void IsContentElement ()
		{
			TextBlockAutomationPeerPoker tbap = new TextBlockAutomationPeerPoker (new TextBlock ());
			Assert.IsTrue (tbap.IsContentElement (), "IsContentElement");
			Assert.IsTrue (tbap.IsContentElementCore_ (), "IsContentElementCore");
		}

		[TestMethod]
		public override void GetLabeledBy_AttachedProperty()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer(fe);

			Assert.IsNull (feap.GetLabeledBy (), "GetLabeledBy");
			Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore");

			FrameworkElement labeledBy = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract labeledByPeer
				= FrameworkElementAutomationPeer.CreatePeerForElement (labeledBy) as FrameworkElementAutomationPeerContract;

			fe.SetValue (AutomationProperties.LabeledByProperty, labeledBy);
			Assert.AreNotSame (labeledByPeer, feap.GetLabeledBy (), "GetLabeledBy NotSame #1");
			Assert.AreNotSame (labeledByPeer, feap.GetLabeledByCore_ (), "GetLabeledByCore NotSame #1");

			AutomationPeer realPeer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			Assert.IsNotNull (feap.GetLabeledBy (), "GetLabeledBy NotNull #2");
			Assert.IsNotNull (feap.GetLabeledByCore_ (), "GetLabeledByCore NotNull #2");

			AutomationPeer realLabeledByPeer = FrameworkElementAutomationPeer.CreatePeerForElement (labeledBy);
			Assert.AreSame (realLabeledByPeer, feap.GetLabeledBy (), "GetLabeledBy Same #3");
			Assert.AreSame (realLabeledByPeer, feap.GetLabeledByCore_ (), "GetLabeledByCore Same #3");
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "TextBlock ContentElement.");

			bool textBlockLoaded = false;
			TextBlock textBlock = CreateConcreteFrameworkElement () as TextBlock;
			textBlock.Loaded += (o, e) => textBlockLoaded = true;
			TestPanel.Children.Add (textBlock);

			EnqueueConditional (() => textBlockLoaded, "TextBlock #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (textBlock);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				textBlock.Text = "Hello world!";
			});
			// We add the text
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (textBlock);
				Assert.IsNull (peer.GetChildren (), "GetChildren #1");
				// Remove text
				textBlock.Text = string.Empty;
				Assert.IsNull (peer.GetChildren (), "GetChildren #2");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public override void GetBoundingRectangle ()
		{
			base.GetBoundingRectangle ();

			TestLocationAndSize ();
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new TextBlock ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new TextBlockAutomationPeerPoker (element as TextBlock);
		}
	}
}
