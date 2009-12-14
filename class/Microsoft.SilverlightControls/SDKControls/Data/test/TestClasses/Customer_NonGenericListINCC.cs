// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test
{
#if Customer
#if TestZero

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericListINCC_0 : DataGridUnitTest_Unrestricted<Customer, NonGenericListINCC_0<Customer>>
    {
    }

#endif

#if TestOne

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericListINCC_1 : DataGridUnitTest_Unrestricted<Customer, NonGenericListINCC_1<Customer>>
    {
    }
    
    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericListINCC_1_RequireGT0 : DataGridUnitTest_RequireGT0<Customer, NonGenericListINCC_1<Customer>>
    {
    }

#endif
    
    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericListINCC_25 : DataGridUnitTest_Unrestricted<Customer, NonGenericListINCC_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericListINCC_25_RequireGT0 : DataGridUnitTest_RequireGT0<Customer, NonGenericListINCC_25<Customer>>
    {
    }

    [TestClass]
    public partial class DataGridUnitTest_Customer_NonGenericListINCC_25_RequireGT1 : DataGridUnitTest_RequireGT1<Customer, NonGenericListINCC_25<Customer>>
    {
    }

#endif

}
