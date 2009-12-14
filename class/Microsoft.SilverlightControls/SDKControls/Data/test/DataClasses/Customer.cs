// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections;
using System.Collections.Generic;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Windows.Media;

namespace System.Windows.Controls.Data.Test.DataClasses
{
    public class Customer : INotifyPropertyChanged, IEditableObject
    {
        struct CustomerData
        {
            internal string _firstName;
            internal string _lastName;
            internal Color _preferredColor;
            internal int _rating;
            internal DateTime _renewalDate;
            internal string _fullAddress;
            internal decimal _yearlyFees;
            internal bool _isRegistered;
            internal bool? _isValid;
        }

        #region Data

        private CustomerData _backupData, _data;
        private bool _editing, _delayPropertyChangeNotifications;
        private static Random random = new Random();

        #endregion Data

        public Customer()
        {
            this._delayPropertyChangeNotifications = true;
            this._data = new CustomerData();

            this._data._firstName = GetFakeString(random);
            this._data._lastName = GetFakeString(random);
            this._data._preferredColor = GetFakeColor(random);
            this._data._rating = random.Next(5);
            this._data._fullAddress = GetFakeString(random);
            this._data._yearlyFees = (decimal)(random.Next(100, 10000) / 100.0);
            this._data._isRegistered = GetFakeBoolean(random);
            this._data._isValid = GetFakeNullableBoolean(random);
            this._data._renewalDate = DateTime.Now.AddDays(-random.Next(364));
        }

        public Customer(string firstName, string lastName, Color preferredColor, int rating, DateTime renewalDate, string fullAddress, decimal yearlyFees, bool isRegistered, bool? isValid) : this()
        {
            this._delayPropertyChangeNotifications = true;
            this._data = new CustomerData();

            this._data._firstName = firstName;
            this._data._lastName = lastName;
            this._data._preferredColor = preferredColor;
            this._data._rating = rating;
            this._data._fullAddress = fullAddress;
            this._data._yearlyFees = yearlyFees;
            this._data._isRegistered = isRegistered;
            this._data._isValid = isValid;
            this._data._renewalDate = renewalDate;
        }

        #region INotifyPropertyChanged Members

        public event PropertyChangedEventHandler PropertyChanged;

        #endregion INotifyPropertyChanged Members

        #region IEditableObject Members

        public void BeginEdit()
        {
            if (!this._editing)
            {
                this._backupData = this._data;
                this._editing = true;
            }
        }

        public void CancelEdit()
        {
            if (this._editing)
            {
                if (!this._delayPropertyChangeNotifications)
                {
                    if (string.Compare(this._data._firstName, this._backupData._firstName, StringComparison.CurrentCulture) != 0)
                    {
                        RaisePropertyChanged("FirstName");
                    }
                    if (string.Compare(this._data._lastName, this._backupData._lastName, StringComparison.CurrentCulture) != 0)
                    {
                        RaisePropertyChanged("LastName");
                    }
                    if (this._data._rating != this._backupData._rating)
                    {
                        RaisePropertyChanged("Rating");
                    }
                    if (this._data._renewalDate != this._backupData._renewalDate)
                    {
                        RaisePropertyChanged("RenewalDate");
                    }
                    if (string.Compare(this._data._fullAddress, this._backupData._fullAddress, StringComparison.CurrentCulture) != 0)
                    {
                        RaisePropertyChanged("FullAddress");
                    }
                    if (this._data._yearlyFees != this._backupData._yearlyFees)
                    {
                        RaisePropertyChanged("YearlyFees");
                    }
                    if (this._data._isRegistered != this._backupData._isRegistered)
                    {
                        RaisePropertyChanged("IsRegistered");
                    }
                    if (this._data._isValid != this._backupData._isValid)
                    {
                        RaisePropertyChanged("IsValid");
                    }
                }
                this._data = this._backupData;
                this._editing = false;
            }
        }

        public void EndEdit()
        {
            if (this._editing)
            {
                if (this._delayPropertyChangeNotifications)
                {
                    if (string.Compare(this._data._firstName, this._backupData._firstName, StringComparison.CurrentCulture) != 0)
                    {
                        RaisePropertyChanged("FirstName");
                    }
                    if (string.Compare(this._data._lastName, this._backupData._lastName, StringComparison.CurrentCulture) != 0)
                    {
                        RaisePropertyChanged("LastName");
                    }
                    if (this._data._rating != this._backupData._rating)
                    {
                        RaisePropertyChanged("Rating");
                    }
                    if (this._data._renewalDate != this._backupData._renewalDate)
                    {
                        RaisePropertyChanged("RenewalDate");
                    }
                    if (string.Compare(this._data._fullAddress, this._backupData._fullAddress, StringComparison.CurrentCulture) != 0)
                    {
                        RaisePropertyChanged("FullAddress");
                    }
                    if (this._data._yearlyFees != this._backupData._yearlyFees)
                    {
                        RaisePropertyChanged("YearlyFees");
                    }
                    if (this._data._isRegistered != this._backupData._isRegistered)
                    {
                        RaisePropertyChanged("IsRegistered");
                    }
                    if (this._data._isValid != this._backupData._isValid)
                    {
                        RaisePropertyChanged("IsValid");
                    }
                }
                this._backupData = new CustomerData();
                this._editing = false;
            }
        }

