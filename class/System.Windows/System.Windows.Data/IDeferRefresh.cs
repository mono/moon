using System;

namespace System.Windows.Data {

	interface IDeferRefresh {
		int DeferLevel { get; set; }
		void Refresh ();
	}
}

