// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Collections.Specialized;
using System.Globalization;
using System.Windows.Automation.Peers;
using System.Windows.Automation.Provider;
using System.Windows.Controls.Primitives;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace System.Windows.Controls.Test
{
    /// <summary>
    /// This class runs the unit tests for TabControl
    /// </summary>
    [TestClass]
    public class TabControlTest : SilverlightControlTest
    {
        /// <summary>
        /// Validate the default property values on the TabControl class.
        /// </summary>
        [TestMethod]
        [Description("Validate the default property values on the TabControl class.")]
        public void TabControlDefaultValuesTest()
        {
            TabControl tabControl = new TabControl();

            Assert.IsTrue(tabControl.IsEnabled);
            Assert.AreEqual(tabControl.TabStripPlacement, Dock.Top);
            Assert.AreEqual(tabControl.SelectedIndex, -1);
            Assert.IsNull(tabControl.SelectedContent);
        }
        
        /// <summary>
        /// Validate that the TabControl throws an exception when a non-TabItem is added as a child.
        /// </summary>
        [TestMethod]
        [Description("Validate that the TabControl throws an exception when a non-TabItem is added as a child.")]
        public void TabControlNonTabItemChildTest()
        {
            TabControl tabControl = new TabControl();
            string exceptionString = string.Empty;

            try
            {
                tabControl.Items.Add("Test");
            }
            catch (ArgumentException e)
            {
                exceptionString = e.Message;
            }

            Assert.AreEqual(exceptionString, string.Format(CultureInfo.InvariantCulture, Resource.TabControl_InvalidChild, typeof(string).ToString()));
        }

        [TestMethod]
        [Description("Ensures that IsEnabled property can be set.")]
        public void SetIsEnabled()
        {
            TabControl tabControl = new TabControl();
            Assert.IsTrue(tabControl.IsEnabled);
            tabControl.IsEnabled = false;
            Assert.IsFalse(tabControl.IsEnabled);
            tabControl.IsEnabled = true;
            Assert.IsTrue(tabControl.IsEnabled);
        }

        /// <summary>
        /// Verify TabControl's TabItem children are updated when IsEnabled changes.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's TabItem children are updated when IsEnabled changes.")]
        [Asynchronous]
        [MoonlightBug ()]
        public void TabItemIsEnabledPropertyUpdateTest()
        {
            TabControl tabControl = new TabControl();

            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);

            CreateAsyncTask(tabControl,
                () => Assert.IsTrue(tabControl.IsEnabled),
                () => Assert.IsTrue(tabItem1.IsEnabled),
                () => Assert.IsTrue(tabItem2.IsEnabled),

                () => tabItem1.IsEnabled = false,
                () => tabControl.IsEnabled = false,
                () => Assert.IsFalse(tabItem1.IsEnabled),
                () => Assert.IsFalse(tabItem2.IsEnabled),

                () => tabControl.IsEnabled = true,
                () => Assert.IsFalse(tabItem1.IsEnabled),
                () => Assert.IsTrue(tabItem2.IsEnabled)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's keeps previous selection if we select an item outside the TabControl.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's keeps previous selection if we select an item outside the TabControl.")]
        [Asynchronous]
        public void SelectItemOutsideControlTest()
        {
            TabControl tabControl = new TabControl();

            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            tabControl.Items.Add(tabItem1);

            CreateAsyncTask(tabControl,
                () => Assert.AreEqual(tabControl.SelectedItem, tabItem1),
                () => Assert.AreEqual(tabControl.SelectedIndex, 0),

                // select item not in TabControl and verify that 
                // the previous selection is maintained
                () => tabControl.SelectedItem = tabItem2,
                () => Assert.AreEqual(tabControl.SelectedItem, tabItem1),
                () => Assert.AreEqual(tabControl.SelectedIndex, 0)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's SelectedIndex property.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's SelectedIndex property.")]
        [Asynchronous]
        public void SelectedIndexTest()
        {
            TabControl tabControl = new TabControl();
            Assert.AreEqual(tabControl.SelectedIndex, -1);

            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(tabItem1),
                () => Assert.AreEqual(tabControl.SelectedIndex, 0),

                () => tabControl.Items.Remove(tabItem1),
                () => Assert.AreEqual(tabControl.SelectedIndex, -1),

                () => tabControl.Items.Add(tabItem1),
                () => tabControl.Items.Add(tabItem2),
                () => tabControl.SelectedIndex = 1,
                () => tabControl.Items.Remove(tabItem2),
                () => Assert.AreEqual(tabControl.SelectedIndex, 0)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's SelectedIndex property handles coercion.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's SelectedIndex property handles coercion.")]
        [Asynchronous]
        public void SelectedIndexCoercionTest()
        {
            TabControl tabControl = new TabControl();
            Assert.AreEqual(tabControl.SelectedIndex, -1);

            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(tabItem1),
                () => tabControl.SelectedIndex = 1,
                () => Assert.AreEqual(tabControl.SelectedIndex, 0),

                () => tabControl.Items.Add(tabItem2),
                () => Assert.AreEqual(tabControl.SelectedIndex, 1)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's SelectedItem property.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's SelectedItem property.")]
        [Asynchronous]
        public void SelectedItemTest()
        {
            TabControl tabControl = new TabControl();
            Assert.AreEqual(tabControl.SelectedItem, null);

            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(tabItem1),
                () => Assert.AreEqual(tabControl.SelectedItem, tabItem1),

                () => tabControl.Items.Remove(tabItem1),
                () => Assert.AreEqual(tabControl.SelectedItem, null),

                () => tabControl.Items.Add(tabItem1),
                () => tabControl.Items.Add(tabItem2),
                () => tabControl.SelectedItem = tabItem2,
                () => Assert.AreEqual(tabControl.SelectedItem, tabItem2),
                () => tabControl.Items.Remove(tabItem2),
                () => Assert.AreEqual(tabControl.SelectedItem, tabItem1)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl handles selection correctly when TabItems with IsSelected=false are passed in.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl handles selection correctly when TabItems with IsSelected=false are passed in.")]
        [Asynchronous]
        public void AddingItemWithIsSelectedFalseTest()
        {
            TabControl tabControl = new TabControl();
            Assert.AreEqual(tabControl.SelectedItem, null);

            TabItem tabItem1 = new TabItem();
            tabItem1.IsSelected = false;
            TabItem tabItem2 = new TabItem();
            tabItem2.IsSelected = false;

            CreateAsyncTask(tabControl,
                delegate 
                {
                    tabControl.Items.Add(tabItem1);
                    Assert.IsNull(tabControl.SelectedItem);
                    tabControl.Items.Add(tabItem2);
                    Assert.AreEqual(tabControl.SelectedItem, tabItem1);
                }
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's SelectedItem and SelectedIndex properties update each other.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's SelectedItem and SelectedIndex properties update each other.")]
        [Asynchronous]
        public void SelectedItemAndIndexSyncTest()
        {
            TabControl tabControl = new TabControl();

            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(tabItem1),
                () => tabControl.Items.Add(tabItem2),

                () => tabControl.SelectedItem = tabItem2,
                () => Assert.AreEqual(tabControl.SelectedIndex, 1),

                () => tabControl.SelectedIndex = 0,
                () => Assert.AreEqual(tabControl.SelectedItem, tabItem1)                
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's OnSelectionChanged override function gets called.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's OnSelectionChanged override function gets called.")]
        [Asynchronous]
        public void OnSelectionChangedOverrideTest()
        {
            TestTabControl tabControl = new TestTabControl();
            Assert.AreEqual(tabControl.TestString, null);
            Assert.AreEqual(tabControl.SelectedIndex, -1);

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(new TabItem()),
                () => Assert.AreEqual(tabControl.TestString, "SelectionChanged")
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's SelectionChanged event.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's SelectionChanged event.")]
        [Asynchronous]
        public void SelectionChangedEventTest()
        {
            TabControl tabControl = new TabControl();
            string testString = String.Empty;
            tabControl.SelectionChanged += delegate { testString = "SelectionChanged"; };
            Assert.AreEqual(tabControl.SelectedIndex, -1);

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(new TabItem()),
                () => Assert.AreEqual(testString, "SelectionChanged")
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's SelectedContent property.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's SelectedContent property.")]
        [Asynchronous]
        public void SelectedContentTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem = new TabItem();
            tabItem.Content = "Test Content";
            Assert.IsNull(tabControl.SelectedContent);

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(tabItem),
                () => Assert.AreEqual(tabControl.SelectedContent, "Test Content")
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's SelectedContent property gets 
        /// updated when TabItem's content is updated.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's SelectedContent property gets updated when TabItem's content is updated.")]
        [Asynchronous]
        public void SelectedContentChangedTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem = new TabItem();
            tabItem.Content = "Test Content";
            Assert.IsNull(tabControl.SelectedContent);

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(tabItem),
                () => Assert.AreEqual(tabControl.SelectedContent, "Test Content"),
                () => tabItem.Content = "New Content",
                () => Assert.AreEqual(tabControl.SelectedContent, "New Content")
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's OnItemsChanged override 
        /// gets called when Items are modified
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's OnItemsChanged override gets called when Items are modified.")]
        [Asynchronous]
        public void OnItemsChangedTest()
        {
            TestOnItemsChanged tabControl = new TestOnItemsChanged();

            TabItem tabItem1 = new TabItem();
            tabItem1.Content = "Tab1";

            TabItem tabItem2 = new TabItem();
            tabItem2.Content = "Tab2";

            CreateAsyncTask(tabControl,
                () => Assert.IsNull(tabControl.TestString),

                () => tabControl.Items.Add(tabItem1),
                () => Assert.AreEqual(tabControl.TestString, "Add"),

                () => tabControl.Items.Remove(tabItem1),
                () => Assert.AreEqual(tabControl.TestString, "Remove"),

                () => tabControl.Items.Add(tabItem1),
                () => tabControl.Items.Add(tabItem2),
                () => tabControl.Items.Clear(),
                () => Assert.AreEqual(tabControl.TestString, "Reset")
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's FindNextTabItem function.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's FindNextTabItem function.")]
        public void FindNextTabItemTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();
            TabItem tabItem3 = new TabItem();

            tabItem1.IsEnabled = true;
            tabItem2.IsEnabled = true;
            tabItem3.IsEnabled = true;
            
            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);
            tabControl.Items.Add(tabItem3);

            Assert.AreEqual(tabItem2, tabControl.TestHook.FindNextTabItem(0, 1));
            Assert.AreEqual(tabItem3, tabControl.TestHook.FindNextTabItem(1, 1));
            Assert.AreEqual(tabItem1, tabControl.TestHook.FindNextTabItem(2, 1));
        }

        /// <summary>
        /// Verify TabControl's TabStripPlacement property.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's TabStripPlacement property.")]
        [Asynchronous]
        public void TabStripPlacementTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem = new TabItem();

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(tabItem),
                () => Assert.AreEqual(tabControl.ElementTemplateTop, tabControl.GetTemplate(tabControl.TabStripPlacement)),

                () => tabControl.TabStripPlacement = Dock.Bottom,
                () => Assert.AreEqual(tabControl.ElementTemplateBottom, tabControl.GetTemplate(tabControl.TabStripPlacement)),

                () => tabControl.TabStripPlacement = Dock.Left,
                () => Assert.AreEqual(tabControl.ElementTemplateLeft, tabControl.GetTemplate(tabControl.TabStripPlacement)),

                () => tabControl.TabStripPlacement = Dock.Right,
                () => Assert.AreEqual(tabControl.ElementTemplateRight, tabControl.GetTemplate(tabControl.TabStripPlacement))
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's AutomationPeer.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's AutomationPeer.")]
        [Asynchronous]
        public void TabControlAutomationPeerTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem = new TabItem();
            TabControlAutomationPeer peer = ((TabControlAutomationPeer)TabControlAutomationPeer.CreatePeerForElement(tabControl));
            ISelectionProvider selector = (ISelectionProvider)peer.GetPattern(PatternInterface.Selection);

            CreateAsyncTask(tabControl,                
                () => Assert.IsFalse(selector.CanSelectMultiple),
                () => Assert.IsTrue(selector.IsSelectionRequired),

                () => Assert.IsNull(selector.GetSelection()),
                () => tabControl.Items.Add(tabItem),
                () => Assert.IsNotNull(selector.GetSelection())
            );

            EnqueueTestComplete();
        }


        /// <summary>
        /// Verify TabControl's template parts.
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's template parts.")]
        public void TemplatePartsAreDefined()
        {
            IDictionary<string, Type> templateParts = typeof(TabControl).GetTemplateParts();
            templateParts.AssertTemplatePartDefined("TemplateTop", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateBottom", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateLeft", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateRight", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TabPanelTop", typeof(TabPanel));
            templateParts.AssertTemplatePartDefined("TabPanelBottom", typeof(TabPanel));
            templateParts.AssertTemplatePartDefined("TabPanelLeft", typeof(TabPanel));
            templateParts.AssertTemplatePartDefined("TabPanelRight", typeof(TabPanel));
            templateParts.AssertTemplatePartDefined("ContentTop", typeof(ContentPresenter));
            templateParts.AssertTemplatePartDefined("ContentBottom", typeof(ContentPresenter));
            templateParts.AssertTemplatePartDefined("ContentLeft", typeof(ContentPresenter));
            templateParts.AssertTemplatePartDefined("ContentRight", typeof(ContentPresenter));
        }

        public class TestTabControl : TabControl
        {
            public string TestString { get; set; }

            protected override void OnSelectionChanged(SelectionChangedEventArgs args)
            {
                base.OnSelectionChanged(args);
                TestString = "SelectionChanged";
            }
        }

        public class TestOnItemsChanged : TabControl
        {
            public string TestString { get; set; }

            protected override void OnItemsChanged(System.Collections.Specialized.NotifyCollectionChangedEventArgs e)
            {
                base.OnItemsChanged(e);
                if (e.Action == NotifyCollectionChangedAction.Add)
                {
                    TestString = "Add";
                }
                else if (e.Action == NotifyCollectionChangedAction.Remove)
                {
                    TestString = "Remove";
                }
                else if (e.Action == NotifyCollectionChangedAction.Reset)
                {
                    TestString = "Reset";
                }
            }
        }
    }
}
