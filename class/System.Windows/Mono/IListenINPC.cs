using System;
using System.ComponentModel;

namespace Mono {

	interface IListenINPC {
		void OnPropertyChanged (object o, PropertyChangedEventArgs e);
	}
}

