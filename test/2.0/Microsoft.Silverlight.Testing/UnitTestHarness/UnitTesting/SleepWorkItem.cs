// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;

namespace Microsoft.Silverlight.Testing.UnitTesting
{
    /// <summary>
    /// Test work item type that does not complete until the sleep time has 
    /// elapsed.  This is NOT a blocking Sleep.
    /// </summary>
    public class SleepWorkItem : WorkItem
    {
        /// <summary>
        /// The amount of milliseconds to wait for.
        /// </summary>
        private int _sleepTime;

        /// <summary>
        /// The DateTime that marks the point in time the task is complete.
        /// </summary>
        private DateTime? _expires;

        /// <summary>
        /// Create a new Sleep work item, including the number of 
        /// milliseconds to wait until continuing.
        /// </summary>
        /// <param name="sleepMilliseconds">Milliseconds to wait.</param>
        public SleepWorkItem(int sleepMilliseconds)
        {
            _sleepTime = sleepMilliseconds;
            _expires = null;
        }

        /// <summary>
        /// On the first time, will calculate the final DateTime.  Otherwise, 
        /// null operation (returns) until that time.
        /// </summary>
        /// <returns>Returns a value indicating whether there is more work to be
        /// done.</returns>
        public override bool Invoke()
        {
            if (_expires == null)
            {
                _expires = DateTime.Now.AddMilliseconds(_sleepTime);
            }
            if (DateTime.Now > _expires) 
            {
                WorkItemComplete();
            }
            return base.Invoke();
        }
    }
}