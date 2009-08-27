using System;
using System.Collections.Generic;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using Microsoft.Silverlight.Testing;


namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class HyperlinkButtonAutomationPeerTest : ButtonBaseAutomationPeerTest {

		public class HyperlinkButtonAutomationPeerPoker : HyperlinkButtonAutomationPeer, FrameworkElementAutomationPeerContract {

			public HyperlinkButtonAutomationPeerPoker (HyperlinkButton owner) : base (owner)
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
				return base.GetBoundingRectangle ();
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
				return base.GetLocalizedControlType ();
			}

			public AutomationOrientation GetOrientationCore_ ()
			{
				return base.GetOrientationCore ();
			}

			public bool HasKeyboardFocusCore_ ()
			{
				return base.HasKeyboardFocus ();
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
		public override void GetClassName ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual("Hyperlink", feap.GetClassName (), "GetClassNameCore");
			Assert.AreEqual("Hyperlink", feap.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			FrameworkElementAutomationPeerContract feap
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement ());
			Assert.AreEqual(AutomationControlType.Hyperlink, feap.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual(AutomationControlType.Hyperlink, feap.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void GetPattern ()
		{
			FrameworkElementAutomationPeer peer
				= CreateConcreteFrameworkElementAutomationPeer (CreateConcreteFrameworkElement())
					as FrameworkElementAutomationPeer;

			Assert.IsNull (peer.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (peer.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (peer.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (peer.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (peer.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (peer.GetPattern (PatternInterface.RangeValue), "RangeValue");
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

			Assert.IsNotNull (peer.GetPattern (PatternInterface.Invoke), "Invoke");
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest()
		{
			ButtonBase button = (ButtonBase)CreateConcreteFrameworkElement ();
			Assert.IsTrue (IsContentPropertyElement (), "ButtonElement ContentElement.");

			bool buttonLoaded = false;
			button.Loaded += (o, e) => buttonLoaded = true;
			TestPanel.Children.Add (button);

			// StackPanel and two TextBlocks
			bool stackPanelLoaded = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
			stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			int INITIAL_CHILDREN_COUNT = 2;

			EnqueueConditional (() => buttonLoaded, "ButtonLoaded #0");
			Enqueue (() =>
			{
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNotNull (peer.GetChildren (), "GetChildren#0");
				Assert.AreEqual (INITIAL_CHILDREN_COUNT, peer.GetChildren ().Count, "GetChildren #0, count");

				Assert.AreEqual(typeof (TextBlockAutomationPeer), peer.GetChildren () [0].GetType (), "GetChildren #0, type#1");
				Assert.AreEqual(typeof (TextBlockAutomationPeer), peer.GetChildren () [1].GetType (), "GetChildren #0, type#2");
				button.Content = stackPanel;
			});
			EnqueueConditional (() => buttonLoaded && stackPanelLoaded, "ButtonLoaded #1");
			Enqueue (() =>
			{
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (button);
				Assert.IsNotNull (peer.GetChildren(), "GetChildren #1");
				Assert.AreEqual (INITIAL_CHILDREN_COUNT + 2, peer.GetChildren ().Count, "GetChildren.Count #1");
				// We add one TextBlock
				stackPanel.Children.Add (new TextBlock () { Text = "Text2" });
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #2");
				Assert.AreEqual (INITIAL_CHILDREN_COUNT + 3, peer.GetChildren ().Count, "GetChildren.Count #2");
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

		[TestMethod]
		public virtual void InvokeProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists)
				return;

			Test_InvokeProvider_Events ((ButtonBase)CreateConcreteFrameworkElement ());
		}

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			return new HyperlinkButton ();
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new HyperlinkButtonAutomationPeerPoker (element as HyperlinkButton);
		}
	}
}
