// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test
{
#if GenericEnumerable
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_0 : DataGridUnitTest_Unrestricted<DataTypes, GenericEnumerable_0<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_0_RequireGeneric : DataGridUnitTest_RequireGeneric<DataTypes, GenericEnumerable_0<DataTypes>>
    {
    }

#endif

#if TestOne
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_1 : DataGridUnitTest_Unrestricted<DataTypes, GenericEnumerable_1<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_1_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypes, GenericEnumerable_1<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_1_RequireGeneric : DataGridUnitTest_RequireGeneric<DataTypes, GenericEnumerable_1<DataTypes>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_25 : DataGridUnitTest_Unrestricted<DataTypes, GenericEnumerable_25<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_25_RequireGT0 : DataGridUnitTest_RequireGT0<DataTypes, GenericEnumerable_25<DataTypes>>
    {
        [Ignore]
        public override void EditUnderlyingData()
        {
        }
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_25_RequireGT1 : DataGridUnitTest_RequireGT1<DataTypes, GenericEnumerable_25<DataTypes>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_DataTypes_GenericEnumerable_25_RequireGeneric : DataGridUnitTest_RequireGeneric<DataTypes, GenericEnumerable_25<DataTypes>>
    {
    }

#endif

}
