// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test
{
#if GenericEnumerable
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_0 : DataGridUnitTest_Unrestricted<DataTypesINPC, GenericEnumerable_0<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_0_RequireGeneric : DataGridUnitTest_RequireGeneric<DataTypesINPC, GenericEnumerable_0<DataTypesINPC>>
    {
    }

#endif

#if TestOne
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_1 : DataGridUnitTest_Unrestricted<DataTypesINPC, GenericEnumerable_1<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_1_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypesINPC, GenericEnumerable_1<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_1_RequireGeneric : DataGridUnitTest_RequireGeneric<DataTypesINPC, GenericEnumerable_1<DataTypesINPC>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_25 : DataGridUnitTest_Unrestricted<DataTypesINPC, GenericEnumerable_25<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_25_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypesINPC, GenericEnumerable_25<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_25_RequireGT1 : DataGridUnitTest_RequireGT1<DataTypesINPC, GenericEnumerable_25<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_GenericEnumerable_25_RequireGeneric : DataGridUnitTest_RequireGeneric<DataTypesINPC, GenericEnumerable_25<DataTypesINPC>>
    {
    }

#endif

}
