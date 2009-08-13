//
// System.Windows.Automation.Peers.AutomationCacheProperty
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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

using System;
using System.Collections.Generic;
using System.Windows.Automation.Provider;

namespace System.Windows.Automation.Peers {

	// Internal interface used by AutomationSingleton.InvalidatePeer to raise Automation Events
	internal interface IAutomationCacheProperty {
		AutomationProperty Property { get; set; }
		object OldValue { get; set; }
		bool CompareValues (out object oldValue);
	}

	// Class used by AutomationPeer to cache main properties
	internal class AutomationCacheProperty<TResult> : IAutomationCacheProperty {

		public AutomationProperty Property {
			get; set;
		}

		public virtual object OldValue {
			get; set;
		}

		public Func<TResult> Delegate {
			get; set;
		}

		public bool CompareValues (out object oldValue)
		{
			object newValue = Delegate ();
			if (object.Equals (newValue, OldValue)) {
				oldValue = null;
				return false;
			} else {
				oldValue = OldValue;
				OldValue = newValue;
				return true;
			}
		}
	}

	internal class AutomationCachePeerProperty : AutomationCacheProperty<AutomationPeer>  {

		public override object OldValue {
			get { return reference.Target; }
			set {
				if (reference == null)
					reference = new WeakReference (value);
				else
					reference.Target = value;
			}
		}

		private WeakReference reference;
	}
}

