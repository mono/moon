// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Linq;

namespace System.Windows.Controls.Data.Test.DataClassSources
{
    public class NonGenericList_0<TDataClass>: NonGenericList<TDataClass>
        where TDataClass: new()
    {
        public NonGenericList_0()
            : base(0)
        {
        }
    }

    public class NonGenericList_1<TDataClass> : NonGenericList<TDataClass>
        where TDataClass : new()
    {
        public NonGenericList_1()
            : base(1)
        {
        }
    }

    public class NonGenericList_25<TDataClass> : NonGenericList<TDataClass>
        where TDataClass : new()
    {
        public NonGenericList_25()
            : base(25)
        {
        }
    }

    public abstract class NonGenericList<TDataClass> : DataClassSource<TDataClass>, IList
    where TDataClass : new()
    {
        protected NonGenericList(int count)
            : base(count)
        {
        }

        #region IList Members

        public virtual int Add(object value)
        {
            this.Items.Add(value);

            return this.Items.IndexOf(value);
        }

        public virtual void Clear()
        {
            this.Items.Clear();
        }

        public bool Contains(object value)
        {
            return this.Items.Contains(value);
        }

        public int IndexOf(object value)
        {
            return this.Items.IndexOf(value);
        }

        public virtual void Insert(int index, object value)
        {
            this.Items.Insert(index, value);
        }

        public bool IsFixedSize
        {
            get { return false; }
        }

        public bool IsReadOnly
        {
            get { return false; }
        }

        public virtual void Remove(object value)
        {
            this.Items.Remove(value);
        }

        public virtual void RemoveAt(int index)
        {
            this.Items.RemoveAt(index);
        }

        public virtual object this[int index]
        {
            get
            {
                return this.Items[index];
            }
            set
            {
                this.Items[index] = value;
            }
        }

        #endregion

        #region ICollection Members

        public void CopyTo(Array array, int index)
        {
            this.Items.CopyTo(array.Cast<object>().ToArray(), index);
        }

        public new int Count
        {
            get { return this.Items.Count; }
        }

        public bool IsSynchronized
        {
            get { return false; }
        }

        private object syncRoot = new object();

        public object SyncRoot
        {
            get { return this.syncRoot; }
        }

        #endregion

        #region IEnumerable Members

        public new IEnumerator GetEnumerator()
        {
            return this.Items.GetEnumerator();
        }

        #endregion
    }
}
