// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test
{
#if Customer
#if NonGenericList
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericList_0 : DataGridUnitTest_Unrestricted<Customer, NonGenericList_0<Customer>>
    {
    }

#endif

#if TestOne

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericList_1 : DataGridUnitTest_Unrestricted<Customer, NonGenericList_1<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericList_1_RequireGT0 : DataGridUnitTest_RequireGT0<Customer, NonGenericList_1<Customer>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericList_25 : DataGridUnitTest_Unrestricted<Customer, NonGenericList_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericList_25_RequireGT0 : DataGridUnitTest_RequireGT0<Customer, NonGenericList_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericList_25_RequireGT1 : DataGridUnitTest_RequireGT1<Customer, NonGenericList_25<Customer>>
    {
    }

#endif
#endif
}
