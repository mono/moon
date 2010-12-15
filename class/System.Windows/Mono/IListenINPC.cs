using System;
using System.ComponentModel;

namespace Mono {

	public interface IListenINPC {
		void OnPropertyChanged (object o, PropertyChangedEventArgs e);
	}
}

