//
// BindingBase.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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

using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Collections.Generic;

using Mono;

namespace System.Windows.Data {
	public abstract class BindingBase {

		object fallbackValue;
		bool issealed;
		string stringFormat;
		object targetNullValue;

		public object FallbackValue {
			get {  return fallbackValue; }
			set {
				CheckSealed ();
				fallbackValue = value;
			}
		}

		internal bool Sealed {
			get {
				return issealed;
			}
			private set {
				issealed = value;
			}
		}

		public string StringFormat {
			get { return stringFormat; }
			set {
				CheckSealed ();
				stringFormat = value;
			}
		}

		public object TargetNullValue {
			get {  return targetNullValue; }
			set {
				CheckSealed ();
				targetNullValue = value;
			}
		}

		protected BindingBase ()
		{
			
		}

		protected void CheckSealed ()
		{
			if (Sealed)
				throw new InvalidOperationException ("The Binding cannot be changed after it has been used");
		}

		internal void Seal ()
		{
			Sealed = true;
		}
	}
}
