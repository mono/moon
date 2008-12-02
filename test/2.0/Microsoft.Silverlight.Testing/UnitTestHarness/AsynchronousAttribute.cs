// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Collections.Generic;
using System.Text;

namespace Microsoft.Silverlight.Testing
{
    /// <summary>
    /// Mark the test method as one which expects asynchronous execution. 
    /// It is important to call TestComplete() once it is ready or your 
    /// test will never continue/timeout.
    /// </summary>
    [AttributeUsage(AttributeTargets.Method, AllowMultiple = false)]
    public sealed class AsynchronousAttribute : Attribute { }
}