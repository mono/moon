using System;
using System.Windows;

namespace Mono {

	class WeakPropertyChangedListener : IWeakListener {

		WeakReference listener;
		IListenPropertyChanged Listener {
			get { return (IListenPropertyChanged) listener.Target; }
			set { listener = new WeakReference (value); }
		}

		DependencyProperty Property {
			get; set;
		}

		DependencyObject Source {
			get; set;
		}

		public WeakPropertyChangedListener (DependencyObject source, DependencyProperty property, IListenPropertyChanged listener)
		{
			Source = source;
			Property = property;
			Listener = listener;

			Source.AddPropertyChangedHandler (property, OnPropertyChanged);
		}

		public void Detach ()
		{
			if (Source != null) {
				Source.RemovePropertyChangedHandler (Property, OnPropertyChanged);
				Source = null;
			}
		}

		void OnPropertyChanged (IntPtr dependency_object, IntPtr propertyChangeArgs, ref MoonError error, IntPtr unused)
		{
			var l = Listener;
			if (l == null)
				Detach ();
			else
				l.OnPropertyChanged (dependency_object, propertyChangeArgs, ref error, unused);
		}
	}
}

