// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test.DataClassSources
{
    public class NonGenericEnumerable_0<TDataClass>: DataClassSource<TDataClass>
        where TDataClass: new()
    {
        public NonGenericEnumerable_0()
            : base(0)
        {
        }
    }

    public class NonGenericEnumerable_1<TDataClass> : DataClassSource<TDataClass>
        where TDataClass : new()
    {
        public NonGenericEnumerable_1()
            : base(1)
        {
        }
    }

    public class NonGenericEnumerable_25<TDataClass> : DataClassSource<TDataClass>
        where TDataClass : new()
    {
        public NonGenericEnumerable_25()
            : base(25)
        {
        }
    }
}
