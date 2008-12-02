// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Diagnostics;

namespace Microsoft.Silverlight.Testing.Harness
{
    /// <summary>
    /// The most verbose log provider, this calls .ToString() on each and every 
    /// LogMessage that it processes.  The output will appear in an attached 
    /// debugger's "Output" window.
    /// </summary>
    public class DebugOutputProvider : LogProvider
    {
        /// <summary>
        /// Gets or sets a value indicating whether any TestResult failures will 
        /// be reported, regardless of whether the TestResult type is being 
        /// monitored for debug output.
        /// </summary>
        public bool ShowAllFailures { get; set; }

        /// <summary>
        /// Gets or sets a value indicating whether all messages appear in the 
        /// debug output.
        /// </summary>
        public bool ShowEverything { get; set; }

        /// <summary>
        /// Initializes a new instance of the DebugOutputProvider class.
        /// </summary>
        public DebugOutputProvider()
        { 
        }
        
        /// <summary>
        /// Display a LogMessage in the debug output window.
        /// </summary>
        /// <param name="logMessage">Message object.</param>
        public override void Process(LogMessage logMessage)
        {
            if (logMessage == null)
            {
                throw new ArgumentNullException("logMessage");
            }

            if ((logMessage.MessageType != LogMessageType.Debug) && !ShowEverything)
            {
                if (!ShowAllFailures)
                {
                    return;
                }
                
                if (logMessage.HasDecorator(LogDecorator.TestOutcome))
                {
                    TestOutcome outcome = (TestOutcome)logMessage[LogDecorator.TestOutcome];
                    if (outcome == TestOutcome.Passed)
                    {
                        return;
                    }
                }
            }

            Debug.WriteLine(logMessage.ToString());
        }
    }
}