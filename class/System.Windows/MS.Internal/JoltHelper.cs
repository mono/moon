//
// InternalTransform.cs
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
using System.Reflection;

namespace MS.Internal
{
	/* This class is used in the drts */
	internal static class JoltHelper
	{
		internal static uint RegisterAssembly (Assembly assembly, bool b)
		{
			if (b) {
				Console.WriteLine ("System.Windows/MS.Internal.JoltHelper:RegisterAssembly (): boolean 'true' parameter not implemented");
				throw new NotImplementedException ();
			}

			// apparently some magic is required here so that the parser can find types in the provided (possibly dynamic) assembly
			Console.WriteLine ("System.Windows/MS.Internal.JoltHelper:RegisterAssembly ({0}): apparently some magic is required here so that the parser can find types in the provided (possibly dynamic) assembly", assembly.FullName);

			return 0; /* the return value seems to be ignored */
		}
	}
}

