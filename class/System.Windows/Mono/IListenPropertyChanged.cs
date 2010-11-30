using System;

namespace Mono {

	interface IListenPropertyChanged {
		void OnPropertyChanged (IntPtr dependency_object, IntPtr propertyChangeArgs, ref MoonError error, IntPtr unused);
	}
}

