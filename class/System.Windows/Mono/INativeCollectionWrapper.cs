using System;
using System.Collections;
using System.Windows;

namespace Mono {

	interface INativeCollectionWrapper : INativeDependencyObjectWrapper {
		IList ManagedList { get; }
	}
}

