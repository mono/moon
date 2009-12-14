// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using Microsoft.Silverlight.Testing;

namespace System.Windows.Controls.Test
{
    public static class SilverlightTestExtensions
    {
        /// <summary>
        /// Enqueue to step allow (for example) event handlers a chance to execute.
        /// </summary>
        /// <param name="test"></param>
        public static void EnqueueYieldThread(this SilverlightTest test)
        {
            test.EnqueueSleep(0);
        }
    }
}
