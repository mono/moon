using System;
using System.Windows;

namespace Mono {

	class WeakLayoutUpdatedListener : IWeakListener {

		WeakReference listener;

		IListenLayoutUpdated Listener {
			get { return (IListenLayoutUpdated) listener.Target; }
			set { listener = new WeakReference (value); }
		}

		Deployment Source {
			get; set;
		}

		public WeakLayoutUpdatedListener (Deployment source, IListenLayoutUpdated listener)
		{
			Source = source;
			Listener = listener;
			Source.LayoutUpdated += HandleSourceLayoutUpdated;
		}

		void HandleSourceLayoutUpdated (object sender, EventArgs e)
		{
			var l = Listener;
			if (l == null)
				Detach ();
			else
				l.OnLayoutUpdated (sender, e);
		}

		public void Detach ()
		{
			Source.LayoutUpdated -= HandleSourceLayoutUpdated;;
		}
	}
}

