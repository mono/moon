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
using System.Windows;

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
        public static int VisualDelayInMilliseconds = 100;

        /// <summary>
        /// Gets the test panel.
        /// </summary>
        [SuppressMessage("Microsoft.Performance", "CA1822:MarkMembersAsStatic", Justification = "Enables future scenarios.")]
        public Panel TestPanel
        {
            get { return TestPanelManager.Instance.TestPanel; }
        }
		public TestPage TestPage
		{
			get { return TestPanelManager.Instance.TestPage; }
		}
        protected void CreateAsyncTest(FrameworkElement element, params Action[] actions)
        {
            Assert.IsNotNull(element);
            actions = actions ?? new Action[] { };

            // Add the element to the test surface and wait until it's loaded
            bool isLoaded = false;
            element.Loaded += delegate { isLoaded = true; };
            TestPanel.Children.Add(element);
            EnqueueConditional(() => isLoaded);

            // Perform the test actions
            foreach (Action action in actions)
            {
                Action capturedAction = action;
                EnqueueCallback(() => capturedAction());
                EnqueueSleep(VisualDelayInMilliseconds);
            }

            // Remove the element and complete the test
            EnqueueCallback(() => TestPanel.Children.Remove(element));
            EnqueueTestComplete();
        }

		protected virtual void EnqueueWaitLoaded (FrameworkElement frameworkElement, string message)
		{
			bool loaded = false;
			frameworkElement.Loaded += (o, e) => {
				loaded = true;
			};
			EnqueueConditional (() => loaded, message);
		}
    }
}