// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.


namespace System.Windows.Controls.Data.Test.DataClassSources
{
    public class GenericEnumerable_0<TDataClass>: GenericDataClassSource<TDataClass>
        where TDataClass: new()
    {
        public GenericEnumerable_0()
            : base(0)
        {
        }
    }

    public class GenericEnumerable_1<TDataClass> : GenericDataClassSource<TDataClass>
    where TDataClass : new()
    {
        public GenericEnumerable_1()
            : base(1)
        {
        }
    }

    public class GenericEnumerable_25<TDataClass> : GenericDataClassSource<TDataClass>
    where TDataClass : new()
    {
        public GenericEnumerable_25()
            : base(25)
        {
        }
    }
}
