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
using System.Windows.Automation.Provider;
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

		[TestMethod]
		public void IInvokeProvider_Invoke ()
		{
			Test_InvokeProvider_Invoke (CreateConcreteFrameworkElement () as ButtonBase);
		}

		[TestMethod]
		[Asynchronous]
		public override void GetChildrenChanged ()
		{
			if (!IsContentPropertyElement ()
			    || !EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			ContentControl control = CreateConcreteFrameworkElement () as ContentControl;
			AutomationPeer peer = null;
			List<AutomationPeer> children = null;
			Assert.IsNotNull (control, "ContentControl");
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
			AutomationPeer tb0 = null;
			AutomationPeer tb1 = null;

			CreateAsyncTest (control,
			() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (control);
				Assert.IsNotNull (peer, "Peer");
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #0");
				Assert.AreEqual (2, children.Count, "Children.Count #0");
				// Control hierarchy: control { textblock, textblock }
				// UIA hierarchy: control { textblock, textblock }
			},
			() => {
				button = new Button () { Content = "Button0" };
				control.Content = button;
				// Control hierarchy: control { textblock, textblock, button }
				// UIA hierarchy: control { textblock, textblock, button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #1");
				Assert.AreEqual (3, children.Count, "Children.Count #1");
				Assert.AreEqual (FrameworkElementAutomationPeer.CreatePeerForElement (button),
					children [2], "GetChildren #2");

				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #0");
				EventsManager.Instance.Reset ();
			},
			() => control.Content = null,
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #3");
				Assert.AreEqual (2, children.Count, "Children.Count #2");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #1");
				EventsManager.Instance.Reset ();

				// Panel subclasses don't expose any AutomationPeer but chidren's
				control.Content = canvas;
				// Control hierarchy: control { textblock, tetxblock, canvas { } }
				// UIA hierarchy: control { textblock, textblock }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #4");
				Assert.AreEqual (2, children.Count, "Children.Count #2");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #2");
				EventsManager.Instance.Reset ();
				canvas.Children.Add (stackPanel);
				// Control hierarchy: control { textblock, textblock, canvas { stackpanel { } } }
				// UIA hierarchy: control { textblock, textblock }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #5");
				Assert.AreEqual (2, children.Count, "Children.Count #3");
				canvas.Children.Add (button);
				// Control hierarchy: control { textblock, textblock, canvas { stackapanel, button } }
				// UIA hierarchy: control { textblock, textblock, button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #6");
				Assert.AreEqual (3, children.Count, "Children.Count #4");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (button)),
				               "GetChildren #7");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #3");
				EventsManager.Instance.Reset ();

				stackPanel.Children.Add (checkbox);
				// Control hierarchy: control { textblock, textblock, canvas { stackPanel { checkbox }, button } }
				// UIA hierarchy: control { textblock, textblock, checkbox, button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #8");
				Assert.AreEqual (4, children.Count, "Children.Count #5");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox)),
				               "GetChildren #9");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (button)),
				               "GetChildren #10");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #4");
				EventsManager.Instance.Reset ();

				stackPanel.Children.Add (grid);
				// Control hierarchy: control { textblock, textblock, canvas { stackPanel { checkbox, grid }, button } }
				// UIA hierarchy: control { textblock, textblock, checkbox, button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #11");
				Assert.AreEqual (4, children.Count, "Children.Count #6");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox)),
				               "GetChildren #12");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (button)),
				               "GetChildren #13");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #5");
				EventsManager.Instance.Reset ();

				grid.Children.Add (textblock);
				// Control hierarchy: control { textblock, textblock, canvas { stackPanel { checkbox, grid { textblock } }, button } }
				// UIA hierarchy: control { textblock, textblock, checkbox, textblock, button }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #14");
				Assert.AreEqual (5, children.Count, "Children.Count #7");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox)),
				               "GetChildren #15");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (button)),
				               "GetChildren #16");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (textblock)),
				               "GetChildren #17");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #6");
				EventsManager.Instance.Reset ();

				canvas.Children.Add (textbox);
				// Control hierarchy: control { textblock, textblock, canvas { stackPanel { checkbox, grid { textblock } }, button, textbox } }
				// UIA hierarchy: control { textblock, textblock, checkbox, textblock, button, textbox }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #18");
				Assert.AreEqual (6, children.Count, "Children.Count #8");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox)),
				               "GetChildren #19");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (button)),
				               "GetChildren #20");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (textblock)),
				               "GetChildren #21");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (textbox)),
				               "GetChildren #22");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #7");
				EventsManager.Instance.Reset ();

				stackPanel.Children.Clear ();
				// Control hierarchy: control { textblock, textblock, canvas { stackPanel { }, button, textbox } }
				// UIA hierarchy: control { textblock, textblock, button, textbox }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #23");
				Assert.AreEqual (4, children.Count, "Children.Count #9");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (button)),
				               "GetChildren #24");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (textbox)),
				               "GetChildren #25");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #8");
				EventsManager.Instance.Reset ();

				// If we modify 'grid' no event should be raised, since is not part of 'canvas' anymore
				grid.Children.Add (gridButton);
			}, // CONTINUE
			() => {
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNull (tuple, "StructureChanged #9");
				EventsManager.Instance.Reset ();
				canvas.Children.Clear ();
				// Control hierarchy: control { textblock, textblock, canvas { } }
				// UIA hierarchy: control { }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #26");
				Assert.AreEqual (2, children.Count, "Children.Count #10");
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
				Assert.IsNotNull (children, "GetChildren #27");
				Assert.AreEqual (5, children.Count, "Children.Count #7");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (stackPanelButton)),
				               "GetChildren #28");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (textblock)),
				               "GetChildren #29");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (gridButton)),
				               "GetChildren #30");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #14");
				EventsManager.Instance.Reset ();

				grid.Children.Add (checkbox);
				// Control hierarchy: canvas { stackpanel { stackPanelButton, grid { textblock, gridButton, checkbox } } }
				// UIA hierarchy: control { stackPanelButton, textblock, gridButton, checkbox }
			},
			() => {
				children = peer.GetChildren ();
				Assert.IsNotNull (children, "GetChildren #31");
				Assert.AreEqual (6, children.Count, "Children.Count #8");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (stackPanelButton)),
				               "GetChildren #32");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (textblock)),
				               "GetChildren #33");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (gridButton)),
				               "GetChildren #34");
				Assert.IsTrue (children.Contains (FrameworkElementAutomationPeer.CreatePeerForElement (checkbox)),
				               "GetChildren #35");
				tuple = EventsManager.Instance.GetAutomationEventFrom (peer, AutomationEvents.StructureChanged);
				Assert.IsNotNull (tuple, "StructureChanged #15");
				EventsManager.Instance.Reset ();
			});
		}

		[TestMethod]
		[Asynchronous]
		public void TestHasKeyboardFocusAfterPattern ()
		{
			HyperlinkButton fe = CreateConcreteFrameworkElement () as HyperlinkButton;

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (fe);
			IInvokeProvider provider = null;

			CreateAsyncTest (fe,
			() => {
				provider = (IInvokeProvider) peer.GetPattern (PatternInterface.Invoke);
				Assert.IsNotNull (provider, "#0");
			}, 
			() => provider.Invoke (),
			() => Assert.IsTrue (peer.HasKeyboardFocus (), "#1"));
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
