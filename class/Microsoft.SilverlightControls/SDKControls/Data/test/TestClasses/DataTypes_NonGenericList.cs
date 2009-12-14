// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test
{
#if NonGenericList
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericList_0 : DataGridUnitTest_Unrestricted<DataTypes, NonGenericList_0<DataTypes>>
    {
    }

#endif

#if TestOne
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericList_1 : DataGridUnitTest_Unrestricted<DataTypes, NonGenericList_1<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericList_1_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypes, NonGenericList_1<DataTypes>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericList_25 : DataGridUnitTest_Unrestricted<DataTypes, NonGenericList_25<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericList_25_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypes, NonGenericList_25<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_NonGenericList_25_RequireGT1 : DataGridUnitTest_RequireGT1<DataTypes, NonGenericList_25<DataTypes>>
    {
    }

#endif

}
