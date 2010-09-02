//
// CollectionViewSource.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#pragma warning disable 67 // "The event 'E' is never used" shown for Filter

using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Globalization;
using System.Runtime.CompilerServices;
using System.Windows;

namespace System.Windows.Data {

	public partial class CollectionViewSource : DependencyObject, IDeferRefresh, ISupportInitialize {
		public static readonly DependencyProperty SourceProperty =
			DependencyProperty.Register ("Source", typeof (object), typeof (CollectionViewSource),
							 new PropertyMetadata (null, SourceChanged));

		static void SourceChanged (DependencyObject o, DependencyPropertyChangedEventArgs e)
		{
			((CollectionViewSource) o).OnSourceChanged (e.OldValue, e.NewValue);
		}

		public static readonly DependencyProperty ViewProperty =
			DependencyProperty.Register ("View", typeof (ICollectionView), typeof (CollectionViewSource), null);

		CultureInfo culture;
		FilterEventHandler filter;

		public event FilterEventHandler Filter {
			add {
				filter += value;
				Refresh ();
			}
			remove {
				filter -= value;
				Refresh ();
			}
		}

		// FIXME: This should be a ConditionalWeakTable. DRT 232 shows that the CollectionViewSource
		// does not hold strong references to the source collections or the ICollectionViews it caches.
		// We could fake it with an IDictionary<WeakHandle<K>, WeakHandle<V>> maybe.
		IDictionary <object, ICollectionView> CachedViews {
			get; set;
		}

		public CultureInfo Culture {
			get { return culture; }
			set {
				culture = value;
				Refresh ();
			}
		}

		int IDeferRefresh.DeferLevel {
			get; set;
		}

		Predicate <object> FilterCallback {
			get; set;
		}

		public ObservableCollection<GroupDescription> GroupDescriptions {
			get; private set;
		}

		public SortDescriptionCollection SortDescriptions {
			get; private set;
		}

		public object Source {
			get { return (object) GetValue (SourceProperty); }
			set { SetValue (SourceProperty, value); }
		}

		public ICollectionView View {
			get { return (ICollectionView) GetValue (ViewProperty); }
			private set { SetValueImpl (ViewProperty, value); }
		}

		public CollectionViewSource ()
		{
			CachedViews = new Dictionary<object, ICollectionView> ();
			SortDescriptions = new SortDescriptionCollection ();
			GroupDescriptions = new ObservableCollection<GroupDescription> ();

			GroupDescriptions.CollectionChanged += (o, e) => Refresh ();
			((INotifyCollectionChanged)SortDescriptions).CollectionChanged += (o, e) => Refresh ();
			FilterCallback = (o) => {
				var h = filter;
				if (h != null) {
					var args = new FilterEventArgs (o);
					h (this, args);
					return args.Accepted;
				}
				return true;
			};
		}

		public IDisposable DeferRefresh ()
		{
			return new Deferrer (this);
		}

		protected virtual void OnCollectionViewTypeChanged (Type oldCollectionViewType, Type newCollectionViewType)
		{
			
		}

		protected virtual void OnSourceChanged (object oldSource, object newSource)
		{
			if (newSource == null) {
				View = null;
			} else {
				ICollectionViewFactory factory = newSource as ICollectionViewFactory;
				if (factory != null) {
					View = factory.CreateView ();
				} else {
					ICollectionView view = null;
					if (CachedViews.TryGetValue (newSource, out view)) {
						View = view;
					} else {
						view = CollectionView.Create ((IEnumerable) newSource);
						CachedViews.Add (newSource, view);
						View = view;
					}
				}
			}

			Refresh ();
		}

		void Refresh ()
		{
			if (((IDeferRefresh) this).DeferLevel != 0 || View == null)
				return;

			using (View.DeferRefresh ()) {
				if (Culture != null)
					View.Culture = Culture;
				View.GroupDescriptions.Clear ();
				for (int i = 0; i < GroupDescriptions.Count; i++)
					View.GroupDescriptions.Add (GroupDescriptions [i]);

				View.SortDescriptions.Clear ();
				for (int i = 0; i < SortDescriptions.Count; i++)
					View.SortDescriptions.Add (SortDescriptions [i]);

				View.Filter = (filter == null) ? null : FilterCallback;
			}
		}

		void IDeferRefresh.Refresh ()
		{
			Refresh ();
		}

		#region ISupportInitialize implementation
		void ISupportInitialize.BeginInit ()
		{
			Console.WriteLine ("NIEX: System.Windows.Data.CollectionViewSource:.ISupportInitialize.BeginInit");
			throw new System.NotImplementedException();
		}

		void ISupportInitialize.EndInit ()
		{
			Console.WriteLine ("NIEX: System.Windows.Data.CollectionViewSource:.ISupportInitialize.EndInit");
			throw new NotImplementedException ();
		}
		#endregion
	}
}