        #endregion IEditableObject Members

        #region Public Properties
        
        public string FirstName
        {
            get
            {
                return this._data._firstName;
            }
            set 
            {
                if (this._data._firstName != value)
                {
                    this._data._firstName = value;
                    if (!this._delayPropertyChangeNotifications || !this._editing)
                    {
                        RaisePropertyChanged("FirstName");
                    }
                }
            }
        }

        public string FullAddress
        {
            get
            {
                return this._data._fullAddress;
            }
            set
            {
                if (this._data._fullAddress != value)
                {
                    this._data._fullAddress = value;
                    if (!this._delayPropertyChangeNotifications || !this._editing)
                    {
                        RaisePropertyChanged("FullAddress");
                    }
                }
            }
        }

        public bool IsRegistered
        {
            get
            {
                return this._data._isRegistered;
            }
            set
            {
                if (this._data._isRegistered != value)
                {
                    this._data._isRegistered = value;
                    if (!this._delayPropertyChangeNotifications || !this._editing)
                    {
                        RaisePropertyChanged("IsRegistered");
                    }
                }
            }
        }

        public bool? IsValid
        {
            get
            {
                return this._data._isValid;
            }
            set
            {
                if (this._data._isValid != value)
                {
                    this._data._isValid = value;
                    if (!this._delayPropertyChangeNotifications || !this._editing)
                    {
                        RaisePropertyChanged("IsValid");
                    }
                }
            }
        }

        public string LastName
        {
            get
            {
                return this._data._lastName;
            }
            set
            {
                if (this._data._lastName != value)
                {
                    this._data._lastName = value;
                    if (!this._delayPropertyChangeNotifications || !this._editing)
                    {
                        RaisePropertyChanged("LastName");
                    }
                }
            }
        }

        [PropertyTestExpectedResults(TestId = "AutogeneratedColumns", IsReadOnly = true)]
        public Color PreferredColor
        {
            get
            {
                return this._data._preferredColor;
            }
        }

        public int Rating
        {
            get
            {
                return this._data._rating;
            }
            set
            {
                if (this._data._rating != value)
                {
                    this._data._rating = value;
                    if (!this._delayPropertyChangeNotifications || !this._editing)
                    {
                        RaisePropertyChanged("Rating");
                    }
                }
            }
        }

        public DateTime RenewalDate
        {
            get
            {
                return this._data._renewalDate;
            }
            set
            {
                if (this._data._renewalDate != value)
                {
                    this._data._renewalDate = value;
                    if (!this._delayPropertyChangeNotifications || !this._editing)
                    {
                        RaisePropertyChanged("RenewalDate");
                    }
                }
            }
        }

        public decimal YearlyFees
        {
            get
            {
                return this._data._yearlyFees;
            }
            set
            {
                if (this._data._yearlyFees != value)
                {
                    this._data._yearlyFees = value;
                    if (!this._delayPropertyChangeNotifications || !this._editing)
                    {
                        RaisePropertyChanged("YearlyFees");
                    }
                }
            }
        }

        [PropertyTestExpectedResults(TestId = "AutogeneratedColumns", IsReadOnly = true)]
        public List<char> Chars
        {
            get
            {
                if (this.FirstName != null)
                {
                    return this.FirstName.ToCharArray().ToList();
                }
                else
                {
                    return new List<char>();
                }
            }
        }
        #endregion Public Properties

        #region Private Methods

        private static string GetFakeString(Random random)
        {
            StringBuilder stringBuilder = new StringBuilder();
            for (int i = 0; i < 8; i++)
            {
                stringBuilder.Append((char)random.Next(65, 90));
            }
            return stringBuilder.ToString();
        }

        private static bool GetFakeBoolean(Random random)
        {
            return random.Next(0, 2000) % 2 == 0;
        }

        private static bool? GetFakeNullableBoolean(Random random)
        {
            return random.Next(0, 3000) % 3 == 0;
        }

        private static Color GetFakeColor(Random random)
        {
            switch (random.Next(0, 9))
            {
                case 0:
                    return Colors.Blue;
                case 1:
                    return Colors.Brown;
                case 2:
                    return Colors.Cyan;
                case 3:
                    return Colors.Gray;
                case 4:
                    return Colors.Green;
                case 5:
                    return Colors.Magenta;
                case 6:
                    return Colors.Orange;
                case 7:
                    return Colors.Purple;
                case 8:
                    return Colors.Red;
                default:
                    return Colors.Yellow;
            }
        }

        private void RaisePropertyChanged(string propertyName)
        {
            if (this.PropertyChanged != null)
            {
                PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }
        }

        #endregion Private Methods
    }

    public class CustomerList : IList<Customer>, IList, INotifyCollectionChanged
    {
        private List<Customer> _customers;

        public event NotifyCollectionChangedEventHandler CollectionChanged;

