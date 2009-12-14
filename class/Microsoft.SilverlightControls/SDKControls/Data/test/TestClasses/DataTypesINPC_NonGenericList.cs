// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test
{
#if NonGenericList
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericList_0 : DataGridUnitTest_Unrestricted<DataTypesINPC, NonGenericList_0<DataTypesINPC>>
    {
    }

#endif

#if TestOne
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericList_1 : DataGridUnitTest_Unrestricted<DataTypesINPC, NonGenericList_1<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericList_1_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypesINPC, NonGenericList_1<DataTypesINPC>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericList_25 : DataGridUnitTest_Unrestricted<DataTypesINPC, NonGenericList_25<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericList_25_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypesINPC, NonGenericList_25<DataTypesINPC>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypesINPC_NonGenericList_25_RequireGT1 : DataGridUnitTest_RequireGT1<DataTypesINPC, NonGenericList_25<DataTypesINPC>>
    {
    }

#endif
}
