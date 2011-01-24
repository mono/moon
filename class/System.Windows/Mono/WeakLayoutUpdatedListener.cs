using System;
using System.Windows;

namespace Mono {

	class WeakLayoutUpdatedListener : IWeakListener {

		WeakReference listener;

		EventHandler Listener {
			get { return (EventHandler) listener.Target; }
			set { listener = new WeakReference (value); }
		}

		Deployment Source {
			get; set;
		}

		public WeakLayoutUpdatedListener (Deployment source, EventHandler listener)
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
				l (sender, e);
		}

		public void Detach ()
		{
			Source.LayoutUpdated -= HandleSourceLayoutUpdated;;
		}
	}
}

