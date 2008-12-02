// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Windows.Threading;
using Microsoft.Silverlight.Testing.Harness;

namespace Microsoft.Silverlight.Testing.UI
{
    /// <summary>
    /// A type which handles preparing the underlying dispatcher or timer from 
    /// which the test work items execute.
    /// </summary>
    public class WebBrowserTick : RunDispatcher
    {
        /// <summary>
        /// Millisecond interval to use for the interval between DispatcherTimer
        /// ticks.
        /// </summary>
        private const int DefaultTickInterval = 1;

        /// <summary>
        /// Dispatcher timer.
        /// </summary>
        private DispatcherTimer _timer;
        
        /// <summary>
        /// Creates a new run method manager using the default value for the 
        /// timer's millisecond interval.
        /// </summary>
        /// <param name="runNextStep">
        /// Conditional delegate which returns true as long as there is 
        /// additional work.
        /// </param>
        public WebBrowserTick(Func<bool> runNextStep) : this(runNextStep, DefaultTickInterval) { }

        /// <summary>
        /// Sets up a new run method manager.
        /// </summary>
        /// <param name="runNextStep">
        /// Conditional delegate which returns true as long as there is 
        /// additional work.
        /// </param>
        /// <param name="millisecondInterval">Milliseconds between ticks, at a 
        /// minimum.</param>
        public WebBrowserTick(Func<bool> runNextStep, int millisecondInterval) : base(runNextStep)
        {
            _timer = new DispatcherTimer();
            _timer.Interval = new TimeSpan(0, 0, 0, 0, millisecondInterval);
        }

        /// <summary>
        /// Begin the execution process by hooking up the underlying 
        /// DispatcherTimer to call into the test framework regularly and 
        /// perform test work items.
        /// </summary>
        public override void Run()
        {
            _timer.Tick += Timer_Tick;
            _timer.Start();
        }

        /// <summary>
        /// Call into the underlying work item queue, if the method manager is 
        /// still set to run.
        /// </summary>
        /// <param name="sender">Sending object.</param>
        /// <param name="e">Event arguments.</param>
        private void Timer_Tick(object sender, EventArgs e)
        {
            _timer.Stop();
            if (RunNextStep())
            {
                _timer.Start();
            }
            else
            {
                OnComplete();
            }
        }
    }
}