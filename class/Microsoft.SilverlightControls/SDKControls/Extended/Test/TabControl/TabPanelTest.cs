// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Controls.Primitives;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;

namespace System.Windows.Controls.Test
{
    /// <summary>
    /// This class runs the unit tests for TabPanel
    /// </summary>
    [TestClass]
    public class TabPanelTest : SilverlightControlTest
    {
        /// <summary>
        /// Validate the TabControlParent property on the TabPanel class.
        /// </summary>
        [TestMethod]
        [Description("Validate the TabControlParent property on the TabPanel class.")]
        [Asynchronous]
        public void TabControlParentTest()
        {
            TabPanel tabPanel = new TabPanel();
            Assert.IsNull(tabPanel.TabControlParent);

            TabControl tabControl = new TabControl();
            CreateAsyncTask(tabControl,
                () => Assert.AreEqual(tabControl.ElementTabPanelTop.TabControlParent, tabControl),
                () => Assert.AreEqual(tabControl.ElementTabPanelBottom.TabControlParent, tabControl),
                () => Assert.AreEqual(tabControl.ElementTabPanelLeft.TabControlParent, tabControl),
                () => Assert.AreEqual(tabControl.ElementTabPanelRight.TabControlParent, tabControl)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Validate the GetDesiredSizeWithoutMargin function.
        /// </summary>
        [TestMethod]
        [Description("Validate the GetDesiredSizeWithoutMargin function.")]
        [Asynchronous]
        public void GetDesiredSizeWithoutMarginTest()
        {
            Size size = new Size();
            TabItem tabItem = new TabItem();
            tabItem.Height = 100;
            tabItem.Width = 100;

            CreateAsyncTask(tabItem,
                () => tabItem.Margin = new Thickness(5, 5, 5, 5),
                () => size = TabPanel.GetDesiredSizeWithoutMargin(tabItem),
                () => Assert.AreEqual(size.Height, 100),
                () => Assert.AreEqual(size.Width, 100)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Validate the GetHeadersSizeTest function.
        /// </summary>
        [TestMethod]
        [Description("Validate the GetHeadersSizeTest function.")]
        [Asynchronous]
        public void GetHeadersSizeTest()
        {
            TabControl tabControl = new TabControl();
            
            tabControl.Height = 100;
            tabControl.Width = 200;

            double[] sizes = new double[3];
            
            TabItem tabItem1 = new TabItem();
            tabItem1.Height = 10;
            tabItem1.Width = 10;
            TabItem tabItem2 = new TabItem();
            tabItem2.Height = 10;
            tabItem2.Width = 20;
            TabItem tabItem3 = new TabItem();
            tabItem3.Height = 10;
            tabItem3.Width = 30;

            tabControl.Items.Add(tabItem1);
            tabControl.Items.Add(tabItem2);
            tabControl.Items.Add(tabItem3);

            FrameworkElement fe = null;
            double selectedMarginSize = 0;
            CreateAsyncTask(tabControl,
                () => sizes = tabControl.ElementTabPanelTop.GetHeadersSize(),
                // First Item Is Selected
                () => fe = (tabItem1.GetTemplate(tabItem1.IsSelected, tabItem1.TabStripPlacement) as Panel).Children[0] as FrameworkElement,
                () => selectedMarginSize += (Math.Abs(fe.Margin.Left + fe.Margin.Right)),
                () => Assert.AreEqual(sizes[0], 10.0 + selectedMarginSize),
                () => Assert.AreEqual(sizes[1], 20.0),
                () => Assert.AreEqual(sizes[2], 30.0)
            );

            EnqueueTestComplete();
        }

        /// <summary>
        /// Verify TabControl's LayoutTabHeaders function
        /// </summary>
        [TestMethod]
        [Description("Verify TabControl's LayoutTabHeaders function.")]
        [Asynchronous]
        public void LayoutTabHeadersTest()
        {
            TabControl tabControl = new TabControl();
            tabControl.Width = 100;
            tabControl.Height = 100;

            TabItem tabItem1 = new TabItem();
            tabItem1.Width = 30;
            tabItem1.Height = 20;

            TabItem tabItem2 = new TabItem();
            tabItem2.Width = 60;
            tabItem2.Height = 20;

            TabItem tabItem3 = new TabItem();
            tabItem3.Width = 70;
            tabItem3.Height = 25;

            CreateAsyncTask(tabControl,
                () => tabControl.Items.Add(tabItem1),
                () => tabControl.Items.Add(tabItem2),
                () => Assert.AreEqual(tabControl.ElementTabPanelTop._numRows, 1),

                () => tabControl.Items.Add(tabItem3),
                () => Assert.AreEqual(tabControl.ElementTabPanelTop._numRows, 2),

                () => tabControl.Items.Remove(tabItem3),
                () => Assert.AreEqual(tabControl.ElementTabPanelTop._numRows, 1)
            );

            EnqueueTestComplete();
        }
    }
}
