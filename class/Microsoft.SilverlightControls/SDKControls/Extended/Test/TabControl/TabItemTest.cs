// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace System.Windows.Controls.Test
{
    /// <summary>
    /// This class runs the unit tests for TabItem
    /// </summary>
    [TestClass]
    public class TabItemTest : SilverlightControlTest
    {
        /// <summary>
        /// Validate the default property values on the TabItem class.
        /// </summary>
        [TestMethod]
        [Description("Validate the default property values on the TabItem class.")]
        public void TabItemDefaultValuesTest()
        {
            TabItem tabItem = new TabItem();

            Assert.IsNull(tabItem.Header);
            Assert.IsFalse(tabItem.IsSelected);
            Assert.IsNull(tabItem.TestHook.TabControlParent);
        }

        /// <summary>
        /// Ensures that IsEnabled property can be set.
        /// </summary>
        [TestMethod]
        [Description("Ensures that IsEnabled property can be set.")]
        public void SetIsEnabled()
        {
            TabItem tabItem = new TabItem();
            tabItem.IsEnabled = false;
            Assert.IsFalse(tabItem.IsEnabled);
            tabItem.IsEnabled = true;
            Assert.IsTrue(tabItem.IsEnabled);
        }

        /// <summary>
        /// Validate the OnSelected function.
        /// </summary>
        [TestMethod]
        [Description("Validate the OnSelected function.")]
        public void OnSelectedTest()
        {
            TestTabItem tabItem = new TestTabItem();
            Assert.IsNull(tabItem.TestString);

            tabItem.IsSelected = false;
            tabItem.IsSelected = true;
            Assert.AreEqual(tabItem.TestString, "Selected");
        }

        /// <summary>
        /// Validate the OnUnselected function.
        /// </summary>
        [TestMethod]
        [Description("Validate the OnUnselected function.")]
        public void OnUnselectedTest()
        {
            TestTabItem tabItem = new TestTabItem();
            Assert.IsNull(tabItem.TestString);

            tabItem.IsSelected = true;
            tabItem.IsSelected = false;
            Assert.AreEqual(tabItem.TestString, "Unselected");
        }

        /// <summary>
        /// Verify TabItem's TabStripPlacement property.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItem's TabStripPlacement property.")]
        public void TabStripPlacementTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem = new TabItem();

            Assert.AreEqual(tabItem.TabStripPlacement, Dock.Top);

            tabControl.Items.Add(tabItem);
            tabItem.IsSelected = true;

            tabControl.TabStripPlacement = Dock.Bottom;
            Assert.AreEqual(tabItem.TabStripPlacement, Dock.Bottom);

            tabControl.TabStripPlacement = Dock.Left;
            Assert.AreEqual(tabItem.TabStripPlacement, Dock.Left);

            tabControl.TabStripPlacement = Dock.Right;
            Assert.AreEqual(tabItem.TabStripPlacement, Dock.Right);

            tabControl.TabStripPlacement = Dock.Top;
            Assert.AreEqual(tabItem.TabStripPlacement, Dock.Top);
        }

        /// <summary>
        /// Verify TabItem's Header property.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItem's Header and HasHeader properties.")]
        public void HeaderTest()
        {
            TabItem tabItem = new TabItem();
            Assert.IsNull(tabItem.Header);
            Assert.IsFalse(tabItem.HasHeader);

            tabItem.Header = "test";
            Assert.AreEqual(tabItem.Header, "test");
            Assert.IsTrue(tabItem.HasHeader);

            Button button = new Button();
            tabItem.Header = button;
            Assert.AreEqual(tabItem.Header, button);
        }

        /// <summary>
        /// Verify TabItem's HeaderTemplate property.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItem's HeaderTemplate property.")]
        [Asynchronous]
        public void HeaderTemplateTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem = new TabItem();
            tabControl.Items.Add(tabItem);

            Assert.IsNull(tabItem.HeaderTemplate);

            CreateAsyncTask(tabControl,
                () => tabItem.Header = "TestHeader",
                () => Assert.AreEqual(tabItem.GetContentControl(tabItem.IsSelected, tabItem.TabStripPlacement).Content, "TestHeader"),

                () => tabItem.HeaderTemplate = new DataTemplate(),
                () => Assert.AreNotEqual(tabItem.GetContentControl(tabItem.IsSelected, tabItem.TabStripPlacement).Content, "TestHeader"),

                () => tabItem.Header = "NewHeader",
                () => Assert.AreNotEqual(tabItem.GetContentControl(tabItem.IsSelected, tabItem.TabStripPlacement).Content, "NewHeader"),

                () => tabItem.HeaderTemplate = null,
                () => Assert.AreEqual(tabItem.GetContentControl(tabItem.IsSelected, tabItem.TabStripPlacement).Content, "NewHeader")
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabItems in the TabControl can share the same content if it is null.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItems in the TabControl can share the same content if it is null.")]
        [Asynchronous]
        public void DuplicateNullContentTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);

            bool errorThrown = false;

            try
            {
                tabItem1.Content = null;
                tabItem2.Content = null;
            }
            catch
            {
                errorThrown = true;
            }
            finally
            {
                Assert.IsFalse(errorThrown);
            }

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabItems in the TabControl Cannot Share the Same Content.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItems in the TabControl Cannot Share the Same Content.")]
        [Asynchronous]
        public void DuplicateContentTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);

            // It should be okay to share non-ui content (so Tab1 and Tab2
            // can both share the same string as content)
            tabItem1.Content = "Tab1";
            tabItem2.Content = tabItem1.Content;

            // It should however throw an exception if they share the same
            // UIElement as content
            bool exceptionThrown = false;
            Button button = new Button();
            tabItem1.Content = button;
            try
            {
                tabItem2.Content = tabItem1.Content;
            }
            catch
            {
                exceptionThrown = true;
            }
            finally
            {
                Assert.IsTrue(exceptionThrown);
            }

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabItems in the TabControl can display a UI based Header correctly.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItems in the TabControl can display a UI based Header correctly.")]
        [Asynchronous]
        public void UIHeaderTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();
            Button header1 = new Button();
            TextBox header2 = new TextBox();
            
            header1.Content = "Tab1";
            header2.Text = "Tab2";

            tabItem1.Header = header1;
            tabItem2.Header = header2;

            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);

            CreateAsyncTask(tabControl,
                () => Assert.AreEqual(tabItem1.Header, header1),
                () => Assert.AreEqual(tabItem2.Header, header2)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabItems in the TabControl can display UI based Content correctly.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItems in the TabControl can display UI based Content correctly.")]
        [Asynchronous]
        public void UIContentTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();
            Button content1 = new Button();
            TextBox content2 = new TextBox();
            
            content1.Content = "Tab1";
            content2.Text = "Tab2";

            tabItem1.Content = content1;
            tabItem2.Content = content2;

            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);

            CreateAsyncTask(tabControl,
                () => Assert.AreEqual(tabItem1.Content, content1),
                () => Assert.AreEqual(tabItem2.Content, content2),

                () => Assert.AreEqual(tabControl.SelectedContent, content1),
                () => tabControl.SelectedIndex = 1,
                () => Assert.AreEqual(tabControl.SelectedContent, content2),
                () => tabControl.SelectedIndex = 0,
                () => Assert.AreEqual(tabControl.SelectedContent, content1)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabItem's IsSelected property.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItem's IsSelected property.")]
        public void IsSelectedTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();

            Assert.IsFalse(tabItem1.IsSelected);
            Assert.IsFalse(tabItem2.IsSelected);

            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);

            Assert.IsTrue(tabItem1.IsSelected);

            tabControl.SelectedIndex = 1;

            Assert.IsFalse(tabItem1.IsSelected);
            Assert.IsTrue(tabItem2.IsSelected);
        }

        /// <summary>
        /// Verify TabItem's UpdateVisualState function.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItem UpdateVisualState function.")]
        public void UpdateVisualStateTest()
        {
            TabControl tabControl = new TabControl();
            TabItem tabItem = new TabItem();
            tabControl.Items.Add(tabItem);
            tabItem.IsSelected = true;

            tabControl.TabStripPlacement = Dock.Top;
            tabItem.UpdateVisualState();
            Assert.AreEqual(tabItem.ElementTemplateTopSelected, tabItem.GetTemplate(tabItem.IsSelected, tabItem.TabStripPlacement));

            tabControl.TabStripPlacement = Dock.Bottom;
            tabItem.UpdateVisualState();
            Assert.AreEqual(tabItem.ElementTemplateBottomSelected, tabItem.GetTemplate(tabItem.IsSelected, tabItem.TabStripPlacement));
            
            tabControl.TabStripPlacement = Dock.Left;
            tabItem.UpdateVisualState();
            Assert.AreEqual(tabItem.ElementTemplateLeftSelected, tabItem.GetTemplate(tabItem.IsSelected, tabItem.TabStripPlacement));

            tabControl.TabStripPlacement = Dock.Right;
            tabItem.UpdateVisualState();
            Assert.AreEqual(tabItem.ElementTemplateRightSelected, tabItem.GetTemplate(tabItem.IsSelected, tabItem.TabStripPlacement));
        }

        /// <summary>
        /// Verify TabItem MouseEvents.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItem MouseEvents.")]
        public void MouseEventsTest()
        {
            TabControl tabControl = new TabControl();
            tabControl.IsEnabled = true;
            TabItem tabItem1 = new TabItem();
            TabItem tabItem2 = new TabItem();
            tabItem2.IsEnabled = true;
            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);

            tabItem2.TestHook.OnMouseEnter(tabItem2, null);
            Assert.IsTrue(tabItem2.TestHook.IsMouseOver);

            tabItem2.TestHook.OnMouseLeave(tabItem2, null);
            Assert.IsFalse(tabItem2.TestHook.IsMouseOver);
        }

        /// <summary>
        /// Verify TabItem's template parts.
        /// </summary>
        [TestMethod]
        [Description("Verify TabItem's template parts.")]
        public void TemplatePartsAreDefined()
        {
            IDictionary<string, Type> templateParts = typeof(TabItem).GetTemplateParts();
            templateParts.AssertTemplatePartDefined("TemplateTopSelected", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateBottomSelected", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateLeftSelected", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateRightSelected", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateTopUnselected", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateBottomUnselected", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateLeftUnselected", typeof(FrameworkElement));
            templateParts.AssertTemplatePartDefined("TemplateRightUnselected", typeof(FrameworkElement));

            templateParts.AssertTemplatePartDefined("HeaderTopSelected", typeof(ContentControl));
            templateParts.AssertTemplatePartDefined("HeaderBottomSelected", typeof(ContentControl));
            templateParts.AssertTemplatePartDefined("HeaderLeftSelected", typeof(ContentControl));
            templateParts.AssertTemplatePartDefined("HeaderRightSelected", typeof(ContentControl));
            templateParts.AssertTemplatePartDefined("HeaderTopUnselected", typeof(ContentControl));
            templateParts.AssertTemplatePartDefined("HeaderBottomUnselected", typeof(ContentControl));
            templateParts.AssertTemplatePartDefined("HeaderLeftUnselected", typeof(ContentControl));
            templateParts.AssertTemplatePartDefined("HeaderRightUnselected", typeof(ContentControl));
        }
    }

    public class TestTabItem : TabItem
    {
        public string TestString { get; set; }

        protected override void OnSelected(RoutedEventArgs e)
        {
            base.OnSelected(e);
            TestString = "Selected";
        }

        protected override void OnUnselected(RoutedEventArgs e)
        {
            base.OnUnselected(e);
            TestString = "Unselected";
        }
    }
}
