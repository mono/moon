// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Controls.Data.Test.DataClasses;
using System.Windows.Controls.Data.Test.DataClassSources;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace System.Windows.Controls.Data.Test
{
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericEnumerable_0 : DataGridUnitTest_Unrestricted<DataTypes, NonGenericEnumerable_0<DataTypes>>
    {
    }

#endif

#if TestOne
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericEnumerable_1 : DataGridUnitTest_Unrestricted<DataTypes, NonGenericEnumerable_1<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericEnumerable_1_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypes, NonGenericEnumerable_1<DataTypes>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericEnumerable_25 : DataGridUnitTest_Unrestricted<DataTypes, NonGenericEnumerable_25<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericEnumerable_25_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypes, NonGenericEnumerable_25<DataTypes>>
    {
        [Ignore]
        public override void EditUnderlyingData()
        {
        }
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericEnumerable_25_RequireGT1 : DataGridUnitTest_RequireGT1<DataTypes, NonGenericEnumerable_25<DataTypes>>
    {
    }
}
