/*
 * UnitTestAssertException.cs.
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
	public class UnitTestAssertException : Exception
	{
		private string message;
		
		protected UnitTestAssertException ()
		{
		}
		
		protected UnitTestAssertException (string msg)
			: base (msg)
		{
			this.message = msg; 
		}

		internal UnitTestAssertException (string msg, Exception inner)
			: base (msg, inner)
		{
			this.message = msg;
		}
		
		public override string Message {
			get { return message == null ? base.Message: message; }
		}
	}
}