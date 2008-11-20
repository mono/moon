/*
 * TimeoutAttribute.cs.
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
	[AttributeUsage (AttributeTargets.Method, AllowMultiple=false)]
	public sealed class TimeoutAttribute : Attribute
	{
		private int timeout;

		public TimeoutAttribute (int timeout)
		{
			this.timeout = timeout;
		}

		public int Timeout {
			get { return timeout; }
		}
	}
}

