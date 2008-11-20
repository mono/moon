/*
 * IgnoreAttribute.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */


using System;

namespace Microsoft.VisualStudio.TestTools.UnitTesting
{
	[AttributeUsage (AttributeTargets.Method | AttributeTargets.Class, AllowMultiple=false)]
	public sealed class IgnoreAttribute : Attribute
	{
		private string reason; // Moonlight addition

		public IgnoreAttribute ()
		{
		}

		public IgnoreAttribute (string reason)  // Moonlight addition
		{
			this.reason = reason;
		}

		public string Reason {  // Moonlight addition
			get { return reason; }
		}
	}
}