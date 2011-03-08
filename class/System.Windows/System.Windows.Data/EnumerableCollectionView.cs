using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.ComponentModel;
using System.Linq;
using System.Collections.Specialized;
using System.Windows.Data;

// Drt 233 checks that this class is in the Microsoft.Internal namespace.
namespace MS.Internal {

	class EnumerableCollectionView : ICollectionView, INotifyPropertyChanged, IDeferRefresh {

		ObservableCollection<object> clonedCollection;
		ListCollectionView view;

		public event EventHandler CurrentChanged;
		public event CurrentChangingEventHandler CurrentChanging;
		public event NotifyCollectionChangedEventHandler CollectionChanged;
		public event PropertyChangedEventHandler PropertyChanged;
		
		public bool CanFilter {
			get { return view.CanFilter; }
		}

		public bool CanGroup {
			get { return view.CanGroup; }
		}

		public bool CanSort {
			get { return view.CanSort; }
		}

		public System.Globalization.CultureInfo Culture {
			get { return view.Culture; }
			set { view.Culture = value; }
		}

		public object CurrentItem {
			get { return view.CurrentItem; }
		}

		public int CurrentPosition {
			get { return view.CurrentPosition; }
		}

		int IDeferRefresh.DeferLevel {
			get { return ((IDeferRefresh) view).DeferLevel; }
			set { ((IDeferRefresh) view).DeferLevel = value; }
		}

		public Predicate<object> Filter {
			get { return view.Filter; }
			set { view.Filter = value; }
		}

		public ObservableCollection<GroupDescription> GroupDescriptions {
			get { return view.GroupDescriptions; }
		}

		public ReadOnlyObservableCollection<object> Groups {
			get { return view.Groups; }
		}

		public bool IsCurrentAfterLast {
			get { return view.IsCurrentAfterLast; }
		}

		public bool IsCurrentBeforeFirst {
			get { return view.IsCurrentBeforeFirst; }
		}

		public bool IsEmpty {
			get { return view.IsEmpty; }
		}

		public SortDescriptionCollection SortDescriptions {
			get { return view.SortDescriptions; }
		}

		public IEnumerable SourceCollection {
			get; private set;
		}

		public EnumerableCollectionView (IEnumerable collection)
		{
			SourceCollection = collection;

			clonedCollection = new ObservableCollection<object> (collection.Cast <object> ());
			view = new ListCollectionView (clonedCollection);

			var c = collection as INotifyCollectionChanged;
			if (c != null)
				c.CollectionChanged += HandleCollectionChanged;

			view.CollectionChanged += (o, e) => {
				var h = CollectionChanged;
				if (h != null)
					h (this, e);
			};
			view.CurrentChanged += (o, e) => {
				var h = CurrentChanged;
				if (h != null)
					h (this, e);
			};
			view.CurrentChanging += (o, e) => {
				var h = CurrentChanging;
				if (h != null)
					h (this, e);
			};
			view.PropertyChanged += (o, e) => {
				var h = PropertyChanged;
				if (h != null)
					h (this, e);
			};
		}

		void HandleCollectionChanged (object sender, NotifyCollectionChangedEventArgs e)
		{
			switch (e.Action) {
			case NotifyCollectionChangedAction.Add:
				for (int i = 0; i < e.NewItems.Count; i++)
					clonedCollection.Insert (e.NewStartingIndex + i, e.NewItems [i]);
				break;
			case NotifyCollectionChangedAction.Remove:
				for (int i = e.OldItems.Count; i > 0; i--)
					clonedCollection.RemoveAt (e.OldStartingIndex);
				break;

			case NotifyCollectionChangedAction.Replace:
				for (int i = 0; i < e.OldItems.Count; i++)
					clonedCollection[e.NewStartingIndex + i] = e.NewItems [i];
				break;
			case NotifyCollectionChangedAction.Reset:
				clonedCollection.Clear ();
				foreach (var v in SourceCollection)
					clonedCollection.Add (v);
				break;
			default:
				throw new NotSupportedException ();
			}
		}

		public bool Contains (object item)
		{
			return view.Contains (item);
		}

		public IDisposable DeferRefresh ()
		{
			return view.DeferRefresh ();
		}

		public bool MoveCurrentTo (object item)
		{
			return view.MoveCurrentTo (item);
		}

		public bool MoveCurrentToFirst ()
		{
			return view.MoveCurrentToFirst ();
		}

		public bool MoveCurrentToLast ()
		{
			return view.MoveCurrentToLast ();
		}

		public bool MoveCurrentToNext ()
		{
			return view.MoveCurrentToNext ();
		}

		public bool MoveCurrentToPosition (int position)
		{
			return view.MoveCurrentToPosition (position);
		}

		public bool MoveCurrentToPrevious ()
		{
			return view.MoveCurrentToPrevious ();
		}

		public void Refresh ()
		{
			view.Refresh ();
		}

		public IEnumerator GetEnumerator ()
		{
			return view.GetEnumerator ();
		}
	}
}
