// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Specialized;

namespace System.Windows.Controls.Data.Test.DataClassSources
{
    public class NonGenericListINCC_0<TDataClass>: NonGenericListINCC<TDataClass>
        where TDataClass: new()
    {
        public NonGenericListINCC_0()
            : base(0)
        {
        }
    }

    public class NonGenericListINCC_1<TDataClass> : NonGenericListINCC<TDataClass>
        where TDataClass : new()
    {
        public NonGenericListINCC_1()
            : base(1)
        {
        }
    }

    public class NonGenericListINCC_25<TDataClass> : NonGenericListINCC<TDataClass>
        where TDataClass : new()
    {
        public NonGenericListINCC_25()
            : base(25)
        {
        }
    }

    public abstract class NonGenericListINCC<TDataClass> : NonGenericList<TDataClass>, INotifyCollectionChanged
    where TDataClass : new()
    {
        protected NonGenericListINCC(int count)
            : base(count)
        {
        }

        public override int Add(object value)
        {
            int index = base.Add(value);

            this.OnCollectionChanged(NotifyCollectionChangedAction.Add, value, null, index);

            return index;
        }

        public override void Clear()
        {
            base.Clear();

            this.OnCollectionChanged(NotifyCollectionChangedAction.Reset, null, null, -1);
        }

        public override void Insert(int index, object value)
        {
            base.Insert(index, value);

            this.OnCollectionChanged(NotifyCollectionChangedAction.Add, value, null, index);
        }

        public override void Remove(object value)
        {
            int index = this.IndexOf(value);

            base.Remove(value);

            this.OnCollectionChanged(NotifyCollectionChangedAction.Remove, value, null, index);
        }

        public override void RemoveAt(int index)
        {
            object item = this[index];

            base.RemoveAt(index);

            this.OnCollectionChanged(NotifyCollectionChangedAction.Remove, item, null, index);
        }

        public override object this[int index]
        {
            get
            {
                return base[index];
            }
            set
            {
                object oldItem = this[index];

                base[index] = value;

                this.OnCollectionChanged(NotifyCollectionChangedAction.Replace, value, oldItem, index);
            }
        }

        #region INotifyCollectionChanged Members

        private void OnCollectionChanged(NotifyCollectionChangedAction action, object item, object oldItem, int index)
        {
            if (this.CollectionChanged != null)
            {
                NotifyCollectionChangedEventArgs args = null;

                switch (action)
                {
                    case NotifyCollectionChangedAction.Add:
                    case NotifyCollectionChangedAction.Remove:
                        args = new NotifyCollectionChangedEventArgs(action, item, index);
                        break;
                    case NotifyCollectionChangedAction.Replace:
                        args = new NotifyCollectionChangedEventArgs(action, item, oldItem, index);
                        break;
                    case NotifyCollectionChangedAction.Reset:
                        args = new NotifyCollectionChangedEventArgs(action);
                        break;
                }
                this.CollectionChanged(this, args);
            }
        }

        public event NotifyCollectionChangedEventHandler CollectionChanged;

        #endregion
    }
}
