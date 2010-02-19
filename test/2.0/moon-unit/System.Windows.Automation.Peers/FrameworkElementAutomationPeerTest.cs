//
// Unit tests for FrameworkElementAutomationPeer
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
using System.Linq;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Reflection;
using System.Reflection.Emit;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Markup;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Windows.Media;
using System.Windows.Input;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class FrameworkElementAutomationPeerTest : SilverlightTest {
		[TestMethod]
		public void CreatePeerForElement ()
		{
			Assert.Throws<ArgumentNullException> (delegate {
				FrameworkElementAutomationPeer.CreatePeerForElement (null);
			}, "null");
		}

		public class ConcreteFrameworkElement : FrameworkElement {
			
			protected override AutomationPeer OnCreateAutomationPeer ()
			{
				return new FrameworkElementAutomationPeerPoker (this);
			}
		}

		[TestMethod]
		public virtual void Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				CreateConcreteFrameworkElementAutomationPeer (null);
			});
		}

		[TestMethod]
		public virtual void GetPattern ()
		{
			AutomationPeer feap = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());

			Assert.IsNull (feap.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (feap.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (feap.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (feap.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (feap.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (feap.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (feap.GetPattern (PatternInterface.Scroll), "Scroll");
			Assert.IsNull (feap.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (feap.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (feap.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (feap.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (feap.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (feap.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (feap.GetPattern (PatternInterface.Window), "Window");
		}

		public interface FrameworkElementAutomationPeerContract {
			AutomationPeer GetLabeledBy ();
			AutomationPeer GetLabeledByCore_ ();
			string GetName ();
			string GetNameCore_ ();
			bool IsContentElement ();
			bool IsContentElementCore_ ();
			bool IsControlElement ();
			bool IsControlElementCore_ ();
			string GetAcceleratorKey ();
			string GetAcceleratorKeyCore_ ();
			string GetAccessKey ();
			string GetAccessKeyCore_ ();
			AutomationControlType GetAutomationControlType ();
			AutomationControlType GetAutomationControlTypeCore_ ();
			string GetAutomationId ();
			string GetAutomationIdCore_ ();
			Rect GetBoundingRectangle ();
			Rect GetBoundingRectangleCore_ ();
			List<AutomationPeer> GetChildren ();
			List<AutomationPeer> GetChildrenCore_ ();
			Point GetClickablePoint ();
			Point GetClickablePointCore_ ();
			string GetHelpText ();
			string GetHelpTextCore_ ();
			string GetItemStatus ();
			string GetItemStatusCore_ ();
			string GetItemType ();
			string GetItemTypeCore_ ();
			string GetLocalizedControlType ();
			string GetLocalizedControlTypeCore_ ();
			AutomationOrientation GetOrientation ();
			AutomationOrientation GetOrientationCore_ ();
			bool HasKeyboardFocus ();
			bool HasKeyboardFocusCore_ ();
			bool IsEnabled ();
			bool IsEnabledCore_ ();
			bool IsKeyboardFocusable ();
			bool IsKeyboardFocusableCore_ ();
			bool IsOffscreen ();
			bool IsOffscreenCore_ ();
			bool IsPassword ();
			bool IsPasswordCore_ ();
			bool IsRequiredForForm ();
			bool IsRequiredForFormCore_ ();
			string GetClassName ();
			string GetClassNameCore_ ();
		}

		public class FrameworkElementAutomationPeerPoker : FrameworkElementAutomationPeer, FrameworkElementAutomationPeerContract {

			public FrameworkElementAutomationPeerPoker (FrameworkElement fe)
				: base (fe)
			{
			}

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

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}
		}

		[TestMethod]
		public virtual void GetAcceleratorKey ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, peer.GetAcceleratorKey (), "GetAcceleratorKey");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAcceleratorKeyCore_ (), "GetAcceleratorKeyCore");
		}

		[TestMethod]
		public virtual void GetAcceleratorKey_AttachedProperty()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (string.Empty, peer.GetAcceleratorKey (), "GetAcceleratorKey #0");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAcceleratorKeyCore_ (), "GetAcceleratorKeyCore #0");

			string acceleratorKey = "CTRL+C";

			fe.SetValue (AutomationProperties.AcceleratorKeyProperty, acceleratorKey);
			Assert.AreEqual (acceleratorKey, peer.GetAcceleratorKey (), "GetAcceleratorKey #1");
			if (feap != null)
				Assert.AreEqual (acceleratorKey, feap.GetAcceleratorKeyCore_ (), "GetAcceleratorKeyCore #1");

			fe.SetValue (AutomationProperties.AcceleratorKeyProperty, null);
			Assert.AreEqual (string.Empty, peer.GetAcceleratorKey (), "GetAcceleratorKey #2");
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAcceleratorKeyCore_ (), "GetAcceleratorKeyCore #2");
		}

		[TestMethod]
		public virtual void GetAcceleratorKey_AttachedPropertyEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AcceleratorKeyProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AcceleratorKeyProperty, "CTRL+C");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AcceleratorKeyProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("CTRL+C", (string) tuple.NewValue, "#2");
			Assert.AreEqual (null, (string) tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AcceleratorKeyProperty, "CTRL+V");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AcceleratorKeyProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("CTRL+V", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("CTRL+C", (string) tuple.OldValue, "#6");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AcceleratorKeyProperty, null);

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AcceleratorKeyProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (null, (string) tuple.NewValue, "#8");
			Assert.AreEqual ("CTRL+V", (string) tuple.OldValue, "#9");
		}

		[TestMethod]
		public virtual void GetClassName ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, peer.GetClassName (), "GetClassNameCore");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public virtual void GetAccessKey ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, peer.GetAccessKey (), "GetAccessKey");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAccessKeyCore_ (), "GetAccessKeyCore");
		}

		[TestMethod]
		public virtual void GetAccessKey_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (string.Empty, peer.GetAccessKey (), "GetAccessKey");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAccessKeyCore_ (), "GetAccessKeyCore");

			string accessKey = "ALT+C";

			fe.SetValue (AutomationProperties.AccessKeyProperty, accessKey);
			Assert.AreEqual (accessKey, peer.GetAccessKey (), "GetAccessKey #1");
			if (feap != null)
				Assert.AreEqual (accessKey, feap.GetAccessKeyCore_ (), "GetAccessKeyCore #1");

			fe.SetValue (AutomationProperties.AccessKeyProperty, null);
			Assert.AreEqual (string.Empty, peer.GetAccessKey (), "GetAccessKey #2");
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAccessKeyCore_ (), "GetAccessKeyCore #2");
		}

		[TestMethod]
		public virtual void GetAccessKey_AttachedPropertyEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AccessKeyProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AccessKeyProperty, "ALT+C");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AccessKeyProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("ALT+C", (string) tuple.NewValue, "#2");
			Assert.AreEqual (null, (string) tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AccessKeyProperty, "ALT+B");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AccessKeyProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("ALT+B", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("ALT+C", (string) tuple.OldValue, "#6");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AccessKeyProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AccessKeyProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (null, (string) tuple.NewValue, "#8");
			Assert.AreEqual ("ALT+B", (string) tuple.OldValue, "#9");
		}

		[TestMethod]
		public virtual void GetAutomationControlType ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.Custom, peer.GetAutomationControlType (), "GetAutomationControlType");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (AutomationControlType.Custom, feap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public virtual void GetAutomationId ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, peer.GetAutomationId (), "GetAutomationId");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAutomationIdCore_ (), "GetAutomationIdCore");
		}

		[TestMethod]
		public virtual void GetAutomationId_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (string.Empty, peer.GetAutomationId (), "GetAutomationId");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAutomationIdCore_(), "GetAutomationIdCore");

			string automationId = "MyAttachedAutomationId";

			fe.SetValue(AutomationProperties.AutomationIdProperty, automationId);
			Assert.AreEqual (automationId, peer.GetAutomationId (), "GetAutomationId #1");
			if (feap != null)
				Assert.AreEqual (automationId, feap.GetAutomationIdCore_(), "GetAutomationIdCore #1");

			fe.SetValue (AutomationProperties.AutomationIdProperty, null);
			Assert.AreEqual (string.Empty, peer.GetAutomationId (), "GetAutomationId #2");
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetAutomationIdCore_ (), "GetAutomationIdCore #2");
		}

		[TestMethod]
		public virtual void GetAutomationId_AttachedPropertyEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AutomationIdProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AutomationIdProperty, "MyAttachedAutomationId");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AutomationIdProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("MyAttachedAutomationId", (string) tuple.NewValue, "#2");
			Assert.AreEqual (null, (string) tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AutomationIdProperty, "OtherId");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AutomationIdProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("OtherId", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("MyAttachedAutomationId", (string) tuple.OldValue, "#6");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.AutomationIdProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.AutomationIdProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (null, tuple.NewValue, "#8");
			Assert.AreEqual ("OtherId", (string) tuple.OldValue, "#9");
		}

		[TestMethod]
		public virtual void GetBoundingRectangle ()
		{
			AutomationPeer peer
				= FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			FrameworkElementAutomationPeerContract feap
				= peer as FrameworkElementAutomationPeerContract;

			Rect boundingRectangle = peer.GetBoundingRectangle ();
			Assert.AreEqual (0, boundingRectangle.X, "GetBoundingRectangle X");
			Assert.AreEqual (0, boundingRectangle.Y, "GetBoundingRectangle Y");
			Assert.AreEqual (0, boundingRectangle.Width, "GetBoundingRectangle Width");
			Assert.AreEqual (0, boundingRectangle.Height, "GetBoundingRectangle Height");

			if (feap != null) {
				boundingRectangle = feap.GetBoundingRectangleCore_ ();
				Assert.AreEqual (0, boundingRectangle.X, "GetBoundingRectangleCore X");
				Assert.AreEqual (0, boundingRectangle.Y, "GetBoundingRectangleCore Y");
				Assert.AreEqual (0, boundingRectangle.Width, "GetBoundingRectangleCore Width");
				Assert.AreEqual (0, boundingRectangle.Height, "GetBoundingRectangleCore Height");
			}

			Assert.AreNotSame (Rect.Empty, boundingRectangle, "GetBoundingRectangleCore Isempty");
		}

		[TestMethod]
		public virtual void GetChildren ()
		{
			AutomationPeer peer 
				= FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (null, peer.GetChildren (), "GetChildren");
			FrameworkElementAutomationPeerContract feap
				= peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (null, feap.GetChildrenCore_ (), "GetChildrenCore");
		}

		[TestMethod]
		[Asynchronous]
		public virtual void GetChildrenChanged ()
		{
			if (!IsContentPropertyElement ()
			    || !EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			ContentControl control = CreateConcreteFrameworkElement () as ContentControl;
			if (control == null) {
				// Other ContentPropertyAttribute decorated classes exposing AutomationPeer
				// are alredy testing StructureChanged: ItemsControl and subclasses.
				EnqueueTestComplete ();
				return;
			}

			AutomationPeer peer = null;
			List<AutomationPeer> children = null;
			Button button = null;
			Canvas canvas = new Canvas ();
			StackPanel stackPanel = new StackPanel ();
			Button stackPanelButton = new Button ();
			CheckBox checkbox = new CheckBox ();
			Grid grid = new Grid ();
			TextBlock textblock = new TextBlock ();
			TextBox textbox = new TextBox ();
			Button gridButton = new Button ();
			AutomationEventTuple tuple = null;

			CreateAsyncTest (control,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
				Assert.IsNotNull (peer, "Peer");
				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
			},
			() => {
				button = new Button () { Content = "Button0" };
				control.Content = button;
				// Control hierarchy: control { button }
				// UIA hierarchy: control { button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #1");
				Assert.AreEqual (1, children.Count, "Children.Count #0");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children [0], "GetChildren #2");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #0");
				EventsManager.Instance.Reset ();
			},
			() => control.Content = null,
			() => {
				Assert.IsNull (peer.GetChildren (), "GetChildren #3");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #1");
				EventsManager.Instance.Reset ();

				// Panel subclasses don't expose any AutomationPeer but chidren's
				control.Content = canvas;
				// Control hierarchy: control { canvas }
				// UIA hierarchy: control { }

			},
			() => {
				Assert.IsNull (peer.GetChildren (), "GetChildren #4");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #2");
				EventsManager.Instance.Reset ();
				canvas.Children.Add (stackPanel);
				// Control hierarchy: control { canvas { stackpanel } }
				// UIA hierarchy: control { }
			},
			() => {
				Assert.IsNull (peer.GetChildren (), "GetChildren #5");
				canvas.Children.Add (button);
				// Control hierarchy: control { canvas { stackapanel, button } }
				// UIA hierarchy: control { button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #6");
				Assert.AreEqual (1, children.Count, "Children.Count #1");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children [0], "GetChildren #7");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #3");
				EventsManager.Instance.Reset ();

				stackPanel.Children.Add (checkbox);
				// Control hierarchy: control { canvas { stackPanel { checkbox }, button } }
				// UIA hierarchy: control { checkbox, button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #8");
				Assert.AreEqual (2, children.Count, "Children.Count #2");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox),
					children [0], "GetChildren #9");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children [1], "GetChildren #10");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #4");
				EventsManager.Instance.Reset ();

				stackPanel.Children.Add (grid);
				// Control hierarchy: control { canvas { stackPanel { checkbox, grid }, button } }
				// UIA hierarchy: control { checkbox, button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #11");
				Assert.AreEqual (2, children.Count, "Children.Count #3");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox),
					children [0], "GetChildren #12");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children [1], "GetChildren #13");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #5");
				EventsManager.Instance.Reset ();

				grid.Children.Add (textblock);
				// Control hierarchy: control { canvas { stackPanel { checkbox, grid { textblock } }, button } }
				// UIA hierarchy: + control { checkbox, textblock, button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #14");
				Assert.AreEqual (3, children.Count, "Children.Count #4");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox),
					children [0], "GetChildren #15");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (textblock),
					children [1], "GetChildren #16");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children [2], "GetChildren #17");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #6");
				EventsManager.Instance.Reset ();

				canvas.Children.Add (textbox);
				// Control hierarchy: control { canvas { stackPanel { checkbox, grid { textblock } }, button, textbox } }
				// UIA hierarchy: control { checkbox, textblock, button, textbox }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #18");
				Assert.AreEqual (4, children.Count, "Children.Count #5");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox),
					children [0], "GetChildren #19");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (textblock),
					children [1], "GetChildren #20");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children [2], "GetChildren #21");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (textbox),
					children [3], "GetChildren #22");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #7");
				EventsManager.Instance.Reset ();

				stackPanel.Children.Clear ();
				// Control hierarchy: control { canvas { stackPanel { }, button, textbox } }
				// UIA hierarchy: control { button, textbox }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #23");
				Assert.AreEqual (2, children.Count, "Children.Count #6");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children [0], "GetChildren #26");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (textbox),
					children [1], "GetChildren #27");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #8");
				EventsManager.Instance.Reset ();

				// If we modify 'grid' no event should be raised, since is not part of 'canvas' anymore
				grid.Children.Add (gridButton);
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #9");
				EventsManager.Instance.Reset ();
				canvas.Children.Clear ();
				// Control hierarchy: control { canvas { } }
				// UIA hierarchy: control { }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNull (children, "GetChildren #28");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #10");
				EventsManager.Instance.Reset ();

				// If we modify 'stackPanel' no event should be raised, since is not part of 'canvas' anymore
				stackPanel.Children.Add (stackPanelButton);
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #11");
				EventsManager.Instance.Reset ();

				stackPanel.Children.Add (grid);
				// We have now: stackpanel { stackPanelButton, grid { textblock, gridButton } }
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #12");
				EventsManager.Instance.Reset ();

				// Event should not be raised
				control.Content = null;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #13");
				EventsManager.Instance.Reset ();

				control.Content = stackPanel;
				// Control hierarchy: canvas { stackpanel { stackPanelButton, grid { textblock, gridButton } } }
				// UIA hierarchy: control { stackPanelButton, textblock, gridButton }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #29");
				Assert.AreEqual (3, children.Count, "Children.Count #7");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (stackPanelButton),
					children [0], "GetChildren #30");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (textblock),
					children [1], "GetChildren #31");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (gridButton),
					children [2], "GetChildren #32");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #14");
				EventsManager.Instance.Reset ();

				grid.Children.Add (checkbox);
				// Control hierarchy: canvas { stackpanel { stackPanelButton, grid { textblock, gridButton, checkbox } } }
				// UIA hierarchy: control { stackPanelButton, textblock, gridButton, checkbox }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #33");
				Assert.AreEqual (4, children.Count, "Children.Count #8");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (stackPanelButton),
					children [0], "GetChildren #34");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (textblock),
					children [1], "GetChildren #35");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (gridButton),
					children [2], "GetChildren #36");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox),
					children [3], "GetChildren #37");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #15");
				EventsManager.Instance.Reset ();
			});
		}

		[TestMethod]
		public virtual void GetClickablePoint ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (0, peer.GetClickablePoint ().X, "GetClickablePoint X");
			Assert.AreEqual (0, peer.GetClickablePoint ().Y, "GetClickablePoint Y");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null) {
				Assert.AreEqual (0, feap.GetClickablePointCore_ ().X, "GetClickablePointCore X");
				Assert.AreEqual (0, feap.GetClickablePointCore_ ().Y, "GetClickablePointCore Y");
			}
		}

		[TestMethod]
		public virtual void GetHelpText ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, peer.GetHelpText (), "GetHelpText");

			FrameworkElementAutomationPeerContract feap
				= peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetHelpTextCore_ (), "GetHelpTextCore");
		}

		[TestMethod]
		public virtual void GetHelpText_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (string.Empty, peer.GetHelpText (), "GetHelpText");
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetHelpTextCore_ (), "GetHelpTextCore");

			string helpText = "My Help Text property";

			fe.SetValue(AutomationProperties.HelpTextProperty, helpText);
			Assert.AreEqual (helpText, peer.GetHelpText (), "GetHelpText #1");
			if (feap != null)
				Assert.AreEqual (helpText, feap.GetHelpTextCore_ (), "GetHelpTextCore #1");

			fe.SetValue (AutomationProperties.HelpTextProperty, null);
			Assert.AreEqual (string.Empty, peer.GetHelpText (), "GetHelpText #2");
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetHelpTextCore_ (), "GetHelpTextCore #2");
		}

		[TestMethod]
		public virtual void GetHelpText_AttachedPropertyEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.HelpTextProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.HelpTextProperty, "My Help Text property");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.HelpTextProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("My Help Text property", (string) tuple.NewValue, "#2");
			Assert.AreEqual (null, (string) tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.HelpTextProperty, "No Help");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.HelpTextProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("No Help", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("My Help Text property", (string) tuple.OldValue, "#6");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.HelpTextProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.HelpTextProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (null, tuple.NewValue, "#8");
			Assert.AreEqual ("No Help", (string) tuple.OldValue, "#9");
		}
		
		[TestMethod]
		public virtual void GetItemStatus ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, peer.GetItemStatus (), "GetItemStatus");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetItemStatusCore_ (), "GetItemStatusCore");
		}

		[TestMethod]
		public virtual void GetItemStatus_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			Assert.AreEqual (string.Empty, peer.GetItemStatus (), "GetItemStatus");
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetItemStatusCore_ (), "GetItemStatusCore");

			string itemStatus = "My Item Status";

			fe.SetValue (AutomationProperties.ItemStatusProperty, itemStatus);
			Assert.AreEqual (itemStatus, peer.GetItemStatus (), "GetItemStatus #1");
			if (feap != null)
				Assert.AreEqual (itemStatus, feap.GetItemStatusCore_ (), "GetItemStatusCore #1");

			fe.SetValue (AutomationProperties.ItemStatusProperty, null);
			Assert.AreEqual (string.Empty, peer.GetItemStatus (), "GetItemStatus #2");
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetItemStatusCore_ (), "GetItemStatusCore #2");
		}

		[TestMethod]
		public virtual void GetItemStatus_AttachedPropertyEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.ItemStatusProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.ItemStatusProperty, "My Item Status");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.ItemStatusProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("My Item Status", (string) tuple.NewValue, "#2");
			Assert.AreEqual (null, tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.ItemStatusProperty, "No Item Status");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.ItemStatusProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("No Item Status", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("My Item Status", tuple.OldValue, "#6");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.ItemStatusProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.ItemStatusProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (null, (string) tuple.NewValue, "#8");
			Assert.AreEqual ("No Item Status", tuple.OldValue, "#9");
		}

		[TestMethod]
		public virtual void GetItemType ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, peer.GetItemType (), "GetItemType");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore");
		}

		[TestMethod]
		public virtual void GetItemType_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (string.Empty, peer.GetItemType (), "GetItemType");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore");

			string itemType = "My Item Type";

			fe.SetValue (AutomationProperties.ItemTypeProperty, itemType);
			Assert.AreEqual (itemType, peer.GetItemType (), "GetItemType #1");
			if (feap != null)
				Assert.AreEqual (itemType, feap.GetItemTypeCore_ (), "GetItemTypeCore #1");

			fe.SetValue (AutomationProperties.ItemTypeProperty, null);
			Assert.AreEqual (string.Empty, peer.GetItemType (), "GetItemType #2");
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore #2");
		}

		[TestMethod]
		public virtual void GetItemType_AttachedPropertyEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.ItemTypeProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.ItemTypeProperty, "My Item Type");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.ItemTypeProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("My Item Type", (string) tuple.NewValue, "#2");
			Assert.AreEqual (null, (string) tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.ItemTypeProperty, "No type");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.ItemTypeProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("No type", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("My Item Type", (string) tuple.OldValue, "#6");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.ItemTypeProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.ItemTypeProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (null, (string) tuple.NewValue, "#8");
			Assert.AreEqual ("No type", (string) tuple.OldValue, "#9");
		}

		[TestMethod]
		public virtual void GetLocalizedControlType ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			string localizedString = peer.GetAutomationControlType ().ToString ().ToLower();
			Assert.AreEqual (localizedString, peer.GetLocalizedControlType(), 
					string.Format ("GetLocalizedControlType: {0}", localizedString));

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (localizedString, feap.GetLocalizedControlTypeCore_(), 
						string.Format ("GetLocalizedControlTypeCore: {0}", localizedString));
		}

		[TestMethod]
		public virtual void GetOrientation ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationOrientation.None, peer.GetOrientation (), "GetOrientation");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (AutomationOrientation.None, feap.GetOrientationCore_ (), "GetOrientationCore");
		}

		[TestMethod]
		[Asynchronous]
		[Ignore("This is working, but I can't test it because the green progressbar steals focus!")]
		public virtual void HasKeyboardFocus ()
		{
			FrameworkElement concreteFrameworkElement = CreateConcreteFrameworkElement ();
			Control control = concreteFrameworkElement as Control;

			if (control == null) {
				// Some FrameworkElement subclasses are sealed, so we are doing this.
				AutomationPeer peer 
					= FrameworkElementAutomationPeer.CreatePeerForElement (concreteFrameworkElement);
				FrameworkElementAutomationPeerContract feap
					= peer as FrameworkElementAutomationPeerContract;
				Assert.IsFalse (peer.HasKeyboardFocus (), "HasKeyboardFocus #0");
				if (feap != null)
					Assert.IsFalse (feap.HasKeyboardFocusCore_ (), "HasKeyboardFocusCore #0");
			} else {
				if (!control.IsTabStop || !control.IsEnabled)
					return;

				bool controlLoaded = false;
				bool panelLoaded = false;
				bool focused = false;
				control.Height = 30;
				control.Loaded += (o, e) => { controlLoaded = true; };
				control.GotFocus += (o, e) => { focused = true; };
				control.LostFocus += (o, e) => { focused = false; };

				StackPanel panel = new StackPanel ();
				panel.Loaded += (o, e) => { panelLoaded = true; };

				Control control1 = CreateConcreteFrameworkElement () as Control; // To allow raising LostFocus
				control1.Height = 30;
				
				//control.TabIndex = 0;
				//control1.TabIndex = 1;

				bool control1Focused = false;
				control1.GotFocus += (o, e) => control1Focused = true;

				panel.Children.Add (control);
				panel.Children.Add (control1);

				TestPanel.Children.Add (panel);

				EnqueueConditional (() => controlLoaded && panelLoaded, "ControlLoaded #0");
				Enqueue (() => {
					AutomationPeer peer 
						= FrameworkElementAutomationPeer.CreatePeerForElement (concreteFrameworkElement);
					Assert.IsFalse (peer.HasKeyboardFocus (), "HasKeyboardFocus #1");

					control1.Focus ();
					FocusManager.GetFocusedElement (); // To synchronize call

					if (EventsManager.Instance.AutomationSingletonExists)
						EventsManager.Instance.Reset ();

					control.Focus ();
					FocusManager.GetFocusedElement (); // To synchronize call

					Assert.IsTrue (peer.HasKeyboardFocus (), "HasKeyboardFocus #2");

					if (EventsManager.Instance.AutomationSingletonExists) {
						AutomationPropertyEventTuple tuple
							= EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.HasKeyboardFocusProperty);
						Assert.IsNotNull (tuple, "Event #0");
						Assert.IsTrue ((bool) tuple.NewValue, "Event #1");
						Assert.IsFalse ((bool) tuple.OldValue, "Event #2");
					}
				});
			}
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public virtual void IsKeyboardFocusable ()
		{
			TestIsKeyboardFocusable ();
		}

		[TestMethod]
		[Asynchronous]
		public virtual void IsKeyboardFocusable_Event ()
		{
			TestIsKeyboardFocusableEvent ();
		}

		[TestMethod]
		public virtual void IsEnabled ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.IsTrue (peer.IsEnabled (), "IsEnabled");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.IsTrue (feap.IsEnabledCore_ (), "IsEnabledCore");
		}

		[TestMethod]
		[Asynchronous]
		public virtual void IsOffScreen ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			bool controlLoaded = false;
			bool layoutUpdated = false;
			fe.Loaded += (o, e) => controlLoaded = true;
			fe.LayoutUpdated += (o, e) => layoutUpdated = true;
			AutomationPeer peer = null;
			ContentControl contentControl = new ContentControl ();
			StackPanel stackPanel = new StackPanel ();
			contentControl.Content = stackPanel;
			stackPanel.Children.Add (fe);
			TestPanel.Children.Add (contentControl);
			// Grid -> StackPanel -> Control
			EnqueueConditional (() => controlLoaded && layoutUpdated , "ControlLoaded #0");
			Enqueue (() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #1");
				layoutUpdated = false;
			});
			Enqueue (() => fe.Visibility = Visibility.Collapsed);
			EnqueueConditional (() => controlLoaded && layoutUpdated, "ControlLoaded #1");
			Enqueue (() => {
				Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #2");
				layoutUpdated = false;
			});
			Enqueue (() => fe.Visibility = Visibility.Visible);
			EnqueueConditional (() => controlLoaded && layoutUpdated, "ControlLoaded #2");
			Enqueue (() => {
				Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #3");
			});
			// If we are Visible but our Parent is not, we should be offscreen too
			Enqueue (() => contentControl.Visibility = Visibility.Collapsed);
			Enqueue (() => { Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #4"); });
			Enqueue (() => fe.Visibility = Visibility.Collapsed);
			Enqueue (() => { Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #5"); });
			Enqueue (() => contentControl.Visibility = Visibility.Visible);
			Enqueue (() => { Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #6"); });
			Enqueue (() => fe.Visibility = Visibility.Visible);
			Enqueue (() => { Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #7"); });
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public virtual void IsOffScreen_ScrollViewer ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			Control control = fe as Control;
			
			if (control == null) {
				EnqueueTestComplete ();
				return;
			}

			ScrollViewer scrollViewer = new ScrollViewer () { Height = 100 };
			StackPanel panel = new StackPanel ();
			scrollViewer.Content = panel;
			AutomationPeer peer = null;

			CreateAsyncTest (scrollViewer,
			() => {
				for (int i = 0; i < 30; i++)
					panel.Children.Add (new TextBlock () { Text = i.ToString () });
				// Our control won't be visible, but still won't be offscreen
				panel.Children.Add (control);
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
				Assert.IsNotNull (peer, "#0");
			},
			() => Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #1"),
			() => control.Visibility = Visibility.Collapsed,
			() => Assert.IsTrue (peer.IsOffscreen (), "IsOffScreen #2"),
			() => control.Visibility = Visibility.Visible,
			() => Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #3")
			);
		}

		[TestMethod]
		[Asynchronous]
		public virtual void IsOffScreen_Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			fe.SetValue (Canvas.TopProperty, (double) 10);
			fe.SetValue (Canvas.LeftProperty, (double) 30);
			fe.SetValue (Canvas.WidthProperty, (double) 150);
			fe.SetValue (Canvas.HeightProperty, (double) 230);

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			CreateAsyncTest (fe,
			() => {
				EventsManager.Instance.Reset ();
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNull (tuple, "#0");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #0");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Visibility = Visibility.Visible;
			},
			() => {
				EventsManager.Instance.Reset ();
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNull (tuple, "#1");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #1");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Visibility = Visibility.Collapsed;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNotNull (tuple, "#2");
				Rect newValue = (Rect) tuple.NewValue;

				Assert.AreEqual (0, newValue.X, "#4");
				Assert.AreEqual (0, newValue.Y, "#5");
				Assert.AreEqual (0, newValue.Width, "#6");
				Assert.AreEqual (0, newValue.Height, "#7");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNotNull (tuple, "IsOffscreen #2");
				Assert.IsFalse ((bool) tuple.OldValue, "IsOffscreen #3");
				Assert.IsTrue ((bool) tuple.NewValue, "IsOffscreen #4");
			},
			() => {
				EventsManager.Instance.Reset ();
				fe.Visibility = Visibility.Visible;
			},
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.BoundingRectangleProperty);
				Assert.IsNotNull (tuple, "#8");
				Rect newValue = (Rect) tuple.NewValue;
				Rect oldValue = (Rect) tuple.OldValue;

				Assert.AreNotEqual (newValue.X, oldValue.X, "#9");
				Assert.AreNotEqual (newValue.Y, oldValue.Y, "#10");
				Assert.AreNotEqual (newValue.Width, oldValue.Width, "#11");
				Assert.AreNotEqual (newValue.Height, oldValue.Height, "#12");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNotNull (tuple, "IsOffscreen #5");
				Assert.IsFalse ((bool) tuple.NewValue, "IsOffscreen #6");
				Assert.IsTrue ((bool) tuple.OldValue, "IsOffscreen #7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public virtual void IsOffScreen_Event1 ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			bool parent0Loaded = false;

			Canvas parent0 = new Canvas ();
			parent0.Height = 100;
			parent0.Width = 100;
			parent0.Loaded += (o, e) => parent0Loaded = true;

			StackPanel parent1 = new StackPanel ();
			parent1.Height = 200;
			parent1.Width = 100;

			parent0.Children.Add (parent1);

			FrameworkElement fe = CreateConcreteFrameworkElement ();

			fe.SetValue (Canvas.WidthProperty, (double) 150);
			fe.SetValue (Canvas.HeightProperty, (double) 230);
			parent1.Children.Add (fe);

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			TestPanel.Children.Add (parent0);

			EnqueueConditional (() => parent0Loaded, "Loaded #0");
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				parent0.Visibility = Visibility.Visible;
			});
			Enqueue (() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #4X");
			});
			Enqueue (() => {
				// Testing when our parent is not Visible
				EventsManager.Instance.Reset ();
				parent0.Visibility = Visibility.Collapsed;
			});
			Enqueue (() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNotNull (tuple, "IsOffscreen #1");
				Assert.IsTrue ((bool) tuple.NewValue, "IsOffscreen #2");
				Assert.IsFalse ((bool) tuple.OldValue, "IsOffscreen #3");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				parent1.Visibility = Visibility.Collapsed;
			});
			Enqueue (() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #4");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				parent0.Visibility = Visibility.Visible;
			});
			Enqueue (() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNull (tuple, "IsOffscreen #5");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				parent1.Visibility = Visibility.Visible;
			});
			Enqueue (() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsOffscreenProperty);
				Assert.IsNotNull (tuple, "IsOffscreen #7");
				Assert.IsFalse ((bool) tuple.NewValue, "IsOffscreen #8");
				Assert.IsTrue ((bool) tuple.OldValue, "IsOffscreen #9");
			});
			EnqueueTestComplete ();
		}
		 
		[TestMethod]
		public virtual void IsPassword ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			Assert.IsFalse (peer.IsPassword (), "IsPassword");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.IsFalse (feap.IsPasswordCore_ (), "IsPasswordCore");
		}

		[TestMethod]
		public virtual void IsRequiredForForm ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			Assert.IsFalse (peer.IsRequiredForForm (), "IsRequiredForForm");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.IsFalse (feap.IsRequiredForFormCore_ (), "IsRequiredForFormCore");
		}

		[TestMethod]
		public virtual void IsRequiredForForm_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.IsFalse (peer.IsRequiredForForm (), "IsRequiredForForm");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.IsFalse (feap.IsRequiredForFormCore_ (), "IsRequiredForFormCore");

			fe.SetValue (AutomationProperties.IsRequiredForFormProperty, true);
			Assert.IsTrue (peer.IsRequiredForForm (), "IsRequiredForForm #1");
			if (feap != null)
				Assert.IsTrue (feap.IsRequiredForFormCore_ (), "IsRequiredForFormCore #1");

			fe.SetValue (AutomationProperties.IsRequiredForFormProperty, false);
			Assert.IsFalse (peer.IsRequiredForForm (), "IsRequiredForForm #2");
			if (feap != null)
				Assert.IsFalse (feap.IsRequiredForFormCore_ (), "IsRequiredForFormCore #2");
		}

		[TestMethod]
		public virtual void IsRequiredForForm_AttachedPropertyEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsRequiredForFormProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.IsRequiredForFormProperty, true);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsRequiredForFormProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.IsTrue ((bool) tuple.NewValue, "#2");
			Assert.IsFalse ((bool) tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.IsRequiredForFormProperty, false);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.IsRequiredForFormProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.IsFalse ((bool) tuple.NewValue, "#5");
			Assert.IsTrue ((bool) tuple.OldValue, "#6");
		}

		[TestMethod]
		public virtual void GetName ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			Assert.AreEqual (String.Empty, peer.GetName (), "GetName");

			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.AreEqual (String.Empty, feap.GetNameCore_ (), "GetNameCore");
		}

		[TestMethod]
		public virtual void GetName_AttachedProperty0 ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.AreEqual (string.Empty, peer.GetName (), "GetName");
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore");

			string name = "Attached Name";

			fe.SetValue (AutomationProperties.NameProperty, name);
			Assert.AreEqual (name, peer.GetName (), "GetName #1");
			if (feap != null)
				Assert.AreEqual (name, feap.GetNameCore_ (), "GetNameCore #1");

			fe.SetValue (AutomationProperties.NameProperty, null);
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #2");
			if (feap != null)
				Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore #2");
		}

		[TestMethod]
		public virtual void GetName_AttachedProperty0Event ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.NameProperty, "Attached Name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("Attached Name", (string) tuple.NewValue, "#2");
			Assert.AreEqual (string.Empty, tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.NameProperty, "Name");
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("Name", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("Attached Name", (string) tuple.OldValue, "#6");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.NameProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (string.Empty, (string) tuple.NewValue, "#8");
			Assert.AreEqual ("Name", (string) tuple.OldValue, "#9");
		}

		[TestMethod]
		public virtual void GetName_AttachedProperty1 ()
		{
			FrameworkElement element = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);

			string textblockName = "Hello world:";
			string nameProperty = "TextBox name";

			TextBlock textblock = new TextBlock ();
			textblock.Text = textblockName;

			element.SetValue (AutomationProperties.NameProperty, nameProperty);
			Assert.AreEqual (nameProperty, peer.GetName (), "GetName #0");
			FrameworkElementAutomationPeerContract tbap = peer as FrameworkElementAutomationPeerContract;
			if (tbap != null)
				Assert.AreEqual (nameProperty, tbap.GetNameCore_ (), "GetNameCore #0");

			element.SetValue (AutomationProperties.LabeledByProperty, textblock);
			Assert.AreEqual (textblockName, peer.GetName (), "GetName #1");
			if (tbap != null)
				Assert.AreEqual (nameProperty, tbap.GetNameCore_ (), "GetNameCore #1");

			textblock.Text = null;
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #2");
			if (tbap != null)
				Assert.AreEqual (nameProperty, tbap.GetNameCore_ (), "GetNameCore #2");

			textblock.Text = string.Empty;
			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #3");
			if (tbap != null)
				Assert.AreEqual (nameProperty, tbap.GetNameCore_ (), "GetNameCore #3");

			element.SetValue (AutomationProperties.NameProperty, null);

			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #4");
			if (tbap != null)
				Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #4");

			element.SetValue (AutomationProperties.LabeledByProperty, null);

			Assert.AreEqual (string.Empty, peer.GetName (), "GetName #5");
			if (tbap != null)
				Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #5");
		}

		[TestMethod]
		public virtual void GetName_AttachedProperty1Event ()
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
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual ("My name", (string) tuple.NewValue, "#2");
			Assert.AreEqual (string.Empty, (string) tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.LabeledByProperty, textblock);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual ("Hello world:", (string) tuple.NewValue, "#5");
			Assert.AreEqual ("My name", (string) tuple.OldValue, "#6");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#7");
			Assert.AreEqual (textblock, tuple.NewValue, "#8");
			Assert.AreEqual (null, tuple.OldValue, "#9");

			EventsManager.Instance.Reset ();
			textblock.Text = null;
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#10");
			Assert.AreEqual (string.Empty, (string) tuple.NewValue, "#11");
			Assert.AreEqual ("Hello world:", (string) tuple.OldValue, "#12");

			tuple = EventsManager.Instance.GetAutomationEventFrom (textblockPeer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#13");
			Assert.AreEqual (string.Empty, (string) tuple.NewValue, "#14");
			Assert.AreEqual ("Hello world:", (string) tuple.OldValue, "#15");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.LabeledByProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.NameProperty);
			Assert.IsNotNull (tuple, "#16");
			Assert.AreEqual ("My name", (string) tuple.NewValue, "#17");
			Assert.AreEqual (string.Empty, (string) tuple.OldValue, "#18");

			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#19");
			Assert.AreEqual (null, tuple.NewValue, "#20");
			Assert.AreEqual (textblock, tuple.OldValue, "#21");
		}

		[TestMethod]
		public virtual void GetLabeledBy ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.IsNull (peer.GetLabeledBy (), "GetLabeledBy");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore");
		}

		[TestMethod]
		public virtual void GetLabeledBy_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);

			Assert.IsNull (peer.GetLabeledBy (), "GetLabeledBy");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore");

			FrameworkElement labeledBy = new TextBlock ();
			AutomationPeer labeledByPeer = FrameworkElementAutomationPeer.CreatePeerForElement (labeledBy);

			fe.SetValue (AutomationProperties.LabeledByProperty, labeledBy);
			Assert.AreSame (labeledByPeer, peer.GetLabeledBy (), "GetLabeledBy #1");
			if (feap != null)
				Assert.AreSame (labeledByPeer, feap.GetLabeledByCore_(), "GetLabeledByCore #1");

			fe.SetValue (AutomationProperties.LabeledByProperty, null);
			Assert.IsNull (peer.GetLabeledBy (), "GetLabeledBy #2");
			if (feap != null)
				Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore #2");
		}

		[TestMethod]
		public virtual void GetLabeledBy_AttachedPropertyEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			FrameworkElement fe = CreateConcreteFrameworkElement ();
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			AutomationPropertyEventTuple tuple = null;
			FrameworkElement labeledBy = new TextBlock ();

			EventsManager.Instance.Reset ();
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNull (tuple, "#0");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.LabeledByProperty, labeledBy);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#1");
			Assert.AreEqual (labeledBy, tuple.NewValue, "#2");
			Assert.AreEqual (null, tuple.OldValue, "#3");

			EventsManager.Instance.Reset ();
			fe.SetValue (AutomationProperties.LabeledByProperty, null);
			tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationElementIdentifiers.LabeledByProperty);
			Assert.IsNotNull (tuple, "#4");
			Assert.AreEqual (null, tuple.NewValue, "#5");
			Assert.AreEqual (labeledBy, tuple.OldValue, "#6");
		}

		[TestMethod]
		public virtual void IsContentElement ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.IsTrue (peer.IsContentElement (), "IsContentElement");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.IsTrue (feap.IsContentElementCore_ (), "IsContentElementCore");
		}

		[TestMethod]
		public virtual void IsControlElement ()
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (CreateConcreteFrameworkElement ());
			Assert.IsTrue (peer.IsControlElement (), "IsControlElement");
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;
			if (feap != null)
				Assert.IsTrue (feap.IsControlElementCore_ (), "IsControlElementCore");
		}
		
		[TestMethod]
		public void CreatePeer ()
		{
			FrameworkElement b = CreateConcreteFrameworkElement ();
			AutomationPeer peer1 = FrameworkElementAutomationPeer.CreatePeerForElement (b);
			AutomationPeer peer2 = FrameworkElementAutomationPeer.CreatePeerForElement (b);
			Assert.IsNotNull (peer1, "#1");
			Assert.AreSame (peer1, peer2, "#2");
		}

		[TestMethod]
		public void CreatePeer2 ()
		{
			FrameworkElement b = CreateConcreteFrameworkElement();
			FrameworkElementAutomationPeer peer1 = new FrameworkElementAutomationPeer (b);
			AutomationPeer peer2 = FrameworkElementAutomationPeer.CreatePeerForElement (b);
			Assert.AreNotSame (peer1, peer2, "#2");
		}
		
		[TestMethod]
		public void CreatePeer3()
		{
			FrameworkElement element = CreateConcreteFrameworkElement();
			FrameworkElementAutomationPeer peer = CreateConcreteFrameworkElementAutomationPeer (element) as FrameworkElementAutomationPeer;
			AutomationPeer realPeer = FrameworkElementAutomationPeer.CreatePeerForElement (element);
			Assert.AreNotSame (peer, realPeer, "#0");

			AutomationPeer anotherRealPeer = FrameworkElementAutomationPeer.CreatePeerForElement(element);
			Assert.AreSame (anotherRealPeer, realPeer, "#1");
		}

		[TestMethod]
		public void FindPeer ()
		{
			Button b = new Button ();
			AutomationPeer peer1 = FrameworkElementAutomationPeer.FromElement (b);
			Assert.IsNull (peer1, "#1");

			peer1 = FrameworkElementAutomationPeer.CreatePeerForElement (b);
			Assert.IsNotNull (peer1, "#2");

			AutomationPeer peer2 = FrameworkElementAutomationPeer.FromElement (b);
			Assert.AreSame (peer1, peer2, "#3");

			peer2 = FrameworkElementAutomationPeer.FromElement (b);
			Assert.AreSame (peer1, peer2, "#4");
		}

		[TestMethod]
		public void Owner ()
		{
			FrameworkElement element = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeer realPeer 
				= FrameworkElementAutomationPeer.CreatePeerForElement (element) as FrameworkElementAutomationPeer;
			Assert.AreSame (element, realPeer.Owner, "#0");
		}

		[TestMethod]
		public virtual void ContentTest ()
		{
			Assert.IsFalse (IsContentPropertyElement (), 
				"FrameworkElementAutomationPeer is not ContentElement. Override this method");
		}

		[TestMethod]
		[Asynchronous]
		public virtual void GetParentTest ()
		{
			bool layoutUpdated = false;
			bool loaded = false;

			Button button = new Button ();
			button.Width = 30;
			button.Height = 30;
			button.LayoutUpdated += (o, e) => layoutUpdated = true;
			button.Loaded += (o, e) => loaded = true;
			FrameworkElement concrete = CreateConcreteFrameworkElement ();
			concrete.Width = 20;
			concrete.Height = 10;
			button.Content = concrete;

			TestPanel.Children.Add (button);

			AutomationPeer buttonPeer = null;
			AutomationPeer concretePeer = null;
			StackPanel firstPanel = null;
			StackPanel secondPanel = null;

			EnqueueConditional (() => loaded, "Loaded #0");
			Enqueue (() => {
				buttonPeer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
				concretePeer = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);

				Assert.IsNotNull (buttonPeer, "CreatePeerForElement #0");
				Assert.IsNotNull (concretePeer, "CreatePeerForElement #1");

				Assert.IsNotNull (buttonPeer.GetChildren (), "GetChildren #0");

				Assert.IsNotNull (concretePeer.GetParent (), "GetParent #1");
				Assert.AreEqual (buttonPeer, concretePeer.GetParent (), "GetParent #2");

				Assert.AreEqual (button, concrete.Parent, "Parent #0");

				layoutUpdated = false;
				button.Content = null;
			});
			EnqueueConditional (() => layoutUpdated, "LayoutUpdated #0");
			Enqueue (() => {
				Assert.IsNull (buttonPeer.GetChildren (), "GetChildren #1");
				Assert.IsNull (concretePeer.GetParent (), "GetParent #3");
				Assert.IsNull (concrete.Parent, "Parent #1");

				// We add a new stack panel, the visual hierarchy will be:
				// ScrollViewer
				// - StackPanel
				firstPanel = new StackPanel ();

				layoutUpdated = false;
				button.Content = firstPanel;
				Assert.AreEqual (button, firstPanel.Parent, "Parent #2");
			});
			EnqueueConditional (() => layoutUpdated, "LayoutUpdated #1");
			Enqueue (() => {
				Assert.IsNull (buttonPeer.GetChildren (), "GetChildren #2");
				Assert.IsNull (concretePeer.GetParent (), "GetParent #4");
				Assert.IsNull (concrete.Parent, "Parent #3");

				// We add the concrete into the new stack panel, the visual hierarchy will be:
				// ScrollViewer
				// - StackPanel
				// -- Concrete
				layoutUpdated = false;
				firstPanel.Children.Add (concrete);
			});
			EnqueueConditional (() => layoutUpdated, "LayoutUpdated #2");
			Enqueue (() => {
				Assert.IsNotNull (buttonPeer.GetChildren (), "GetChildren #3");
				Assert.IsNotNull (concretePeer.GetParent (), "GetParent #5");
				Assert.AreEqual (buttonPeer, concretePeer.GetParent (), "GetParent #6");
				Assert.AreEqual (firstPanel, concrete.Parent, "Parent #4");

				// We remove the concrete and because we are going to add a stackpanel into 
				// the stackpanel this shouldn't change the peers, the visual hierarchy will be:
				// ScrollViewer
				// - StackPanel
				// -- StackPanel
				layoutUpdated = false;
				firstPanel.Children.Remove (concrete);
			});
			EnqueueConditional (() => layoutUpdated, "LayoutUpdated #3");
			Enqueue (() => {
				Assert.IsNull (buttonPeer.GetChildren (), "GetChildren #4");
				Assert.IsNull (concretePeer.GetParent (), "GetParent #7");
				Assert.IsNull (concrete.Parent, "Parent #5");

				secondPanel = new StackPanel ();
				layoutUpdated = false;
				firstPanel.Children.Add (secondPanel);
			});
			EnqueueConditional (() => layoutUpdated, "LayoutUpdated #4");
			Enqueue (() => {
				Assert.IsNull (buttonPeer.GetChildren (), "GetChildren #4");
				Assert.IsNull (concretePeer.GetParent (), "GetParent #7");

				layoutUpdated = false;
				secondPanel.Children.Add (concrete);
			});
			EnqueueConditional (() => layoutUpdated, "LayoutUpdated #5");
			Enqueue (() => {
				Assert.IsNotNull (buttonPeer.GetChildren (), "GetChildren #5");
				Assert.IsNotNull (concretePeer.GetParent (), "GetParent #8");
				Assert.AreEqual (buttonPeer, concretePeer.GetParent (), "GetParent #9");
				Assert.AreEqual (firstPanel, secondPanel.Parent, "Parent #6");
				Assert.AreEqual (secondPanel, concrete.Parent, "Parent #7");
			});
			EnqueueTestComplete ();
		}

		// All "visible" controls must override GetBoundingRectangle and call this method
		protected void TestLocationAndSize ()
		{
			bool concreteLoaded = false;
			bool concreteLayoutUpdated = false;
			FrameworkElement concrete = CreateConcreteFrameworkElement ();
			concrete.Loaded += (o, e) => concreteLoaded = true;
			concrete.LayoutUpdated += (o, e) => concreteLayoutUpdated = true;
			AutomationPeer bap = FrameworkElementAutomationPeer.CreatePeerForElement (concrete);

			// I'm going to add a canvas to explicitly indicate Width/Height
			bool canvasLoaded = false;
			bool canvasLayoutUpdated = false;
			Canvas canvas = new Canvas ();
			canvas.Children.Add (concrete);
			canvas.Loaded += (o, e) => canvasLoaded = true;
			canvas.LayoutUpdated += (o, e) => canvasLayoutUpdated = true;

			concrete.SetValue (Canvas.TopProperty, (double) 10);
			concrete.SetValue (Canvas.LeftProperty, (double) 30);
			concrete.SetValue (Canvas.WidthProperty, (double) 152);
			concrete.SetValue (Canvas.HeightProperty, (double) 234);
			concreteLayoutUpdated = false;

			TestPanel.Children.Add (canvas);

			EnqueueConditional (() => concreteLoaded && canvasLoaded, "ConcreteLoaded #0");
			// Testing Widht & Height
			Enqueue (() => {
				Rect boundingRectangle = bap.GetBoundingRectangle ();
				Assert.AreEqual (30, boundingRectangle.X, "GetBoundingRectangle X #0");
				Assert.AreEqual (10, boundingRectangle.Y, "GetBoundingRectangle Y #0");
				Assert.AreEqual (152, boundingRectangle.Width, "GetBoundingRectangle Width #0");
				Assert.AreEqual (234, boundingRectangle.Height, "GetBoundingRectangle Height #0");
				concreteLayoutUpdated = false;

				concrete.SetValue (Canvas.TopProperty, (double) 100);
				concrete.SetValue (Canvas.LeftProperty, (double) 300);
				// Now using the properties, shouldn't affect
				concrete.Width = 200;
				concrete.Height = 350;
			});
			EnqueueConditional (() => concreteLayoutUpdated || canvasLayoutUpdated, "ConcreteLayoutUpdated #0");
			Enqueue (() => {
				Rect boundingRectangle = bap.GetBoundingRectangle ();

				Assert.AreEqual (300, boundingRectangle.X, "GetBoundingRectangle X #1");
				Assert.AreEqual (100, boundingRectangle.Y, "GetBoundingRectangle Y #1");
				Assert.AreEqual (200, boundingRectangle.Width, "GetBoundingRectangle Width #1");
				Assert.AreEqual (350, boundingRectangle.Height, "GetBoundingRectangle Height #1");
				concreteLayoutUpdated = false;
			});
			Enqueue (() => {
				// A more complex layout, a canvas containing a canvas in a canvas with a button
				canvas.Children.Remove (concrete);
				canvasLayoutUpdated = false;

				Canvas first = new Canvas ();
				canvas.Children.Add (first);
				first.SetValue (Canvas.LeftProperty, (double) 10);
				first.SetValue (Canvas.TopProperty, (double) 20);
				first.SetValue (Canvas.WidthProperty, (double) 300);
				first.SetValue (Canvas.HeightProperty, (double) 300);

				Canvas second = new Canvas ();
				first.Children.Add (second);
				second.SetValue (Canvas.LeftProperty, (double) 10);
				second.SetValue (Canvas.TopProperty, (double) 20);
				second.SetValue (Canvas.WidthProperty, (double) 200);
				second.SetValue (Canvas.HeightProperty, (double) 200);

				second.Children.Add (concrete);
				concrete.SetValue (Canvas.LeftProperty, (double) 5);
				concrete.SetValue (Canvas.TopProperty, (double) 5);
				concrete.SetValue (Canvas.WidthProperty, (double) 100);
				concrete.SetValue (Canvas.HeightProperty, (double) 100);
			});
			EnqueueConditional (() => concreteLayoutUpdated && canvasLayoutUpdated, "ConcreteLayoutUpdated #1");
			Enqueue (() => {
				Rect boundingRectangle = bap.GetBoundingRectangle ();

				Assert.AreEqual (25, boundingRectangle.X, "GetBoundingRectangle X #2");
				Assert.AreEqual (45, boundingRectangle.Y, "GetBoundingRectangle Y #2");
				Assert.AreEqual (100, boundingRectangle.Width, "GetBoundingRectangle Width #2");
				Assert.AreEqual (100, boundingRectangle.Height, "GetBoundingRectangle Height #2");

				canvasLayoutUpdated = false;
				concrete.Visibility = Visibility.Collapsed;
			});
			// We are going to test Height and Width when Visibility is collapsed
			EnqueueConditional (() => concreteLayoutUpdated && canvasLayoutUpdated, "ConcreteLayoutUpdated #2");
			Enqueue (() => {
				Rect boundingRectangle = bap.GetBoundingRectangle ();

				Assert.AreEqual (0, boundingRectangle.X, "GetBoundingRectangle X #3");
				Assert.AreEqual (0, boundingRectangle.Y, "GetBoundingRectangle Y #3");
				Assert.AreEqual (0, boundingRectangle.Width, "GetBoundingRectangle Width #3");
				Assert.AreEqual (0, boundingRectangle.Height, "GetBoundingRectangle Height #3");
			});
			EnqueueTestComplete ();
		}

		protected bool IsContentPropertyElement ()
		{
			UIElement uielement = CreateConcreteFrameworkElement ();

			object[] attributes
				= uielement.GetType ().GetCustomAttributes (typeof (ContentPropertyAttribute), true);
			if (attributes.Length == 0)
				return false;

			ContentPropertyAttribute attribute = (ContentPropertyAttribute) attributes[0];
			PropertyInfo propertyInfo = uielement.GetType ().GetProperty (attribute.Name,
				BindingFlags.Public | BindingFlags.Instance | BindingFlags.GetProperty);
			if (propertyInfo == null)
				return false;

			MethodInfo methodInfo = propertyInfo.GetGetMethod ();
			if (methodInfo == null)
				return false;

			return true;
		}

		protected virtual FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new ConcreteFrameworkElement ();
		}

		protected virtual FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new FrameworkElementAutomationPeerPoker (element);
		}

		protected void TestIsKeyboardFocusable ()
		{
			FrameworkElement element = CreateConcreteFrameworkElement ();
			Control control = element as Control;
			AutomationPeer peer = null;
			FrameworkElementAutomationPeerContract feap = null;
			ScrollViewer viewer = new ScrollViewer ();

			if (control == null) {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);
				feap = peer as FrameworkElementAutomationPeerContract;
				CreateAsyncTest (element,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #0");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #0");
				});
			} else {
				viewer.Content = control;
				CreateAsyncTest (viewer,
				() => {
					peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);
					feap = peer as FrameworkElementAutomationPeerContract;
					Assert.IsNotNull (peer, "Create #0");
					Assert.IsTrue (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #1");
					if (feap != null)
						Assert.IsTrue (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #1");
				},
				() => control.IsEnabled = false,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #2");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #2");
				},
				() => control.IsEnabled = true,
				() => {
					Assert.IsTrue (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #3");
					if (feap != null)
						Assert.IsTrue (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #3");
				},
				() => control.Visibility = Visibility.Collapsed,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #4");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #4");
				},
				() => control.Visibility = Visibility.Visible,
				() => {
					Assert.IsTrue (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #5");
					if (feap != null)
						Assert.IsTrue (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #5");
				},
				// Now we change parent's visibility, doesn't affect us at all
				() => viewer.Visibility = Visibility.Collapsed,
				() => {
					Assert.IsTrue (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #6");
					if (feap != null)
						Assert.IsTrue (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #6");
				},
				// We return everything to normal, to keep sanity
				() => viewer.Visibility = Visibility.Visible,
				() => control.IsTabStop = false,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #7");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #7");
				},
				() => control.IsTabStop = true,
				() => {
					Assert.IsTrue (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #8");
					if (feap != null)
						Assert.IsTrue (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #8");
				});
			}
		}

		protected void TestIsNotKeyboardFocusable ()
		{
			FrameworkElement element = CreateConcreteFrameworkElement ();
			Control control = element as Control;
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (element);
			FrameworkElementAutomationPeerContract feap = peer as FrameworkElementAutomationPeerContract;

			if (control == null)
				EnqueueTestComplete ();
			else {
				CreateAsyncTest (control,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #1");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #1");
				},
				() => control.IsEnabled = false,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #2");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #2");
				},
				() => control.IsEnabled = true,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #3");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #3");
				},
				() => control.Visibility = Visibility.Collapsed,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #4");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #4");
				},
				() => control.Visibility = Visibility.Visible,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #5");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #5");
				},
				() => control.IsTabStop = false,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #6");
					if (feap != null)
						Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #6");
				},
				() => control.IsTabStop = true,
				() => {
					Assert.IsTrue (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #7");
					if (feap != null)
						Assert.IsTrue (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore #7");
				});
			}
		}

		protected void TestIsKeyboardFocusableEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			FrameworkElement element = CreateConcreteFrameworkElement ();
			Control control = element as Control;
			AutomationPeer peer = null;
			AutomationPropertyEventTuple tuple = null;

			ScrollViewer scrollViewer = new ScrollViewer ();

			if (control == null)
				EnqueueTestComplete ();
			else {
				scrollViewer.Content = control;

				CreateAsyncTest (scrollViewer,
				() => {
					peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
					Assert.IsTrue (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #1");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsEnabled = false;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNotNull (tuple, "#0");
					Assert.IsTrue ((bool) tuple.OldValue, "OldValue #0");
					Assert.IsFalse ((bool) tuple.NewValue, "NewValue #0");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsEnabled = true;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNotNull (tuple, "#1");
					Assert.IsFalse ((bool) tuple.OldValue, "OldValue #1");
					Assert.IsTrue ((bool) tuple.NewValue, "NewValue #1");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.Visibility = Visibility.Collapsed;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNotNull (tuple, "#2");
					Assert.IsTrue ((bool) tuple.OldValue, "OldValue #2");
					Assert.IsFalse ((bool) tuple.NewValue, "NewValue #2");
				},
				() => { 
					EventsManager.Instance.Reset ();
					control.Visibility = Visibility.Visible;
				},
				// Now we change parent's visibility, doesn't affect us at all
				() => {
					EventsManager.Instance.Reset ();
					scrollViewer.Visibility = Visibility.Collapsed;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
                                                                                               AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNull (tuple, "#4");
				},
				// We return everything to normal, to keep sanity
				() => {
					EventsManager.Instance.Reset ();
					scrollViewer.Visibility = Visibility.Visible;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNull (tuple, "#4");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsTabStop = false;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNotNull (tuple, "#5");
					Assert.IsTrue ((bool) tuple.OldValue, "OldValue #5");
					Assert.IsFalse ((bool) tuple.NewValue, "NewValue #5");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsTabStop = true;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNotNull (tuple, "#6");
					Assert.IsFalse ((bool) tuple.OldValue, "OldValue #6");
					Assert.IsTrue ((bool) tuple.NewValue, "NewValue #6");
				});
			}
		}

		protected void TestIsNotKeyboardFocusableEvent ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			FrameworkElement element = CreateConcreteFrameworkElement ();
			Control control = element as Control;
			TestIsNotKeyboardFocusableEvent (control);
		}

		protected void TestIsNotKeyboardFocusableEvent (Control control)
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			AutomationPeer peer = null;
			AutomationPropertyEventTuple tuple = null;

			if (control == null)
				EnqueueTestComplete ();
			else {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
				CreateAsyncTest (control,
				() => {
					Assert.IsFalse (peer.IsKeyboardFocusable (), "IsKeyboardFocusable #1");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsEnabled = false;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNull (tuple, "#0");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsEnabled = true;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNull (tuple, "#1");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.Visibility = Visibility.Collapsed;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNull (tuple, "#2");
				},
				() => { 
					EventsManager.Instance.Reset ();
					control.Visibility = Visibility.Visible;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNull (tuple, "#3");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsTabStop = false;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNull (tuple, "#4");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsTabStop = true;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNotNull (tuple, "#5");
					Assert.IsFalse ((bool) tuple.OldValue, "OldValue #0");
					Assert.IsTrue ((bool) tuple.NewValue, "NewValue #0");
				},
				() => {
					EventsManager.Instance.Reset ();
					control.IsTabStop = false;
				},
				() => {
					tuple = EventsManager.Instance.GetAutomationEventFrom (peer,
					                                                       AutomationElementIdentifiers.IsKeyboardFocusableProperty);
					Assert.IsNotNull (tuple, "#6");
					Assert.IsTrue ((bool) tuple.OldValue, "OldValue #1");
					Assert.IsFalse ((bool) tuple.NewValue, "NewValue #1");
				});
			}
		}


		internal class AutomationEventTuple {
			public AutomationPeer Peer { get; set; } 
			public AutomationEvents Event { get; set; }
		}

		internal class AutomationPropertyEventTuple {
			public AutomationPeer Peer { get; set; }
			public AutomationProperty Property { get; set; }
			public object OldValue { get; set; }
			public object NewValue { get; set; }
		}

		internal class EventsManager {
			public static readonly EventsManager Instance = new EventsManager ();

			private EventsManager ()
			{
				properties = new List<AutomationPropertyEventTuple> ();
				events = new List<AutomationEventTuple> ();

				// We try to load System.Windows to also listen our internal events.
				// "System.Windows, Version=3.0.0.0, Culture=neutral, PublicKeyToken=0738eb9f132ed756"
				AssemblyName name = new AssemblyName ();
				name.Name = "System.Windows";
				name.Version = new Version (3, 0, 0, 0);
				name.CultureInfo = new global::System.Globalization.CultureInfo (string.Empty);
				name.SetPublicKeyToken (new byte[] {0x07, 0x38, 0xeb, 0x9f, 0x13, 0x2e, 0xd7, 0x56});
			
				Assembly assembly;
				try {
					assembly = Assembly.Load (name);
				} catch (Exception) {
					return;
				}
				singletonType = assembly.GetType ("System.Windows.Automation.Peers.AutomationSingleton");
				if (singletonType == null)
					return;

				FieldInfo info = singletonType.GetField ("Instance", BindingFlags.Public | BindingFlags.Static);
				automationSingleton = info.GetValue (null);

				singletonType.GetMethod ("ForceAccessibilityEnabled",
				                         BindingFlags.NonPublic | BindingFlags.Instance)
				             .Invoke (automationSingleton, null);

				AddEventHandler ("AutomationPropertyChanged", "AddPropertyEvent");
				AddEventHandler ("AutomationEventRaised", "AddEvent");
			}

			public void Reset ()
			{
				events.Clear ();
				properties.Clear ();
			}

			public AutomationEventTuple GetAutomationEventFrom (AutomationPeer peer, AutomationEvents evnt)
			{
				return (from e in events where e.Event == evnt && peer == e.Peer select e).FirstOrDefault();
			}

			public AutomationPropertyEventTuple GetAutomationEventFrom (AutomationPeer peer, AutomationProperty property)
			{
				return (from e in properties where e.Property == property && peer == e.Peer select e).FirstOrDefault();
			}

			public bool AutomationSingletonExists {
				get { return automationSingleton != null; }
			}

			public ReadOnlyCollection<AutomationEventTuple> Events {
				get { return new ReadOnlyCollection<AutomationEventTuple> (events); }
			}

			public ReadOnlyCollection<AutomationPropertyEventTuple> PropertyEvents {
				get { return new ReadOnlyCollection<AutomationPropertyEventTuple> (properties); }
			}

			public void AddEvent (object obj)
			{
 				events.Add (new AutomationEventTuple () {
						Peer  = GetProperty<AutomationPeer> (obj, "Peer"),
						Event = GetProperty<AutomationEvents> (obj, "Event")
					}
				);
			}

			public void AddPropertyEvent (object obj)
			{
 				properties.Add (new AutomationPropertyEventTuple () {
						Peer     = GetProperty<AutomationPeer> (obj, "Peer"),
						Property = GetProperty<AutomationProperty> (obj, "Property"),
						OldValue = GetProperty<object> (obj, "OldValue"),
						NewValue = GetProperty<object> (obj, "NewValue"),
					}
				);
			}

			private void AddEventHandler (string eventName, string handlerName)
			{
				EventInfo propertyChangedEvent = singletonType.GetEvent (eventName);
				Type delegateType = propertyChangedEvent.EventHandlerType;

				MethodInfo addEventMethod = 
					typeof (EventsManager).GetMethod (handlerName,
					                                  new Type[] { typeof (object) });
				DynamicMethod dynamicMethod 
					= new DynamicMethod (string.Empty, 
					                     null,
							     GetDelegateParameterTypes (delegateType),
							     typeof (EventsManager));
				ILGenerator ilgen = dynamicMethod.GetILGenerator();
				ilgen.Emit (OpCodes.Ldarg_0);
				ilgen.Emit (OpCodes.Ldarg_2);
        			ilgen.Emit (OpCodes.Callvirt, addEventMethod);
				ilgen.Emit (OpCodes.Ret);

				Delegate delegateEmitted = dynamicMethod.CreateDelegate (delegateType, this);

				MethodInfo addHandler = propertyChangedEvent.GetAddMethod ();
				addHandler.Invoke (automationSingleton, new object[] { delegateEmitted });
			}

			private Type[] GetDelegateParameterTypes (Type d)
			{
				MethodInfo invoke = d.GetMethod ("Invoke");
				ParameterInfo[] parameters = invoke.GetParameters ();

				Type[] typeParameters = new Type [parameters.Length + 1];
				typeParameters [0] = typeof (EventsManager); 
				for (int i = 0; i < parameters.Length; i++)
					typeParameters [i + 1] = parameters [i].ParameterType;
				return typeParameters;
			}

			private TResult GetProperty<TResult> (object reference,
			                                      string propertyName)
			{
				PropertyInfo propertyInfo
					= reference.GetType ().GetProperty (propertyName,
			        	                                    BindingFlags.Public 
									     | BindingFlags.Instance
									     | BindingFlags.GetProperty);
				if (propertyInfo == null)
					throw new NotSupportedException (string.Format ("Property not found: {0} ", propertyName));

				return (TResult) propertyInfo.GetValue (reference, null);
			}

			private List<AutomationEventTuple> events;
			private List<AutomationPropertyEventTuple> properties;
			private object automationSingleton;
			private Type singletonType;
		}

		internal class PeerFromProvider : FrameworkElementAutomationPeer {

			public PeerFromProvider () : base (new Button ()) 
			{
			}

			public AutomationPeer GetPeerFromProvider (IRawElementProviderSimple provider)
			{
				return PeerFromProvider (provider);
			}

		}
	}
}
