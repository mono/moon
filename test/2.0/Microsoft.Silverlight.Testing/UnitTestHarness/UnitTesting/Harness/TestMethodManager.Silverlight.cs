// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System;
using System.Reflection;
using Microsoft.Silverlight.Testing.Harness;
using Microsoft.Silverlight.Testing.UnitTesting.Metadata;

#if SILVERLIGHT
using System.Windows.Browser;
using Microsoft.Silverlight.Testing.UnitTesting.UI;
#endif

namespace Microsoft.Silverlight.Testing.UnitTesting.Harness
{
    /// <summary>
    /// Manager for planning, processing, and reporting the result of a single 
    /// test method for a unit test provider.
    /// </summary>
    public partial class TestMethodManager : UnitTestCompositeWorkItem
    {
        /// <summary>
        /// Optional code run at the end of the FirstInvoke method.
        /// </summary>
        private void FirstInvokeOptional()
        {
#if SILVERLIGHT
            Enqueue(delegate
            {
                TestHtmlElementManager.Instance.ClearDirtyScratchElement();
                TestPanelManager.Instance.ClearUsedChildren();
            });
#endif
        }
    }
}