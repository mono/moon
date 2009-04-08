// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.Silverlight.Testing.UnitTesting
{
    /// <summary>
    /// A test work item that is complete once the condition is asserted.
    /// </summary>
    public class ConditionalWorkItem : WorkItem
    {
        DateTime? start;
        /// <summary>
        /// The conditional delegate.
        /// </summary>
        private Func<bool> _delegate;
        private string _message;
        private TimeSpan timeout;

        /// <summary>
        /// Construct a new conditional work item.
        /// </summary>
        /// <param name="conditionalMethod">Conditional delegate.</param>
        public ConditionalWorkItem(Func<bool> conditionalMethod) : this(conditionalMethod, TimeSpan.FromSeconds(10), "")
        {

        }
        
        public ConditionalWorkItem(Func<bool> conditionalMethod, TimeSpan timeout) : this(conditionalMethod, timeout, "")
        {

        }
        
        public ConditionalWorkItem(Func<bool> conditionalMethod, string message) : this(conditionalMethod, TimeSpan.FromSeconds(5), message)
        {

        }

        public ConditionalWorkItem (Func<bool> conditionalMethod, TimeSpan timeout, string message)
            : base ()
        {
            _delegate = conditionalMethod;
            _message = message ?? "";
            this.timeout = timeout;
        }

        /// <summary>
        /// Invoke the condition, will continue invoking until 
        /// the condition is false.
        /// </summary>
        /// <returns>Completes the invocation once the condition is true.</returns>
        public override bool Invoke()
        {
            if (_delegate () == true)
            {
                this.WorkItemComplete ();
            }
            else
            {
                if (!start.HasValue)
                    start = DateTime.Now;

                if ((DateTime.Now - start) > timeout) 
                    Assert.Fail ("Timeout exceeded on conditional wait. " + _message);
            }
            return base.Invoke();
        }
    }
}