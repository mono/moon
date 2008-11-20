/*
 * ExpectedExceptionAttribute.cs.
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
	public class ExpectedExceptionAttribute : Attribute
	{
		private Type exception_type;
		private string message;

		public ExpectedExceptionAttribute (Type exceptionType) 
			: this (exceptionType, null)
		{
		}

		public ExpectedExceptionAttribute (Type exceptionType, string message)
		{
			this.exception_type = exceptionType;
			this.message = message;
		}
		
		public string Message {
			get { return message; }
		}
	
		public Type ExceptionType {
			get { return exception_type; }
		}
		
	}
}