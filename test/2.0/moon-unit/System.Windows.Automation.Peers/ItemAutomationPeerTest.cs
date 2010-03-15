//
// Unit tests for ItemAutomationPeer
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

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ItemAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class ConcreteContentControl : ContentControl {
			public ConcreteContentControl ()
				: base ()
			{
			}

			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new ItemAutomationPeerPoker (this);
			}
		}

		public class ItemAutomationPeerPoker : ItemAutomationPeer, FrameworkElementAutomationPeerContract {

			public ItemAutomationPeerPoker (UIElement item) 
				: base (item)
			{
			}

			#region Overridden Methods

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
			}

			public string GetItemTypeCore_ ()
			{
				return base.GetItemTypeCore ();
			}

			public ItemsControlAutomationPeer ItemsControlAutomationPeer_ {
				get { return base.ItemsControlAutomationPeer; }
			}

			public object Item_ {
				get { return base.Item; }
			}

			#endregion

			#region Wrapper Methods

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
		public void CtorWantsAContentControl ()
		{
			// needs a ContentControl even if the ctor accept an UIElement
			Assert.Throws<InvalidCastException> (delegate {
				new ItemAutomationPeerPoker (new Slider ());
			});
		}

		[TestMethod]
		public void Protected ()
		{
			ContentControl cc = new ContentControl ();
			ItemAutomationPeerPoker iap = new ItemAutomationPeerPoker (cc);
			Assert.AreEqual (String.Empty, iap.GetNameCore_ (), "GetNameCore");
			Assert.AreEqual (String.Empty, iap.GetItemTypeCore_ (), "GetItemTypeCore");
			Assert.IsNull (iap.ItemsControlAutomationPeer_, "ItemsControlAutomationPeer");
			Assert.IsTrue (Object.ReferenceEquals (cc, iap.Item_), "Item");

			TextBlock textBlock = new TextBlock () { Text = "Name" };
			cc.Content = textBlock;
			Assert.AreEqual ("Name", iap.GetNameCore_ (), "GetNameCore");
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsSource ()
		{
			ListBox listbox = new ListBox ();
			List<Car> carList = new List<Car> ();
			carList.Add (new Car () { Name = "Ferrari", Price = 150000 });
			carList.Add (new Car () { Name = "Honda", Price = 12500 });
			carList.Add (new Car () { Name = "Toyota", Price = 11500 });
			listbox.DisplayMemberPath = "Name";
			listbox.ItemsSource = carList;

			CreateAsyncTest (listbox,
			() => {
				AutomationPeer listboxPeer = FrameworkElementAutomationPeer.CreatePeerForElement (listbox);
				List<AutomationPeer> children = listboxPeer.GetChildren ();
				Assert.IsNotNull (children, "#0");
				Assert.AreEqual (3, children.Count, "#1");

				Assert.AreEqual ("Ferrari 150000", children [0].GetName (), "#2");
				Assert.AreEqual ("Honda 12500", children [1].GetName (), "#3");
				Assert.AreEqual ("Toyota 11500", children [2].GetName (), "#4");
			});
		}

		[TestMethod]
		public override void GetItemType_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (string.Empty, feap.GetItemType (), "GetItemType");
			Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore");

			string itemType = "My Item Type";

			fe.SetValue (AutomationProperties.ItemTypeProperty, itemType);
			Assert.AreEqual (string.Empty, feap.GetItemType (), "GetItemType #1");
			Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore #1");

			fe.SetValue (AutomationProperties.ItemTypeProperty, null);
			Assert.AreEqual (string.Empty, feap.GetItemType (), "GetItemType #2");
			Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore #2");
		}

		[TestMethod]
		public override void GetName_AttachedProperty0 ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (string.Empty, feap.GetName (), "GetName");
			Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore");

			string name = "Attached Name";

			fe.SetValue (AutomationProperties.NameProperty, name);
			Assert.AreEqual (string.Empty, feap.GetName (), "GetName #1");
			Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore #1");

			fe.SetValue (AutomationProperties.NameProperty, null);
			Assert.AreEqual (string.Empty, feap.GetName (), "GetName #2");
			Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore #2");
		}

		[TestMethod]
		[Asynchronous]
		public override void GetName_AttachedProperty0Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			ContentControl fe = CreateConcreteFrameworkElement () as ContentControl;
			fe.Content = null;

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
				Assert.IsNull (tuple, "#1");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.SetValue (AutomationProperties.NameProperty, null);
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#2");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Content = "Hello!";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#3");
				Assert.AreEqual ("Hello!", (string) tuple.NewValue, "#4");
				Assert.AreEqual (string.Empty, (string) tuple.OldValue, "#5");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.SetValue (AutomationProperties.NameProperty, "Hi");
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNull (tuple, "#6");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Content = "Hey!";
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
				Assert.IsNotNull (tuple, "#7");
				Assert.AreEqual ("Hey!", (string) tuple.NewValue, "#8");
				Assert.AreEqual ("Hello!", (string) tuple.OldValue, "#9");
			});
		}

		[TestMethod]
		public override void GetName_AttachedProperty1 ()
		{
			FrameworkElement element = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract tbap = CreateConcreteFrameworkElementAutomationPeer (element);

			string textblockName = "Hello world:";
			string nameProperty = "Name property";

			TextBlock textblock = new TextBlock ();
			textblock.Text = textblockName;

			element.SetValue (AutomationProperties.NameProperty, nameProperty);
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #0");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #0");

			element.SetValue (AutomationProperties.LabeledByProperty, textblock);
			Assert.AreEqual (textblockName, tbap.GetName (), "GetName #1");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #1");

			textblock.Text = null;
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #2");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #2");

			textblock.Text = string.Empty;
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #3");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #3");

			element.SetValue (AutomationProperties.NameProperty, null);

			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #4");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #4");

			element.SetValue (AutomationProperties.LabeledByProperty, null);

			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #5");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #5");
		}

		[TestMethod]
		public override void GetName_AttachedProperty1Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			TextBlock textblock = new TextBlock () { Text = "Hello world:" };
			AutomationPeer textblockPeer = FrameworkElementAutomationPeer.CreatePeerForElement (textblock);

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.NameProperty, "My name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#1");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.LabeledByProperty, textblock);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#2");
			Assert.AreEqual ("Hello world:", (string) tuple.NewValue, "#3");
			Assert.AreEqual (string.Empty, (string) tuple.OldValue, "#4");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#5");
			Assert.AreEqual (textblock, tuple.NewValue, "#6");
			Assert.AreEqual (null, tuple.OldValue, "#7");

			EventsManager.Instance.Reset ();
			textblock.Text = null;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#8");
			Assert.AreEqual (string.Empty, (string) tuple.NewValue, "#9");
			Assert.AreEqual ("Hello world:", (string) tuple.OldValue, "#10");

			tuple = EventsManager.Instance.GetAutomationEventFrom (textblockPeer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#11");
			Assert.AreEqual (string.Empty, (string) tuple.NewValue, "#12");
			Assert.AreEqual ("Hello world:", (string) tuple.OldValue, "#13");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.LabeledByProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#14");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#15");
			Assert.AreEqual (null, tuple.NewValue, "#16");
			Assert.AreEqual (textblock, tuple.OldValue, "#17");
		}


		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "ItemAutomation is ContentElement.");

			bool contentControlLoaded = false;
			ContentControl contentControl = CreateConcreteFrameworkElement () as ContentControl;
			contentControl.Loaded += (o, e) => contentControlLoaded = true;
			TestPanel.Children.Add (contentControl);

			// StackPanel and two TextBlocks
			bool stackPanelLoaded = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
			stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			EnqueueConditional (() => contentControlLoaded, "ContentControlLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (contentControl);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				contentControl.Content = stackPanel;
			});
			EnqueueConditional (() => contentControlLoaded && stackPanelLoaded, "ContentControlLoaded #1");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (contentControl);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #1");
				// We add one TextBlock
				stackPanel.Children.Add (new TextBlock () { Text = "Text2" });
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #2");
				Assert.AreEqual (3, peer.GetChildren ().Count, "GetChildren.Count #2");
			});
			EnqueueTestComplete ();
		}

		public class MyListBox : ListBox {
			public ScrollViewer GetScrollViewer ()
			{
				return GetTemplateChild ("ScrollViewer") as ScrollViewer;
			}
		}

		[TestMethod]
		[Asynchronous]
		public override void GetParentTest ()
		{
			TestGetParent (new MyListBox ());
		}

		[TestMethod]
		[Asynchronous]
		public virtual void GetParentTest_NoTemplate ()
		{
			TestGetParent (new MyListBox () { Template = null });
		}

		protected void TestGetParent (MyListBox listbox)
		{
			listbox.Width = 100;
			listbox.Height = 100;
			bool layoutUpdated = false;
			bool loaded = false;
			listbox.LayoutUpdated += (o, e) => layoutUpdated = true;
			listbox.Loaded += (o, e) => loaded = true;

			TestPanel.Children.Add (listbox);
			AutomationPeer peer = null;
			AutomationPeer peerParent = null;
			ListBoxItem item0 = null;
			ListBoxItem item1 = null;
			List<AutomationPeer> children = null;

			EnqueueConditional (() => loaded, "Loaded #0");
			Enqueue (() => {
				item0 = new ListBoxItem () { Content = "Item 0" };
				item1 = new ListBoxItem () { Content = "Item 1" };

				peer = FrameworkElementAutomationPeer.CreatePeerForElement (listbox);
				Assert.IsNotNull (peer, "CreatePeerForElement #0");
				Assert.IsNull (peer.GetChildren (), "GetChildren #1");

				listbox.Items.Add (item0);
				listbox.Items.Add (item1);

				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #2");
				Assert.AreEqual (2, children.Count, "GetChildren #3");

				if (listbox.Template == null)
					peerParent = peer;
				else {
					// When default Template is used the Parent is the ScrollViewer 
					// not the ListBox
					ScrollViewer viewer = listbox.GetScrollViewer ();
					Assert.IsNotNull (viewer, "Missing ScrollViewer");
					if (viewer != null)
						peerParent = FrameworkElementAutomationPeer.CreatePeerForElement (viewer);
				}

				Assert.AreEqual (peerParent, children[0].GetParent (), "GetParent #1");
				Assert.AreEqual (peerParent, children[1].GetParent (), "GetParent #2");

				layoutUpdated = false;
				listbox.Items.Remove (item1);
			});
			EnqueueConditional (() => layoutUpdated, "LayoutUpdated #0");
			Enqueue (() => {
				Assert.IsNull (children [1].GetParent (), "GetParent #2");
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #4");
				Assert.AreEqual (1, peer.GetChildren ().Count, "GetChildren #5");
			});
			EnqueueTestComplete ();
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ConcreteContentControl ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ItemAutomationPeerPoker (element as ConcreteContentControl);
		}

		class Car {
			public string Name { get; set; }
			public int Price { get; set; }

			public override string ToString ()
			{
				return string.Format ("{0} {1}", Name, Price);
			}
		}
	}
}
