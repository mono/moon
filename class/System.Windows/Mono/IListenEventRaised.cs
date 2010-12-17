using System;

namespace Mono {

	interface IListenEventRaised {
		void OnEventRaised (object o, EventArgs e);
	}
}

