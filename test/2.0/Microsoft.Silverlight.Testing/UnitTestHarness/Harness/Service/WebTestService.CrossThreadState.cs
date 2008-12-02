// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Globalization;
using System.Net;
using System.Text;
using System.IO;
using System.Windows.Browser;
using System.Threading;

namespace Microsoft.Silverlight.Testing.Harness.Service
{
    /// <summary>
    /// Provides out-of-process access to operating system functions and other 
    /// services such as visual verification, if present.
    /// </summary>
    public partial class WebTestService
    {
        /// <summary>
        /// A simple type to store the state information for the cross-thread 
        /// callback.
        /// </summary>
        private class CrossThreadState
        {
            /// <summary>
            /// Initializes a new TemporaryStateObject object.
            /// </summary>
            /// <param name="callback">The callback action.</param>
            /// <param name="result">The result object.</param>
            public CrossThreadState(Action<ServiceResult> callback, WebServiceResult result)
            {
                Callback = callback;
                Result = result;
            }

            /// <summary>
            /// Gets the callback action.
            /// </summary>
            public Action<ServiceResult> Callback { get; private set; }

            /// <summary>
            /// Gets the result object.
            /// </summary>
            public WebServiceResult Result { get; private set; }
        }
    }
}