        public CustomerList()
        {
            this._customers = new List<Customer>();

            // Fake Data
            Add(new Customer("Maria", "Anders", Colors.Orange, 1, new DateTime(2006, 5, 5), "Obere Str. 57, Berlin 12209, Germany", (decimal)12.25, false, null));
            Add(new Customer("Frédérique", "Citeaux", Colors.Purple, 5, DateTime.Now, "24, place Kléber, 67000 Strasbourg, France", (decimal)15.55, true, null));
            Add(new Customer("Maurizio", "Moroni", Colors.Yellow, 1, DateTime.Now, "Strada Provinciale 124, 42100 Reggio Emilia, Italy", (decimal)11.5, true, false));
            Add(new Customer("Georg", "Pipps", Colors.Green, 1, DateTime.Now, "Geislweg 14, Salzburg 5020, Austria", (decimal)9.99, false, true));
        }

        public CustomerList(int numberOfCustomers)
        {
            this._customers = new List<Customer>();

            for (int i = 0; i < numberOfCustomers; i++)
            {
                Add(new Customer());
                System.Threading.Thread.Sleep(25);
            }
        }

        #region ICollection Members

        public void CopyTo(System.Array array, int index)
        {
            throw new NotImplementedException();
        }

        public bool IsSynchronized
        {
            get
            {
                return false;
            }
        }

        public object SyncRoot
        {
            get
            {
                return this;
            }
        }

        #endregion

        #region IList Members

        public bool IsFixedSize
        {
            get
            {
                return false;
            }
        }

        public bool IsReadOnly
        {
            get
            {
                return false;
            }
        }

        object IList.this[int index]
        {
            get
            {
                return this._customers[index];
            }
            set
            {
                Customer customer = value as Customer;
                if (customer == null)
                {
                    throw new ArgumentException();
                }
                this._customers[index] = customer;
            }
        }

        public int Add(object value)
        {
            Customer customer = value as Customer;
            if (customer == null)
            {
                throw new ArgumentException("value");
            }
            Add(customer);
            return this.Count - 1;
        }

        public bool Contains(object value)
        {
            if (value == null)
            {
                throw new ArgumentNullException("value");
            }
            return Contains(value as Customer);
        }

        public int IndexOf(object value)
        {
            if (value == null)
            {
                throw new ArgumentNullException("value");
            }
            return IndexOf(value as Customer);
        }

        public void Insert(int index, object value)
        {
            Insert(index, value as Customer);
        }

        public void Remove(object value)
        {
            Remove(value as Customer);
        }
        #endregion

        #region IList<Customer> Members

        public int IndexOf(Customer item)
        {
            if (item == null)
            {
                throw new ArgumentNullException("item");
            }
            return this._customers.IndexOf(item);
        }

        public void Insert(int index, Customer item)
        {
            if (item == null)
            {
                throw new ArgumentNullException("item");
            }
            this._customers.Insert(index, item);
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, item, index));
        }

        public void RemoveAt(int index)
        {
            Customer customer = this._customers[index];
            this._customers.RemoveAt(index);
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, customer, index));
        }

        public Customer this[int index]
        {
            get
            {
                return this._customers[index];
            }
            set
            {
                if (this._customers[index] != value)
                {
                    if (value == null)
                    {
                        throw new ArgumentNullException("value");
                    }
                    this._customers[index] = value;
                    RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Replace, value, index));
                }
            }
        }

        #endregion

        #region ICollection<Customer> Members

        public void Add(Customer item)
        {
            if (item == null)
            {
                throw new ArgumentNullException("item");
            }
            this._customers.Add(item);
            //AttachEventHandlers(customer);
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Add, item, this.Count - 1));
        }

        public void Clear()
        {
            this._customers.Clear();
            RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Reset));
        }

        public bool Contains(Customer item)
        {
            if (item == null)
            {
                throw new ArgumentNullException("value");
            }
            return this._customers.Contains(item);
        }

        public void CopyTo(Customer[] array, int arrayIndex)
        {
            this._customers.CopyTo(array, arrayIndex);
        }

        public int Count
        {
            get
            {
                return this._customers.Count;
            }
        }

        public bool Remove(Customer customer)
        {
            if (customer == null)
            {
                throw new ArgumentNullException();
            }
            int index = this._customers.IndexOf(customer);
            if (index > -1)
            {
                this._customers.RemoveAt(index);
                RaiseCollectionChanged(new NotifyCollectionChangedEventArgs(NotifyCollectionChangedAction.Remove, customer, index));
                return true;
            }
            return false;
        }

        #endregion

        #region IEnumerable<Customer> Members

        public IEnumerator<Customer> GetEnumerator()
        {
            return this._customers.GetEnumerator();
        }

        #endregion

        #region IEnumerable Members

        System.Collections.IEnumerator System.Collections.IEnumerable.GetEnumerator()
        {
            return this._customers.GetEnumerator();
        }

        #endregion

        #region Private methods

        private void RaiseCollectionChanged(NotifyCollectionChangedEventArgs e)
        {
            if (CollectionChanged != null)
            {
                CollectionChanged(this, e);
            }
        }

        #endregion Private methods

    }
}
