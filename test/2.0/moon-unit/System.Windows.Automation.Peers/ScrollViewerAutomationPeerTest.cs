//
// Unit tests for ScrollViewerAutomationPeer
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
using System.Text;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Automation.Provider;
using System.Windows.Automation.Peers;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Automation.Peers {

	[TestClass]
	public class ScrollViewerAutomationPeerTest : FrameworkElementAutomationPeerTest {

		public class ScrollViewerAutomationPeerConcrete : ScrollViewerAutomationPeer, FrameworkElementAutomationPeerContract {

			public ScrollViewerAutomationPeerConcrete (ScrollViewer owner)
				: base (owner)
			{
			}

			#region Overridden methods

			public bool IsControlElementCore_ ()
			{
				return base.IsControlElementCore ();
			}

			public string GetClassNameCore_ ()
			{
				return base.GetClassNameCore ();
			}

			public AutomationControlType GetAutomationControlTypeCore_ ()
			{
				return base.GetAutomationControlTypeCore ();
			}

			#endregion

			#region Wrapper Methods

			public string GetNameCore_ ()
			{
				return base.GetNameCore ();
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

			public global::System.Windows.Rect GetBoundingRectangleCore_ ()
			{
				return base.GetBoundingRectangleCore ();
			}

			public global::System.Collections.Generic.List<AutomationPeer> GetChildrenCore_ ()
			{
				return base.GetChildrenCore ();
			}

			public global::System.Windows.Point GetClickablePointCore_ ()
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

		[TestMethod]
		public override void GetAutomationControlType ()
		{
			ScrollViewerAutomationPeerConcrete svapp = new ScrollViewerAutomationPeerConcrete (new ScrollViewer ());
			Assert.AreEqual (AutomationControlType.Pane, svapp.GetAutomationControlType (), "GetAutomationControlType");
			Assert.AreEqual (AutomationControlType.Pane, svapp.GetAutomationControlTypeCore_ (), "GetAutomationControlTypeCore");
		}

		[TestMethod]
		public override void IsControlElement ()
		{
			ScrollViewerAutomationPeerConcrete svapp = new ScrollViewerAutomationPeerConcrete (new ScrollViewer ());
			Assert.IsTrue (svapp.IsControlElement (), "IsControlElement");
			Assert.IsTrue (svapp.IsControlElementCore_ (), "IsControlElementCore");
		}

		[TestMethod]
		public override void GetClassName ()
		{
			ScrollViewerAutomationPeerConcrete svapp = new ScrollViewerAutomationPeerConcrete (new ScrollViewer ());
			Assert.AreEqual ("ScrollViewer", svapp.GetClassName (), "GetClassNameCore");
			Assert.AreEqual ("ScrollViewer", svapp.GetClassNameCore_ (), "GetClassNameCoreCore");
		}

		[TestMethod]
		public override void GetPattern ()
		{
			ScrollViewerAutomationPeerConcrete svapp = new ScrollViewerAutomationPeerConcrete (new ScrollViewer ());

			Assert.IsNull (svapp.GetPattern (PatternInterface.Dock), "Dock");
			Assert.IsNull (svapp.GetPattern (PatternInterface.ExpandCollapse), "ExpandCollapse");
			Assert.IsNull (svapp.GetPattern (PatternInterface.Grid), "Grid");
			Assert.IsNull (svapp.GetPattern (PatternInterface.GridItem), "GridItem");
			Assert.IsNull (svapp.GetPattern (PatternInterface.Invoke), "Invoke");
			Assert.IsNull (svapp.GetPattern (PatternInterface.MultipleView), "MultipleView");
			Assert.IsNull (svapp.GetPattern (PatternInterface.RangeValue), "RangeValue");
			Assert.IsNull (svapp.GetPattern (PatternInterface.ScrollItem), "ScrollItem");
			Assert.IsNull (svapp.GetPattern (PatternInterface.Selection), "Selection");
			Assert.IsNull (svapp.GetPattern (PatternInterface.SelectionItem), "SelectionItem");
			Assert.IsNull (svapp.GetPattern (PatternInterface.Table), "Table");
			Assert.IsNull (svapp.GetPattern (PatternInterface.TableItem), "TableItem");
			Assert.IsNull (svapp.GetPattern (PatternInterface.Toggle), "Toggle");
			Assert.IsNull (svapp.GetPattern (PatternInterface.Transform), "Transform");
			Assert.IsNull (svapp.GetPattern (PatternInterface.Value), "Value");
			Assert.IsNull (svapp.GetPattern (PatternInterface.Window), "Window");

			Assert.IsNotNull (svapp.GetPattern (PatternInterface.Scroll), "Scroll");
		}

		[TestMethod]
		public override void HasKeyboardFocus ()
		{
			// The control by itself is not focusable, the content is
		}

		[TestMethod]
		[Asynchronous]
		public override void ContentTest ()
		{
			Assert.IsTrue (IsContentPropertyElement (), "ScrollViewer ContentElement.");

			bool scrollViewerLoaded = false;
			bool stackPanelLayoutChanged = false;
			ScrollViewer scrollViewer = CreateConcreteFrameworkElement () as ScrollViewer;
			scrollViewer.Loaded += (o, e) => scrollViewerLoaded = true;
			scrollViewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Auto;
			scrollViewer.VerticalScrollBarVisibility = ScrollBarVisibility.Auto;
			TestPanel.Children.Add (scrollViewer);

			// StackPanel and two TextBlocks
			bool stackPanelLoaded = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.LayoutUpdated += (o, e) => stackPanelLayoutChanged = true;
			stackPanel.Children.Add (new TextBlock () { Text = "Text0" });
			stackPanel.Children.Add (new TextBlock () { Text = "Text1" });
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			EnqueueConditional (() => scrollViewerLoaded, "ScrollViewerLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (scrollViewer);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				scrollViewer.Content = stackPanel;
			});
			EnqueueConditional (() => scrollViewerLoaded && stackPanelLoaded, "ScrollViewerLoaded #1");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (scrollViewer);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #1");
				Assert.AreEqual (2, peer.GetChildren ().Count, "GetChildren.Count #1");
				// We add one TextBlock
				stackPanel.Children.Add (new TextBlock () { Text = "Text2" });
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #2");
				Assert.AreEqual (3, peer.GetChildren ().Count, "GetChildren.Count #2");
				stackPanelLayoutChanged = false;

				// Let's add more items to show ScrollBars
				for (int i = 0; i < 100; i++)
					stackPanel.Children.Add (new TextBlock () { Text = string.Format ("Item {0}", i) });
			});
			EnqueueConditional (() => scrollViewerLoaded && stackPanelLayoutChanged, "ScrollViewerLoaded #2");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (scrollViewer);
				Assert.IsNotNull (peer.GetChildren (), "GetChildren #3");
				// 103 children and 1 scrollbar
				Assert.AreEqual (104, peer.GetChildren ().Count, "GetChildren.Count #3");
				// Let's add a long string in order to show horizontal scrollbar
				string longString = "my long string";
				StringBuilder builder = new StringBuilder ();
				for (int i = 0; i < 50; i++)
					builder.Append (longString);

				stackPanel.Children.Add (new TextBlock () { Text = string.Format ("ItemX {0}", builder.ToString ()) });

				Assert.IsNotNull (peer.GetChildren (), "GetChildren #4");
				// 103 children and 2 scrollbar
				Assert.AreEqual (105, peer.GetChildren ().Count, "GetChildren.Count #4");
			});
			EnqueueTestComplete ();
		}

		#region Pattern Tests

		[TestMethod]
		[Asynchronous]
		public void IScrollProvider_Methods ()
		{
			bool scrollViewerLoaded = false;

			ScrollViewer scrollViewer = CreateConcreteFrameworkElement () as ScrollViewer;
			scrollViewer.Loaded += (o, e) => scrollViewerLoaded = true;
			scrollViewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Auto;
			scrollViewer.VerticalScrollBarVisibility = ScrollBarVisibility.Auto;
			
			// We are going to use this canvas to define an explicit 
			// scrollviewer size and location
			Canvas canvas = new Canvas ();
			TestPanel.Children.Add (canvas);

			scrollViewer.SetValue (Canvas.LeftProperty, 5d);
			scrollViewer.SetValue (Canvas.TopProperty, 5d);
			scrollViewer.Width = 100;
			scrollViewer.Height = 150;
			canvas.Children.Add (scrollViewer);

			bool stackPanelLoaded = false;
			bool stackPanelLayoutChanged = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.LayoutUpdated += (o, e) => stackPanelLayoutChanged = true;
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			EnqueueConditional (() => scrollViewerLoaded, "ScrollViewerLoaded #0");
			Enqueue (() => {
				AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (scrollViewer);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");
				IScrollProvider scrollProvider = (IScrollProvider) peer.GetPattern (PatternInterface.Scroll);

				Assert.IsNull (peer.GetChildren (), "GetChildren #0");
				scrollViewer.Content = stackPanel;

				Assert.IsFalse (scrollProvider.HorizontallyScrollable, "HorizontallyScrollable #0");
				Assert.IsFalse (scrollProvider.VerticallyScrollable, "VerticallyScrollable #0");

				Assert.AreEqual (100, scrollProvider.VerticalViewSize, "VerticalViewSize #0");
				Assert.AreEqual (100, scrollProvider.HorizontalViewSize, "HorizontalViewSize #0");

				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #0");
				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #0");
			});
			EnqueueConditional (() => stackPanelLoaded, "StackPanelLayoutChanged #0");
			Enqueue (() => {
				stackPanelLayoutChanged = false;
				stackPanel.Children.Add (new Button () { Content = "big button", Width = 110 });
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #0");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				Assert.IsTrue (scrollProvider.HorizontallyScrollable, "HorizontallyScrollable #1");
				Assert.IsFalse (scrollProvider.VerticallyScrollable, "VerticallyScrollable #1");

				Assert.AreEqual (100, scrollProvider.VerticalViewSize, "VerticalViewSize #1");
				double viewsize = (scrollViewer.ViewportWidth * 100) / scrollViewer.ExtentWidth;
				Assert.AreEqual (viewsize, scrollProvider.HorizontalViewSize, "HorizontalViewSize #1");

				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #1");
				Assert.AreEqual (scrollViewer.HorizontalOffset, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #1");

				stackPanelLayoutChanged = false;
				stackPanel.Children.RemoveAt (0);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #1");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				Assert.IsFalse (scrollProvider.HorizontallyScrollable, "HorizontallyScrollable #2");
				Assert.IsFalse (scrollProvider.VerticallyScrollable, "VerticallyScrollable #2");

				Assert.AreEqual (100, scrollProvider.VerticalViewSize, "VerticalViewSize #2");
				Assert.AreEqual (100, scrollProvider.HorizontalViewSize, "HorizontalViewSize #2");

				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #2");
				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #2");

				stackPanelLayoutChanged = false;
				stackPanel.Children.Add (new Button () { Content = "big button", Width = 10, Height = 160 });
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #2");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				Assert.IsFalse (scrollProvider.HorizontallyScrollable, "HorizontallyScrollable #3");
				Assert.IsTrue (scrollProvider.VerticallyScrollable, "VerticallyScrollable #3");

				double viewsize = (scrollViewer.ViewportHeight * 100) / scrollViewer.ExtentHeight;
				Assert.AreEqual (viewsize, scrollProvider.VerticalViewSize, "VerticalViewSize #3");
				Assert.AreEqual (100, scrollProvider.HorizontalViewSize, "HorizontalViewSize #3");

				Assert.AreEqual (scrollViewer.VerticalOffset, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #3");
				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #3");

				stackPanelLayoutChanged = false;
				stackPanel.Children.RemoveAt (0);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #3");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				Assert.IsFalse (scrollProvider.HorizontallyScrollable, "HorizontallyScrollable #4");
				Assert.IsFalse (scrollProvider.VerticallyScrollable, "VerticallyScrollable #4");

				Assert.AreEqual (100, scrollProvider.VerticalViewSize, "VerticalViewSize #4");
				Assert.AreEqual (100, scrollProvider.HorizontalViewSize, "HorizontalViewSize #4");

				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #4");
				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #4");

				stackPanelLayoutChanged = false;
				stackPanel.Children.Add (new Button () { Content = "big button", Width = 10, Height = 160 });
				stackPanel.Children.Add (new Button () { Content = "big button", Width = 110 });
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #4");
			double verticalOffset = 0;
			double horizontalOffset = 0;
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				Assert.IsTrue (scrollProvider.HorizontallyScrollable, "HorizontallyScrollable #5");
				Assert.IsTrue (scrollProvider.VerticallyScrollable, "VerticallyScrollable #5");

				double viewsizeVertical = (scrollViewer.ViewportHeight * 100) / scrollViewer.ExtentHeight;
				Assert.AreEqual (viewsizeVertical, scrollProvider.VerticalViewSize, "VerticalViewSize #5");
				double viewsizeHorizontal = (scrollViewer.ViewportWidth * 100) / scrollViewer.ExtentWidth;
				Assert.AreEqual (viewsizeHorizontal, scrollProvider.HorizontalViewSize, "HorizontalViewSize #5");

				verticalOffset = scrollViewer.VerticalOffset;
				Assert.AreEqual (verticalOffset, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #5");
				horizontalOffset = scrollViewer.HorizontalOffset;
				Assert.AreEqual (horizontalOffset, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #5");

				stackPanel.Children.Add (new Button () { Content = "big button 2", Width = 10, Height = 160 });
				stackPanel.Children.Add (new Button () { Content = "big button 3", Width = 110 });
				stackPanel.Children.Add (new Button () { Content = "big button 4", Width = 10, Height = 160 });
				stackPanel.Children.Add (new Button () { Content = "big button 5", Width = 110 });
				
				stackPanelLayoutChanged = false;
				scrollProvider.Scroll (ScrollAmount.SmallIncrement, ScrollAmount.SmallIncrement);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #5");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				double newVerticalOffset = scrollViewer.VerticalOffset;
				double newHorizontalOffset = scrollViewer.HorizontalOffset;

				Assert.AreNotEqual (newVerticalOffset, verticalOffset, "Old/New VerticalViewSize #0");
				Assert.AreNotEqual (newHorizontalOffset, horizontalOffset, "Old/New HorizontalViewSize #0");

				verticalOffset = newVerticalOffset;
				horizontalOffset = newHorizontalOffset;

				stackPanelLayoutChanged = false;
				scrollProvider.Scroll (ScrollAmount.NoAmount, ScrollAmount.SmallIncrement);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #6");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				double newVerticalOffset = scrollViewer.VerticalOffset;
				double newHorizontalOffset = scrollViewer.HorizontalOffset;

				Assert.AreNotEqual (newVerticalOffset, verticalOffset, "Old/New VerticalViewSize #1");
				Assert.AreEqual (newHorizontalOffset, horizontalOffset, "Old/New HorizontalViewSize #1");

				verticalOffset = newVerticalOffset;
				horizontalOffset = newHorizontalOffset;

				stackPanelLayoutChanged = false;
				scrollProvider.Scroll (ScrollAmount.SmallIncrement, ScrollAmount.NoAmount);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #7");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				double newVerticalOffset = scrollViewer.VerticalOffset;
				double newHorizontalOffset = scrollViewer.HorizontalOffset;

				Assert.AreEqual (newVerticalOffset, verticalOffset, "Old/New VerticalViewSize #2");
				Assert.AreNotEqual (newHorizontalOffset, horizontalOffset, "Old/New HorizontalViewSize #2");

				verticalOffset = newVerticalOffset;
				horizontalOffset = newHorizontalOffset;

				stackPanelLayoutChanged = false;
				scrollProvider.Scroll (ScrollAmount.NoAmount, ScrollAmount.NoAmount);
			});
			EnqueueConditional (() => !stackPanelLayoutChanged, "StackPanelLayoutChanged #8");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				double newVerticalOffset = scrollViewer.VerticalOffset;
				double newHorizontalOffset = scrollViewer.HorizontalOffset;

				Assert.AreEqual (newVerticalOffset, verticalOffset, "Old/New VerticalViewSize #3");
				Assert.AreEqual (newHorizontalOffset, horizontalOffset, "Old/New HorizontalViewSize #3");

				stackPanelLayoutChanged = false;
				scrollProvider.SetScrollPercent (20, 30);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #9");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				Assert.AreEqual (30, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #6");
				Assert.AreEqual (20, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #6");

				stackPanelLayoutChanged = false;
				scrollProvider.SetScrollPercent (ScrollPatternIdentifiers.NoScroll, 50);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #10");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				Assert.AreEqual (50, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #7");
				Assert.AreEqual (20, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #7"); // same value

				stackPanelLayoutChanged = false;
				scrollProvider.SetScrollPercent (ScrollPatternIdentifiers.NoScroll, ScrollPatternIdentifiers.NoScroll);
			});
			EnqueueConditional (() => !stackPanelLayoutChanged, "StackPanelLayoutChanged #11");
			Enqueue (() => {
				IScrollProvider scrollProvider = ScrollProviderFromScrollViewer (scrollViewer);

				Assert.AreEqual (50, scrollProvider.VerticalScrollPercent, "VerticalScrollPercent #8"); // same value
				Assert.AreEqual (20, scrollProvider.HorizontalScrollPercent, "HorizontalScrollPercent #8"); // same value	
			});
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		public void IScrollProvider_HorizontalMethods ()
		{
			ScrollViewer viewer = new ScrollViewer () {
				HorizontalScrollBarVisibility = ScrollBarVisibility.Auto,
				VerticalScrollBarVisibility = ScrollBarVisibility.Auto
			};

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (viewer);
			IScrollProvider p = (IScrollProvider) peer.GetPattern (PatternInterface.Scroll);
			viewer.Content = new Button { Width = 1000, Height = 1000 };

			CreateAsyncTest (viewer,
				() => viewer.ApplyTemplate (),
				// Visible and MaxWidth = 0
				() => {
					viewer.MaxWidth = 0;
					viewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Visible;
				},
				() => Assert.IsTrue (p.HorizontallyScrollable, "#1"),
				() => Assert.AreEqual ((viewer.ViewportWidth * 100) / viewer.ExtentWidth, p.HorizontalViewSize, "#2"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.HorizontalScrollPercent, "#3"),
				// Visible and MaxWidth = 200
				() => viewer.MaxWidth = 200,
				() => Assert.IsTrue (p.HorizontallyScrollable, "#4"),
				() => Assert.AreEqual ((viewer.ViewportWidth * 100) / viewer.ExtentWidth, p.HorizontalViewSize, "#5"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.HorizontalScrollPercent, "#6"),
				// Hidden and MaxWidth = 0
				() => {
					viewer.MaxWidth = 0;
					viewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Hidden;
				},
				() => Assert.IsTrue (p.HorizontallyScrollable, "#7"),
				() => Assert.AreEqual ((viewer.ViewportWidth * 100) / viewer.ExtentWidth, p.HorizontalViewSize, "#8"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.HorizontalScrollPercent, "#9"),
				// Hidden and MaxWidth = 200
				() => viewer.MaxWidth = 200,
				() => Assert.IsTrue (p.HorizontallyScrollable, "#10"),
				() => Assert.AreEqual ((viewer.ViewportWidth * 100) / viewer.ExtentWidth, p.HorizontalViewSize, "#11"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.HorizontalScrollPercent, "#12"),
				// Disabled and MaxWidth = 0
				() => {
					viewer.MaxWidth = 0;
					viewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Disabled;
				},
				() => Assert.IsFalse (p.HorizontallyScrollable, "#13"),
				() => Assert.AreEqual (100d, p.HorizontalViewSize, "#14"),
				() => Assert.AreEqual (0d, viewer.ExtentWidth, "#15"),
				() => Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, p.HorizontalScrollPercent, "#16"),
				// Disabled and MaxWidth = 200
				() => viewer.MaxWidth = 200,
				() => Assert.IsFalse (p.HorizontallyScrollable, "#17"),
				() => Assert.AreEqual (100d, p.HorizontalViewSize, "#18"),
				() => Assert.AreNotEqual (0d, viewer.ExtentWidth, "#19"),
				() => Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, p.HorizontalScrollPercent, "#20"),
				// Auto and MaxWidth = 0
				() => {
					viewer.MaxWidth = 0;
					viewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Auto;
				},
				() => Assert.IsTrue (p.HorizontallyScrollable, "#21"),
				() => Assert.AreEqual ((viewer.ViewportWidth * 100) / viewer.ExtentWidth, p.HorizontalViewSize, "#22"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.HorizontalScrollPercent, "#23"),
				// Auto and MaxWidth = 200
				() => viewer.MaxWidth = 200,
				() => Assert.IsTrue (p.HorizontallyScrollable, "#24"),
				() => Assert.AreEqual ((viewer.ViewportWidth * 100) / viewer.ExtentWidth, p.HorizontalViewSize, "#25"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.HorizontalScrollPercent, "#26")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void IScrollProvider_VerticalMethods ()
		{
			ScrollViewer viewer = new ScrollViewer () {
				HorizontalScrollBarVisibility = ScrollBarVisibility.Auto,
				VerticalScrollBarVisibility = ScrollBarVisibility.Auto
			};

			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (viewer);
			IScrollProvider p = (IScrollProvider) peer.GetPattern (PatternInterface.Scroll);
			viewer.Content = new Button { Width = 1000, Height = 1000 };

			CreateAsyncTest (viewer,
				() => viewer.ApplyTemplate (),
				// Visible and MaxHeight = 0
				() => {
					viewer.MaxHeight = 0;
					viewer.VerticalScrollBarVisibility = ScrollBarVisibility.Visible;
				},
				() => Assert.IsTrue (p.VerticallyScrollable, "#1"),
				() => Assert.AreEqual ((viewer.ViewportHeight * 100) / viewer.ExtentHeight, p.VerticalViewSize, "#2"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.VerticalScrollPercent, "#3"),
				// Visible and MaxHeight = 200
				() => viewer.MaxHeight = 200,
				() => Assert.IsTrue (p.VerticallyScrollable, "#4"),
				() => Assert.AreEqual ((viewer.ViewportHeight * 100) / viewer.ExtentHeight, p.VerticalViewSize, "#5"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.VerticalScrollPercent, "#6"),
				// Hidden and MaxHeight = 0
				() => {
					viewer.MaxHeight = 0;
					viewer.VerticalScrollBarVisibility = ScrollBarVisibility.Hidden;
				},
				() => Assert.IsTrue (p.VerticallyScrollable, "#7"),
				() => Assert.AreEqual ((viewer.ViewportHeight * 100) / viewer.ExtentHeight, p.VerticalViewSize, "#8"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.VerticalScrollPercent, "#9"),
				// Hidden and MaxHeight = 200
				() => viewer.MaxHeight = 200,
				() => Assert.IsTrue (p.VerticallyScrollable, "#10"),
				() => Assert.AreEqual ((viewer.ViewportHeight * 100) / viewer.ExtentHeight, p.VerticalViewSize, "#11"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.VerticalScrollPercent, "#12"),
				// Disabled and MaxHeight = 0
				() => {
					viewer.MaxHeight = 0;
					viewer.VerticalScrollBarVisibility = ScrollBarVisibility.Disabled;
				},
				() => Assert.IsFalse (p.VerticallyScrollable, "#13"),
				() => Assert.AreEqual (100d, p.VerticalViewSize, "#14"),
				() => Assert.AreEqual (0d, viewer.ExtentHeight, "#15"),
				() => Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, p.VerticalScrollPercent, "#16"),
				// Disabled and MaxHeight = 200
				() => viewer.MaxHeight = 200,
				() => Assert.IsFalse (p.VerticallyScrollable, "#17"),
				() => Assert.AreEqual (100d, p.VerticalViewSize, "#18"),
				() => Assert.AreNotEqual (0d, viewer.ExtentHeight, "#19"),
				() => Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, p.VerticalScrollPercent, "#20"),
				// Auto and MaxHeight = 0
				() => {
					viewer.MaxHeight = 0;
					viewer.VerticalScrollBarVisibility = ScrollBarVisibility.Auto;
				},
				() => Assert.IsTrue (p.VerticallyScrollable, "#21"),
				() => Assert.AreEqual ((viewer.ViewportHeight * 100) / viewer.ExtentHeight, p.VerticalViewSize, "#22"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.VerticalScrollPercent, "#23"),
				// Auto and MaxHeight = 200
				() => viewer.MaxHeight = 200,
				() => Assert.IsTrue (p.VerticallyScrollable, "#24"),
				() => Assert.AreEqual ((viewer.ViewportHeight * 100) / viewer.ExtentHeight, p.VerticalViewSize, "#25"),
				() => Assert.AreEqual (viewer.VerticalOffset, p.VerticalScrollPercent, "#26")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void IScrollProvider_Events ()
		{
			if (!EventsManager.Instance.AutomationSingletonExists) {
				EnqueueTestComplete ();
				return;
			}

			bool scrollViewerLoaded = false;

			ScrollViewer scrollViewer = CreateConcreteFrameworkElement () as ScrollViewer;
			scrollViewer.Loaded += (o, e) => scrollViewerLoaded = true;
			scrollViewer.HorizontalScrollBarVisibility = ScrollBarVisibility.Auto;
			scrollViewer.VerticalScrollBarVisibility = ScrollBarVisibility.Auto;
			
			// We are going to use this canvas to define an explicit 
			// scrollviewer size and location
			Canvas canvas = new Canvas ();
			TestPanel.Children.Add (canvas);

			scrollViewer.SetValue (Canvas.LeftProperty, 5d);
			scrollViewer.SetValue (Canvas.TopProperty, 5d);
			scrollViewer.Width = 100;
			scrollViewer.Height = 150;
			canvas.Children.Add (scrollViewer);

			bool stackPanelLoaded = false;
			bool stackPanelLayoutChanged = false;
			StackPanel stackPanel = new StackPanel ();
			stackPanel.LayoutUpdated += (o, e) => stackPanelLayoutChanged = true;
			stackPanel.Loaded += (o, e) => stackPanelLoaded = true;

			AutomationPeer peer = null;
			IScrollProvider scrollProvider = null;
			AutomationPropertyEventTuple propertyTuple = null;
			double viewsizeVertical = 0;
			double viewsizeHorizontal = 0;
			double verticalOffset = 0;
			double horizontalOffset = 0;

			EnqueueConditional (() => scrollViewerLoaded, "ScrollViewerLoaded #0");
			Enqueue (() => {
				peer = FrameworkElementAutomationPeer.CreatePeerForElement (scrollViewer);
				Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");
				scrollProvider = (IScrollProvider) peer.GetPattern (PatternInterface.Scroll);

				scrollViewer.Content = stackPanel;
			});
			EnqueueConditional (() => stackPanelLoaded, "StackPanelLayoutChanged #0");
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				stackPanel.Children.Add (new Button () { Content = "big button", Width = 110 });
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #0");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticallyScrollableProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #0");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontallyScrollableProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #1");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetAutomationEventFrom.OldValue #1");
				Assert.IsTrue ((bool) propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #1");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalViewSizeProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #2");

				viewsizeHorizontal = (scrollViewer.ViewportWidth * 100) / scrollViewer.ExtentWidth;
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalViewSizeProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #3");
				Assert.AreEqual (100, (double) propertyTuple.OldValue, "GetAutomationEventFrom.OldValue #3");
				Assert.AreEqual (viewsizeHorizontal, (double) propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #3");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #4");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalScrollPercentProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #5");
				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, (double) propertyTuple.OldValue, "GetAutomationEventFrom.OldValue #5");
				Assert.AreEqual (scrollViewer.HorizontalOffset, (double) propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #5");
				horizontalOffset = scrollViewer.HorizontalOffset;
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				stackPanel.Children.RemoveAt (0);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #1");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticallyScrollableProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #6");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontallyScrollableProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #7");
				Assert.IsTrue ((bool) propertyTuple.OldValue, "GetAutomationEventFrom.OldValue #6");
				Assert.IsFalse ((bool) propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #6");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalViewSizeProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #8");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalViewSizeProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #9");
				Assert.AreEqual (viewsizeHorizontal, (double) propertyTuple.OldValue, "GetAutomationEventFrom.OldValue #7");
				Assert.AreEqual (100d, (double) propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #7");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #10");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalScrollPercentProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #11");
				Assert.AreEqual (horizontalOffset, (double) propertyTuple.OldValue, "GetAutomationEventFrom.OldValue #8");
				Assert.AreEqual (ScrollPatternIdentifiers.NoScroll, (double) propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #8");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				stackPanel.Children.Add (new Button () { Content = "big button", Width = 10, Height = 160 });
				stackPanel.Children.Add (new Button () { Content = "big button", Width = 110 });
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #2");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticallyScrollableProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #12");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetAutomationEventFrom.OldValue #12");
				Assert.IsTrue ((bool) propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #12");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontallyScrollableProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #13");
				Assert.IsFalse ((bool) propertyTuple.OldValue, "GetAutomationEventFrom.OldValue #13");
				Assert.IsTrue ((bool) propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #13");

				viewsizeVertical = (scrollViewer.ViewportHeight * 100) / scrollViewer.ExtentHeight;
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalViewSizeProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #14");
				Assert.AreEqual (viewsizeVertical, scrollProvider.VerticalViewSize, "VerticalViewSize #1");
				
				viewsizeHorizontal = (scrollViewer.ViewportWidth * 100) / scrollViewer.ExtentWidth;
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalViewSizeProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #15");
				Assert.AreEqual (viewsizeHorizontal, scrollProvider.HorizontalViewSize, "HorizontalViewSize #1");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanel.Children.Add (new Button () { Content = "big button 2", Width = 10, Height = 160 });
				stackPanel.Children.Add (new Button () { Content = "big button 3", Width = 110 });
				stackPanel.Children.Add (new Button () { Content = "big button 4", Width = 10, Height = 160 });
				stackPanel.Children.Add (new Button () { Content = "big button 5", Width = 110 });
				
				stackPanelLayoutChanged = false;
				scrollProvider.Scroll (ScrollAmount.SmallIncrement, ScrollAmount.SmallIncrement);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #3");
			Enqueue (() => {
				double newVerticalOffset = scrollViewer.VerticalOffset;
				double newHorizontalOffset = scrollViewer.HorizontalOffset;

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalViewSizeProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #16");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalViewSizeProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #17");

				Assert.AreNotEqual (newVerticalOffset, verticalOffset, "Old/New VerticalViewSize #0");
				Assert.AreNotEqual (newHorizontalOffset, horizontalOffset, "Old/New HorizontalViewSize #0");

				verticalOffset = newVerticalOffset;
				horizontalOffset = newHorizontalOffset;
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				scrollProvider.Scroll (ScrollAmount.NoAmount, ScrollAmount.SmallIncrement);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #4");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalScrollPercentProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #18");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #19");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				scrollProvider.Scroll (ScrollAmount.SmallIncrement, ScrollAmount.NoAmount);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #5");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #20");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalScrollPercentProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #21");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				scrollProvider.Scroll (ScrollAmount.NoAmount, ScrollAmount.NoAmount);
			});
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #22");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #23");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				scrollProvider.SetScrollPercent (20, 30);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #7");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalScrollPercentProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #22");
				Assert.AreEqual (30d, propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #22");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalScrollPercentProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #23");
				Assert.AreEqual (20d, propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #23");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				scrollProvider.SetScrollPercent (ScrollPatternIdentifiers.NoScroll, 50);
			});
			EnqueueConditional (() => stackPanelLayoutChanged, "StackPanelLayoutChanged #8");
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalScrollPercentProperty);
				Assert.IsNotNull (propertyTuple, "GetAutomationEventFrom #24");
				Assert.AreEqual (50d, propertyTuple.NewValue, "GetAutomationEventFrom.NewValue #24");
				Assert.AreEqual (30d, propertyTuple.OldValue, "GetAutomationEventFrom.NewValue #24");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #25");
			});
			Enqueue (() => {
				EventsManager.Instance.Reset ();
				stackPanelLayoutChanged = false;
				scrollProvider.SetScrollPercent (ScrollPatternIdentifiers.NoScroll, ScrollPatternIdentifiers.NoScroll);
			});
			Enqueue (() => {
				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.VerticalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #26");

				propertyTuple = EventsManager.Instance.GetAutomationEventFrom (peer, 
				                                                               ScrollPatternIdentifiers.HorizontalScrollPercentProperty);
				Assert.IsNull (propertyTuple, "GetAutomationEventFrom #27");
			});
			EnqueueTestComplete ();
		}

		private IScrollProvider ScrollProviderFromScrollViewer (ScrollViewer viewer)
		{
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (viewer);
			Assert.IsNotNull (peer, "FrameworkElementAutomationPeer.CreatePeerForElement");
			return (IScrollProvider) peer.GetPattern (PatternInterface.Scroll);
		}

		#endregion

		protected override FrameworkElement CreateConcreteFrameworkElement ()
		{
			ScrollViewer control = new ScrollViewer ();
			control.HorizontalScrollBarVisibility = ScrollBarVisibility.Auto;
			control.VerticalScrollBarVisibility = ScrollBarVisibility.Auto;
			return control;
		}

		protected override FrameworkElementAutomationPeerContract CreateConcreteFrameworkElementAutomationPeer (FrameworkElement element)
		{
			return new ScrollViewerAutomationPeerConcrete (element as ScrollViewer);
		}
	}
}
