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
    /// A special indicator attribute to enable better debugging using 
    /// Microsoft.Silverlight.Testing.  
    /// 
    /// As there is very little parameter information available for a test run, 
    /// this attribute singles out specific classes to execute when found.
    /// 
    /// This attribute is unique to this environment and not compatible with any 
    /// desktop unit test framework without using a shim if it is left in code.
    /// </summary>
    [AttributeUsage(AttributeTargets.Class | AttributeTargets.Method, AllowMultiple = false)]
    public sealed class ExclusiveAttribute : Attribute { }
}