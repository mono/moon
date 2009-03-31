// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics.CodeAnalysis;
using System.IO;
using System.Net;
using System.Reflection;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Controls;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.Silverlight.Testing.UnitTesting.UI
{
    /// <summary>
    /// A manager for the underlying TestSurface Panel.
    /// </summary>
    public class TestPanelManager
    {
        /// <summary>
        /// Gets the single instance of the test surface manager.
        /// </summary>
        public static TestPanelManager Instance
        {
            get { return _instance; }
        }

        /// <summary>
        /// Instance of the manager.
        /// </summary>
        private static TestPanelManager _instance = new TestPanelManager();

        /// <summary>
        /// Private constructor.
        /// </summary>
        private TestPanelManager()
        {
        }

        /// <summary>
        /// The test page object.
        /// </summary>
        private TestPage _testPage;

        /// <summary>
        /// A value indicating whether the panel is dirty.
        /// </summary>
        private bool _dirty;

        /// <summary>
        /// Gets or sets the Reference to the TestPage user control.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode", Justification = "Provided for future framework use.")]
        internal TestPage TestPage
        {
            get { return _testPage; }
            set { _testPage = value; ClearChildren (); }
        }

        /// <summary>
        /// Gets the TestSurface Panel, and tracks the use for the 
        /// current test method.  When the test completes, the panel children 
        /// will be cleared automatically.
        /// </summary>
        public Panel TestPanel
        {
            get
            {
                _dirty = true;
                return (Panel)_testPage.TestPanel.Children[0];
            }
        }

        /// <summary>
        /// Remove the children from the test surface, if it has 
        /// been used.
        /// </summary>
        public void ClearUsedChildren()
        {
            if (_dirty)
            {
                ClearChildren();
            }
        }

        /// <summary>
        /// Remove the children from the test surface.
        /// </summary>
        public void ClearChildren()
        {
            if (_testPage != null && _testPage.TestPanel != null)
            {
                _testPage.TestPanel.Children.Clear ();
                _testPage.TestPanel.Children.Add (new Grid ());
            }

            _dirty = false;
        }
    }
}