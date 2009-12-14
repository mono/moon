// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Generic;
using System.Linq;

namespace System.Windows.Controls.Data.Test.DataClassSources
{
    public abstract class DataClassSource<TDataClass> : IEnumerable
        where TDataClass : new()
    {
        protected DataClassSource(int count)
        {
            this.Count = count;
        }

        protected int Count { get; private set; }

        private List<object> _items;

        protected List<object> Items
        {
            get
            {
                if (_items == null)
                {
                    _items = new List<object>(Generate());
                }

                return _items;
            }
        }

        protected virtual IEnumerable<object> Generate()
        {
            for (int i = 0; i < this.Count; i++)
            {
                yield return new TDataClass();
            }
        }

        #region IEnumerable Members

        public IEnumerator GetEnumerator()
        {
            return this.Items.GetEnumerator();
        }

        #endregion
    }

    public abstract class GenericDataClassSource<TDataClass> : DataClassSource<TDataClass>, IEnumerable<TDataClass>
        where TDataClass : new()
    {
        protected GenericDataClassSource(int count)
            : base(count)
        {
        }

        #region IEnumerable<TDataClass> Members

        public new IEnumerator<TDataClass> GetEnumerator()
        {
            return this.Items.Cast<TDataClass>().GetEnumerator();
        }

        #endregion
    }

    public static class Extensions
    {
        public static T First<T>(this IEnumerable seq)
        {
            return Enumerable.First(seq.Cast<T>());
        }

        public static object First(this IEnumerable seq)
        {
            return Enumerable.First(seq.Cast<object>());
        }

        public static object Last(this IEnumerable seq)
        {
            return Enumerable.Last(seq.Cast<object>());
        }

        public static int Count(this IEnumerable seq)
        {
            return Enumerable.Count(seq.Cast<object>());
        }

        public static IEnumerable Skip(this IEnumerable seq, int n)
        {
            IEnumerable result = Enumerable.Skip(seq.Cast<object>(), n);

            foreach (object item in result)
            {
                yield return item;
            }
        }
    }
}
