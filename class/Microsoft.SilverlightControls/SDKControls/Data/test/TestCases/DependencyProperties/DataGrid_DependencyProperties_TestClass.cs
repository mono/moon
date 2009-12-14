// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
    [TestClass]
    /// <summary>
    /// These tests don't depend on entity types
    /// </summary>
    public partial class DataGrid_DependencyProperties_TestClass : SilverlightTest
    {
        int counter = 0;

        void control_SelectionChanged(object sender, EventArgs e)
        {
            counter++;
        }
    }
}
