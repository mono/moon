//
// XamlParseException.cs
//
// Author:
//   Miguel de Icaza (miguel@novell.com)
//
// Copyright 2007 Novell, Inc.
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

// Temporary namespace, need to fix olive/class/PresentationFramework namespaces
namespace System.Windows.Markup {
	public class XamlParseException : SystemException {
		uint line, col;
		
		internal XamlParseException ()
		{
			line = 0;
			col = 0;
		}
		
		internal XamlParseException (string msg) : base (msg)
		{
			line = 0;
			col = 0;
		}
		
		internal XamlParseException (uint line, uint col)
		{
			this.line = line;
			this.col = col;
		}

		internal XamlParseException (int line, int col, string msg)
			: base (String.Format ("({0},{1}): {2}", line, col, msg))
		{
			this.line = (uint) line;
			this.col = (uint) col;
		}
		
		public uint LineNumber {
			get { return line; }
		}
		public uint LinePosition {
			get { return col; }
		}
	}
}
