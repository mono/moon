// 
// DataObject.cs
// 
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
// 
// Copyright 2010 Novell, Inc.
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

using System.Security;

namespace System.Windows {

	[MonoTODO]
	public sealed class DataObject : IDataObject {

		// FIXME: only a very small subset if supported by SL4 - otherwise most of it simply throws SecurityException

		public DataObject ()
		{
			Console.WriteLine ("System.Windows.DataObject..ctor: NIEX");
		}

		public DataObject (object data)
		{
			// this is what SL4 throws (so it's like implemented ;-)
			throw new NotImplementedException ();
		}

		public object GetData (string format)
		{
			throw new SecurityException ();
		}

		public object GetData (Type format)
		{
			throw new SecurityException ();
		}

		public object GetData (string format, bool autoConvert)
		{
			throw new SecurityException ();
		}

		public bool GetDataPresent (string format)
		{
			throw new SecurityException ();
		}

		public bool GetDataPresent (Type format)
		{
			throw new SecurityException ();
		}

		public bool GetDataPresent (string format, bool autoConvert)
		{
			throw new SecurityException ();
		}

		public string [] GetFormats ()
		{
			throw new SecurityException ();
		}

		public string [] GetFormats (bool autoConvert)
		{
			throw new SecurityException ();
		}

		public void SetData (object data)
		{
			throw new SecurityException ();
		}

		public void SetData (string format, object data)
		{
			throw new SecurityException ();
		}

		public void SetData (Type format, object data)
		{
			throw new SecurityException ();
		}

		public void SetData (string format, object data, bool autoConvert)
		{
			throw new SecurityException ();
		}
	}
}

