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
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericEnumerable_0 : DataGridUnitTest_Unrestricted<DataTypesINPC, NonGenericEnumerable_0<DataTypesINPC>>
    {
    }

#endif

#if TestOne
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericEnumerable_1 : DataGridUnitTest_Unrestricted<DataTypesINPC, NonGenericEnumerable_1<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericEnumerable_1_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypesINPC, NonGenericEnumerable_1<DataTypesINPC>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericEnumerable_25 : DataGridUnitTest_Unrestricted<DataTypesINPC, NonGenericEnumerable_25<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericEnumerable_25_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypesINPC, NonGenericEnumerable_25<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericEnumerable_25_RequireGT1 : DataGridUnitTest_RequireGT1<DataTypesINPC, NonGenericEnumerable_25<DataTypesINPC>>
    {
    }
}
