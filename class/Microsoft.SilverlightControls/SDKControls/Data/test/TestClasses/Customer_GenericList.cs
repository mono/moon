// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test
{
#if Customer
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_0 : DataGridUnitTest_Unrestricted<Customer, GenericList_0<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_0_RequireGeneric : DataGridUnitTest_RequireGeneric<Customer, GenericList_0<Customer>>
    {
    }

#endif

#if TestOne

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_1 : DataGridUnitTest_Unrestricted<Customer, GenericList_1<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_1_RequireGT0 : DataGridUnitTest_RequireGT0<Customer, GenericList_1<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_1_RequireGeneric : DataGridUnitTest_RequireGeneric<Customer, GenericList_1<Customer>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_25 : DataGridUnitTest_Unrestricted<Customer, GenericList_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_25_RequireGT0 : DataGridUnitTest_RequireGT0<Customer, GenericList_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_25_RequireGT1 : DataGridUnitTest_RequireGT1<Customer, GenericList_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericList_25_RequireGeneric : DataGridUnitTest_RequireGeneric<Customer, GenericList_25<Customer>>
    {
    }

#endif

}
