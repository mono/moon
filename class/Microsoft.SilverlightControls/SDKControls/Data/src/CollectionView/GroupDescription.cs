// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.ObjectModel;   // ObservableCollection
using System.Collections.Specialized;   // NotifyCollectionChangedEvent*
using System.Globalization;             // CultureInfo

namespace System.ComponentModel
{
    /// <summary>
    /// Base class for group descriptions.
    /// A GroupDescription describes how to divide the items in a collection
    /// into groups.
    /// </summary>
    public abstract class GroupDescription : INotifyPropertyChanged
    {
        #region Constructors

        //------------------------------------------------------
        //
        //  Constructors
        //
        //------------------------------------------------------

        /// <summary>
        /// Initializes a new instance of GroupDescription.
        /// </summary>
        protected GroupDescription()
        {
            _explicitGroupNames = new ObservableCollection<object>();
            _explicitGroupNames.CollectionChanged += new NotifyCollectionChangedEventHandler(OnGroupNamesChanged);
        }

        #endregion Constructors

        #region INotifyPropertyChanged

        /// <summary>
        ///     This event is raised when a property of the group description has changed.
        /// </summary>
        event PropertyChangedEventHandler INotifyPropertyChanged.PropertyChanged
        {
            add
            {
                PropertyChanged += value;
            }
            remove
            {
                PropertyChanged -= value;
            }
        }

        /// <summary>
        /// PropertyChanged event (per <see cref="INotifyPropertyChanged" />).
        /// </summary>
        protected virtual event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// A subclass can call this method to raise the PropertyChanged event.
        /// </summary>
        protected virtual void OnPropertyChanged(PropertyChangedEventArgs e)
        {
            if (PropertyChanged != null)
            {
                PropertyChanged(this, e);
            }
        }

        #endregion INotifyPropertyChanged

        #region Public Properties

        //------------------------------------------------------
        //
        //  Public Properties
        //
        //------------------------------------------------------

        /// <summary>
        /// This list of names is used to initialize a group with a set of
        /// subgroups with the given names.  (Additional subgroups may be
        /// added later, if there are items that don't match any of the names.)
        /// </summary>
        public ObservableCollection<object> GroupNames
        {
            get { return _explicitGroupNames; }
        }

        /// <summary>
        /// This method is used by TypeDescriptor to determine if this property should
        /// be serialized.
        /// </summary>
        [EditorBrowsable(EditorBrowsableState.Never)]
        public bool ShouldSerializeGroupNames()
        {
            return (_explicitGroupNames.Count > 0);
        }

        #endregion Public Properties

        #region Public Methods

        //------------------------------------------------------
        //
        //  Public Methods
        //
        //------------------------------------------------------

        /// <summary>
        /// Return the group name(s) for the given item
        /// </summary>
        public abstract object GroupNameFromItem(object item, int level, CultureInfo culture);

        /// <summary>
        /// Return true if the names match (i.e the item should belong to the group).
        /// </summary>
        public virtual bool NamesMatch(object groupName, object itemName)
        {
            return Object.Equals(groupName, itemName);
        }

        #endregion Public Methods

        #region Private Methods

        //------------------------------------------------------
        //
        //  Private Methods
        //
        //------------------------------------------------------

        void OnGroupNamesChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            OnPropertyChanged(new PropertyChangedEventArgs("GroupNames"));
        }

        #endregion Private Methods

        #region Private fields

        //------------------------------------------------------
        //
        //  Private fields
        //
        //------------------------------------------------------

        ObservableCollection<object> _explicitGroupNames;

        #endregion Private fields
    }
}
