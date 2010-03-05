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

using System;

namespace System.Windows {
	public sealed class DataObject : IDataObject {
		public DataObject ()
		{
			Console.WriteLine ("System.Windows.DataObject..ctor: NIEX");
			throw new NotImplementedException ();
		}

		public DataObject (object data)
		{
			Console.WriteLine ("System.Windows.DataObject..ctor: NIEX");
			throw new NotImplementedException ();
		}

		public object GetData (string format)
		{
			Console.WriteLine ("System.Windows.DataObject.GetData (string): NIEX");
			throw new NotImplementedException ();
		}

		public object GetData (Type format)
		{
			Console.WriteLine ("System.Windows.DataObject.GetData (Type): NIEX");
			throw new NotImplementedException ();
		}

		public object GetData (string format, bool autoConvert)
		{
			Console.WriteLine ("System.Windows.DataObject.GetData (string, bool): NIEX");
			throw new NotImplementedException ();
		}

		public bool GetDataPresent (string format)
		{
			Console.WriteLine ("System.Windows.DataObject.GetDataPresent (string): NIEX");
			throw new NotImplementedException ();
		}

		public bool GetDataPresent (Type format)
		{
			Console.WriteLine ("System.Windows.DataObject.GetDataPresent (Type): NIEX");
			throw new NotImplementedException ();
		}

		public bool GetDataPresent (string format, bool autoConvert)
		{
			Console.WriteLine ("System.Windows.DataObject.GetDataPresent (string, bool): NIEX");
			throw new NotImplementedException ();
		}

		public string [] GetFormats ()
		{
			Console.WriteLine ("System.Windows.DataObject.GetFormats (): NIEX");
			throw new NotImplementedException ();
		}

		public string [] GetFormats (bool autoConvert)
		{
			Console.WriteLine ("System.Windows.DataObject.GetFormats (bool): NIEX");
			throw new NotImplementedException ();
		}

		public void SetData (object data)
		{
			Console.WriteLine ("System.Windows.DataObject.SetData (object): NIEX");
			throw new NotImplementedException ();
		}

		public void SetData (string format, object data)
		{
			Console.WriteLine ("System.Windows.DataObject.SetData (string, object): NIEX");
			throw new NotImplementedException ();
		}

		public void SetData (Type format, object data)
		{
			Console.WriteLine ("System.Windows.DataObject.SetData (Type, object): NIEX");
			throw new NotImplementedException ();
		}

		public void SetData (string format, object data, bool autoConvert)
		{
			Console.WriteLine ("System.Windows.DataObject.SetData (string, object, bool): NIEX");
			throw new NotImplementedException ();
		}
	}
}

