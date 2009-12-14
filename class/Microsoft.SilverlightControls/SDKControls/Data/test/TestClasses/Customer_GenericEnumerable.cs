// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test
{
#if Customer
#if GenericEnumerable
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_0 : DataGridUnitTest_Unrestricted<Customer, GenericEnumerable_0<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_0_RequireGeneric : DataGridUnitTest_RequireGeneric<Customer, GenericEnumerable_0<Customer>>
    {
    }

#endif

#if TestOne

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_1 : DataGridUnitTest_Unrestricted<Customer, GenericEnumerable_1<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_1_RequireGT0 : DataGridUnitTest_RequireGT0<Customer, GenericEnumerable_1<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_1_RequireGeneric : DataGridUnitTest_RequireGeneric<Customer, GenericEnumerable_1<Customer>>
    {
    }

#endif

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_25 : DataGridUnitTest_Unrestricted<Customer, GenericEnumerable_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_25_RequireGT0 : DataGridUnitTest_RequireGT0<Customer, GenericEnumerable_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_25_RequireGT1 : DataGridUnitTest_RequireGT1<Customer, GenericEnumerable_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_GenericEnumerable_25_RequireGeneric : DataGridUnitTest_RequireGeneric<Customer, GenericEnumerable_25<Customer>>
    {
    }

#endif
#endif

}
