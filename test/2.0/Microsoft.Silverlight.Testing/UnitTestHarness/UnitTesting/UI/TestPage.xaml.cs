// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// A user control that should be used as the root visual for a Silverlight 
    /// plugin if developers would like to use the advanced TestSurface 
    /// functionality within Microsoft.Silverlight.Testing.
    /// 
    /// The TestSurface is automatically cleared after each test scenario 
    /// completes, eliminating the need for many additional cleanup methods.
    /// </summary>
    public partial class TestPage : UserControl
    {
        /// <summary>
        /// Initializes the TestPage object.
        /// </summary>
        public TestPage()
        {
            InitializeComponent();
            Loaded += new RoutedEventHandler(TestPage_Loaded);
        }

        /// <summary>
        /// Sets the test panel instance.
        /// </summary>
        /// <param name="sender">The sending object.</param>
        /// <param name="e">The event arguments.</param>
        private void TestPage_Loaded(object sender, RoutedEventArgs e)
        {
            TestPanelManager.Instance.TestPage = this;
        }

        /// <summary>
        /// Gets the test surface, a dynamic Panel that removes its children 
        /// elements after each test completes.
        /// </summary>
        public Panel TestPanel
        {
            get { return TestLayoutRoot; }
        }
    }
}