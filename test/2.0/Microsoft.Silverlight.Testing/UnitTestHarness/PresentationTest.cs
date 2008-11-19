// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.Windows.Controls;
using Microsoft.Silverlight.Testing.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting.UI;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// Implementation of useful properties and features for presentation 
    /// platform tests (Silverlight and WPF).
    /// 
    /// Tests using this functionality will not be compatible with the full 
    /// desktop framework's Visual Studio Team Test environment.
    /// </summary>
    public abstract class PresentationTest : WorkItemTest
    {
        /// <summary>
        /// Gets the test panel.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic", Justification = "Enables future scenarios.")]
        public Panel TestPanel
        {
            get { return TestPanelManager.Instance.TestPanel; }
        }
    }
}