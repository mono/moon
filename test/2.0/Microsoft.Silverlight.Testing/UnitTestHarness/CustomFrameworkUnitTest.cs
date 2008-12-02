// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.ComponentModel;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting;
using Microsoft.Silverlight.Testing.UnitTesting.Harness;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// Base class for test cases that use special functionality of the 
    /// Microsoft.Silverlight.Testing unit test framework.
    /// 
    /// Tests that derive from CustomTest in most cases will not be source- or 
    /// functionality- compatible with the more advanced desktop Visual Studio 
    /// Test Team environment and harnesses.
    /// </summary>
    public abstract class CustomFrameworkUnitTest
    {
        /// <summary>
        /// Gets or sets a value indicating whether global unhandled exceptions 
        /// should be intercepted by the test harness.
        /// </summary>
        public bool InterceptUnhandledExceptions
        {
            get { return UnitTestHarness.InterceptAllExceptions; }
            set { UnitTestHarness.InterceptAllExceptions = value; }
        }

        /// <summary>
        /// Gets or sets the unit test harness instance.  Hidden from the VS 
        /// browser as test developers should not need to use this property.
        /// </summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public UnitTestHarness UnitTestHarness 
        { 
            get; 
            set;
        }

        /// <summary>
        /// Gets the current test task container.
        /// </summary>
        /// <returns>The current container for the test's tasks.</returns>
        [EditorBrowsable(EditorBrowsableState.Never)]
        protected CompositeWorkItem WorkItemContainer
        {
            get
            {
                if (UnitTestHarness.DispatcherStack.CurrentCompositeWorkItem != null && (UnitTestHarness.DispatcherStack.CurrentCompositeWorkItem as CompositeWorkItem != null))
                {
                    CompositeWorkItem disp = (CompositeWorkItem)UnitTestHarness.DispatcherStack.CurrentCompositeWorkItem;
                    return disp;
                }
                else
                {
                    // TODO: Better Exception message
                    // NOTE: Removed assert; previously if a test was already 
                    // completed, this would also throw an exception with 
                    // another check
                    throw new InvalidOperationException();
                }
            }
        }
        
        /// <summary>
        /// Process an exception using the test engine logic for 
        /// ExpectedExceptions and logging the exception if needed.
        /// </summary>
        /// <param name="ex">Exception object.</param>
        public virtual void HandleException(Exception ex)
        {
            WorkItemContainer.WorkItemException(ex);
        }
    }
}