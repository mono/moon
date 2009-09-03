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
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, feap.GetAcceleratorKey (), "GetAcceleratorKey");
			Assert.AreEqual (string.Empty, feap.GetAcceleratorKeyCore_ (), "GetAcceleratorKeyCore");
		}

		[TestMethod]
		public virtual void GetAcceleratorKey_AttachedProperty()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (string.Empty, feap.GetAcceleratorKey (), "GetAcceleratorKey #0");
			Assert.AreEqual (string.Empty, feap.GetAcceleratorKeyCore_ (), "GetAcceleratorKeyCore #0");

			string acceleratorKey = "CTRL+C";

			fe.SetValue (AutomationProperties.AcceleratorKeyProperty, acceleratorKey);
			Assert.AreEqual (acceleratorKey, feap.GetAcceleratorKey (), "GetAcceleratorKey #1");
			Assert.AreEqual (acceleratorKey, feap.GetAcceleratorKeyCore_ (), "GetAcceleratorKeyCore #1");

			fe.SetValue (AutomationProperties.AcceleratorKeyProperty, null);
			Assert.AreEqual (string.Empty, feap.GetAcceleratorKey (), "GetAcceleratorKey #2 YY");
			Assert.AreEqual (string.Empty, feap.GetAcceleratorKeyCore_ (), "GetAcceleratorKeyCore #2 YY");
		}

		[TestMethod]
		public virtual void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap 
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());

			Assert.AreEqual (string.Empty, feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual (string.Empty, feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public virtual void GetAccessKey ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, feap.GetAccessKey (), "GetAccessKey");
			Assert.AreEqual (string.Empty, feap.GetAccessKeyCore_ (), "GetAccessKeyCore");
		}

		[TestMethod]
		public virtual void GetAccessKey_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (string.Empty, feap.GetAccessKey (), "GetAccessKey");
			Assert.AreEqual (string.Empty, feap.GetAccessKeyCore_ (), "GetAccessKeyCore");

			string accessKey = "ALT+C";

			fe.SetValue (AutomationProperties.AccessKeyProperty, accessKey);
			Assert.AreEqual (accessKey, feap.GetAccessKey (), "GetAccessKey #1");
			Assert.AreEqual (accessKey, feap.GetAccessKeyCore_ (), "GetAccessKeyCore #1");

			fe.SetValue (AutomationProperties.AccessKeyProperty, null);
			Assert.AreEqual (string.Empty, feap.GetAccessKey (), "GetAccessKey #2");
			Assert.AreEqual (string.Empty, feap.GetAccessKeyCore_ (), "GetAccessKeyCore #2");
		}

		[TestMethod]
		public virtual void GetAutomationControlType ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationControlType.Custom, feap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Custom, feap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public virtual void GetAutomationId ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, feap.GetAutomationId (), "GetAutomationId");
			Assert.AreEqual (string.Empty, feap.GetAutomationIdCore_ (), "GetAutomationIdCore");
		}

		[TestMethod]
		public virtual void GetAutomationId_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);
			Assert.AreEqual (string.Empty, feap.GetAutomationId (), "GetAutomationId");
			Assert.AreEqual (string.Empty, feap.GetAutomationIdCore_(), "GetAutomationIdCore");

			string automationId = "MyAttachedAutomationId";

			fe.SetValue(AutomationProperties.AutomationIdProperty, automationId);
			Assert.AreEqual (automationId, feap.GetAutomationId (), "GetAutomationId #1");
			Assert.AreEqual (automationId, feap.GetAutomationIdCore_(), "GetAutomationIdCore #1");

			fe.SetValue (AutomationProperties.AutomationIdProperty, null);
			Assert.AreEqual (string.Empty, feap.GetAutomationId (), "GetAutomationId #2");
			Assert.AreEqual (string.Empty, feap.GetAutomationIdCore_ (), "GetAutomationIdCore #2");
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
		public virtual void GetClickablePoint ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (0, feap.GetClickablePoint ().X, "GetClickablePoint X");
			Assert.AreEqual (0, feap.GetClickablePoint ().Y, "GetClickablePoint Y");
			Assert.AreEqual (0, feap.GetClickablePointCore_ ().X, "GetClickablePointCore X");
			Assert.AreEqual (0, feap.GetClickablePointCore_ ().Y, "GetClickablePointCore Y");
		}

		[TestMethod]
		public virtual void GetHelpText ()
		{
			FrameworkElementAutomationPeerContract feap 
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, feap.GetHelpText (), "GetHelpText");
			Assert.AreEqual (string.Empty, feap.GetHelpTextCore_ (), "GetHelpTextCore");
		}

		[TestMethod]
		public virtual void GetHelpText_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);
			Assert.AreEqual (string.Empty, feap.GetHelpText (), "GetHelpText");
			Assert.AreEqual (string.Empty, feap.GetHelpTextCore_ (), "GetHelpTextCore");

			string helpText = "My Help Text property";

			fe.SetValue(AutomationProperties.HelpTextProperty, helpText);
			Assert.AreEqual (helpText, feap.GetHelpText (), "GetHelpText #1");
			Assert.AreEqual (helpText, feap.GetHelpTextCore_ (), "GetHelpTextCore #1");

			fe.SetValue (AutomationProperties.HelpTextProperty, null);
			Assert.AreEqual (string.Empty, feap.GetHelpText (), "GetHelpText #2");
			Assert.AreEqual (string.Empty, feap.GetHelpTextCore_ (), "GetHelpTextCore #2");
		}
		
		[TestMethod]
		public virtual void GetItemStatus ()
		{
			FrameworkElementAutomationPeerContract feap 
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, feap.GetItemStatus (), "GetItemStatus");
			Assert.AreEqual (string.Empty, feap.GetItemStatusCore_ (), "GetItemStatusCore");
		}

		[TestMethod]
		public virtual void GetItemStatus_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);
			Assert.AreEqual (string.Empty, feap.GetItemStatus (), "GetItemStatus");
			Assert.AreEqual (string.Empty, feap.GetItemStatusCore_ (), "GetItemStatusCore");

			string itemStatus = "My Item Status";

			fe.SetValue (AutomationProperties.ItemStatusProperty, itemStatus);
			Assert.AreEqual (itemStatus, feap.GetItemStatus (), "GetItemStatus #1");
			Assert.AreEqual (itemStatus, feap.GetItemStatusCore_ (), "GetItemStatusCore #1");

			fe.SetValue (AutomationProperties.ItemStatusProperty, null);
			Assert.AreEqual (string.Empty, feap.GetItemStatus (), "GetItemStatus #2");
			Assert.AreEqual (string.Empty, feap.GetItemStatusCore_ (), "GetItemStatusCore #2");
		}

		[TestMethod]
		public virtual void GetItemType ()
		{
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (string.Empty, feap.GetItemType (), "GetItemType");
			Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore");
		}

		[TestMethod]
		public virtual void GetItemType_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (string.Empty, feap.GetItemType (), "GetItemType");
			Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore");

			string itemType = "My Item Type";

			fe.SetValue (AutomationProperties.ItemTypeProperty, itemType);
			Assert.AreEqual (itemType, feap.GetItemType (), "GetItemType #1");
			Assert.AreEqual (itemType, feap.GetItemTypeCore_ (), "GetItemTypeCore #1");

			fe.SetValue (AutomationProperties.ItemTypeProperty, null);
			Assert.AreEqual (string.Empty, feap.GetItemType (), "GetItemType #2");
			Assert.AreEqual (string.Empty, feap.GetItemTypeCore_ (), "GetItemTypeCore #2");
		}

		[TestMethod]
		public virtual void GetLocalizedControlType ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			string localizedString = feap.GetAutomationControlType ().ToString ().ToLower();
			Assert.AreEqual (localizedString, feap.GetLocalizedControlType(), 
					string.Format ("GetLocalizedControlType: {0}", localizedString));
			Assert.AreEqual (localizedString, feap.GetLocalizedControlTypeCore_(), 
					string.Format ("GetLocalizedControlTypeCore: {0}", localizedString));
		}

		[TestMethod]
		public virtual void GetOrientation ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual (AutomationOrientation.None, feap.GetOrientation (), "GetOrientation");
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
		public virtual void IsKeyboardFocusable ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.IsFalse (feap.IsKeyboardFocusable (), "IsKeyboardFocusable");
			Assert.IsFalse (feap.IsKeyboardFocusableCore_ (), "IsKeyboardFocusableCore");
		}

		[TestMethod]
		public virtual void IsEnabled ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());

			Assert.IsTrue (feap.IsEnabled (), "IsEnabled");
			Assert.IsTrue (feap.IsEnabledCore_ (), "IsEnabledCore");
		}

		[TestMethod]
		[Asynchronous]
		public virtual void IsOffScreen ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			bool controlLoaded = false;
			fe.Loaded += (o, e) => controlLoaded = true;
			TestPanel.Children.Add (fe);
			EnqueueConditional (() => controlLoaded, "ControlLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsFalse (peer.IsOffscreen (), "IsOffScreen #1");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		public virtual void IsPassword ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.IsFalse (feap.IsPassword (), "IsPassword");
			Assert.IsFalse (feap.IsPasswordCore_ (), "IsPasswordCore");
		}

		[TestMethod]
		public virtual void IsRequiredForForm ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.IsFalse (feap.IsRequiredForForm (), "IsRequiredForForm");
			Assert.IsFalse (feap.IsRequiredForFormCore_ (), "IsRequiredForFormCore");
		}

		[TestMethod]
		public virtual void IsRequiredForForm_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.IsFalse (feap.IsRequiredForForm (), "IsRequiredForForm");
			Assert.IsFalse (feap.IsRequiredForFormCore_ (), "IsRequiredForFormCore");

			fe.SetValue (AutomationProperties.IsRequiredForFormProperty, true);
			Assert.IsTrue (feap.IsRequiredForForm (), "IsRequiredForForm #1");
			Assert.IsTrue (feap.IsRequiredForFormCore_ (), "IsRequiredForFormCore #1");

			fe.SetValue (AutomationProperties.IsRequiredForFormProperty, false);
			Assert.IsFalse (feap.IsRequiredForForm (), "IsRequiredForForm #2");
			Assert.IsFalse (feap.IsRequiredForFormCore_ (), "IsRequiredForFormCore #2");
		}

		[TestMethod]
		public virtual void GetName ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (String.Empty, feap.GetName (), "GetName");
			Assert.AreEqual (String.Empty, feap.GetNameCore_ (), "GetNameCore");
		}

		[TestMethod]
		public virtual void GetName_AttachedProperty0 ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);

			Assert.AreEqual (string.Empty, feap.GetName (), "GetName");
			Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore");

			string name = "Attached Name";

			fe.SetValue (AutomationProperties.NameProperty, name);
			Assert.AreEqual (name, feap.GetName (), "GetName #1");
			Assert.AreEqual (name, feap.GetNameCore_ (), "GetNameCore #1");

			fe.SetValue (AutomationProperties.NameProperty, null);
			Assert.AreEqual (string.Empty, feap.GetName (), "GetName #2");
			Assert.AreEqual (string.Empty, feap.GetNameCore_ (), "GetNameCore #2");
		}

		[TestMethod]
		public virtual void GetName_AttachedProperty1 ()
		{
			FrameworkElement element = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract tbap = CreateConcreteFrameworkElementAutomationPeer (element);

			string textblockName = "Hello world:";
			string nameProperty = "TextBox name";

			TextBlock textblock = new TextBlock ();
			textblock.Text = textblockName;

			element.SetValue (AutomationProperties.NameProperty, nameProperty);
			Assert.AreEqual (nameProperty, tbap.GetName (), "GetName #0");
			Assert.AreEqual (nameProperty, tbap.GetNameCore_ (), "GetNameCore #0");

			element.SetValue (AutomationProperties.LabeledByProperty, textblock);
			Assert.AreEqual (textblockName, tbap.GetName (), "GetName #1");
			Assert.AreEqual (nameProperty, tbap.GetNameCore_ (), "GetNameCore #1");

			textblock.Text = null;
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #2");
			Assert.AreEqual (nameProperty, tbap.GetNameCore_ (), "GetNameCore #2");

			textblock.Text = string.Empty;
			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #3");
			Assert.AreEqual (nameProperty, tbap.GetNameCore_ (), "GetNameCore #3");

			element.SetValue (AutomationProperties.NameProperty, null);

			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #4");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #4");

			element.SetValue (AutomationProperties.LabeledByProperty, null);

			Assert.AreEqual (string.Empty, tbap.GetName (), "GetName #5");
			Assert.AreEqual (string.Empty, tbap.GetNameCore_ (), "GetNameCore #5");
		}

		[TestMethod]
		public virtual void GetLabeledBy ()
		{
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.IsNull (feap.GetLabeledBy (), "GetLabeledBy");
			Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore");
		}

		[TestMethod]
		public virtual void GetLabeledBy_AttachedProperty ()
		{
			FrameworkElement fe = CreateConcreteFrameworkElement ();
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (fe);
			Assert.IsNull (feap.GetLabeledBy (), "GetLabeledBy");
			Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore");

			FrameworkElement labeledBy = new TextBlock ();
			AutomationPeer labeledByPeer = FrameworkElementAutomationPeer.CreatePeerForElement (labeledBy);

			fe.SetValue (AutomationProperties.LabeledByProperty, labeledBy);
			Assert.AreSame (labeledByPeer, feap.GetLabeledBy (), "GetLabeledBy #1");
			Assert.AreSame (labeledByPeer, feap.GetLabeledByCore_(), "GetLabeledByCore #1");

			fe.SetValue (AutomationProperties.LabeledByProperty, null);
			Assert.IsNull (feap.GetLabeledBy (), "GetLabeledBy #2");
			Assert.IsNull (feap.GetLabeledByCore_ (), "GetLabeledByCore #2");
		}

		[TestMethod]
		public virtual void IsContentElement ()
		{
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.IsTrue (feap.IsContentElement (), "IsContentElement");
			Assert.IsTrue (feap.IsContentElementCore_ (), "IsContentElementCore");
		}

		[TestMethod]
		public virtual void IsControlElement ()
		{
			FrameworkElementAutomationPeerContract feap = CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.IsTrue (feap.IsControlElement (), "IsControlElement");
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

				FieldInfo bridgeField =
					singletonType.GetField ("accessibilityEnabled", BindingFlags.NonPublic | BindingFlags.Instance);
				bridgeField.SetValue (automationSingleton, true);

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
