// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Linq;

namespace System.Windows.Controls.Data.Test.DataClassSources
{
    public class GenericList_0<TDataClass>: GenericList<TDataClass>
        where TDataClass: new()
    {
        public GenericList_0()
            : base(0)
        {
        }
    }

    public class GenericList_1<TDataClass> : GenericList<TDataClass>
        where TDataClass : new()
    {
        public GenericList_1()
            : base(1)
        {
        }
    }

    public class GenericList_5<TDataClass> : GenericList<TDataClass>
        where TDataClass : new()
    {
        public GenericList_5()
            : base(5)
        {
        }
    }

    public class GenericList_25<TDataClass> : GenericList<TDataClass>
        where TDataClass : new()
    {
        public GenericList_25()
            : base(25)
        {
        }
    }

    public abstract class GenericList<TDataClass> : DataClassSource<TDataClass>, IList<TDataClass>
        where TDataClass : new()
    {
        protected GenericList(int count)
            : base(count)
        {
        }

        #region IList<TDataClass> Members

        public int IndexOf(TDataClass item)
        {
            return Items.IndexOf(item);
        }

        public void Insert(int index, TDataClass item)
        {
            Items.Insert(index, item);
        }

        public void RemoveAt(int index)
        {
            Items.RemoveAt(index);
        }

        public TDataClass this[int index]
        {
            get
            {
                return (TDataClass)Items[index];
            }
            set
            {
                Items[index] = value;
            }
        }

        #endregion

        #region ICollection<TDataClass> Members

        public void Add(TDataClass item)
        {
            Items.Add(item);
        }

        public void Clear()
        {
            Items.Clear();
        }

        public bool Contains(TDataClass item)
        {
            return Items.Contains(item);
        }

        public void CopyTo(TDataClass[] array, int arrayIndex)
        {
            Items.CopyTo(array.Cast<object>().ToArray(), arrayIndex);
        }

        public new int Count
        {
            get { return Items.Count; }
        }

        public bool IsReadOnly
        {
            get { return false; }
        }

        public bool Remove(TDataClass item)
        {
            return Items.Remove(item);
        }

        #endregion

        #region IEnumerable<TDataClass> Members

        public new IEnumerator<TDataClass> GetEnumerator()
        {
            return Items.Cast<TDataClass>().GetEnumerator();
        }

        #endregion
    }
}
