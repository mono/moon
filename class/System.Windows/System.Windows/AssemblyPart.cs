//
// AssemblyPart.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008-2009 Novell, Inc.
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
using System.IO;
using System.Reflection;
using Mono;

namespace System.Windows {

	public sealed partial class AssemblyPart : DependencyObject {

		static AssemblyPart ()
		{
			SourceProperty = DependencyProperty.Lookup (Kind.ASSEMBLYPART, "Source", typeof (string));
		}

		static Type MemoryStreamType = typeof (MemoryStream);

		byte[] StreamToBuffer (Stream assemblyStream)
		{
			// it's normally bad to depend on Stream.Length since some stream (e.g. NetworkStream) 
			// don't implement them. However it is safe in this case (i.e. SL2 depends on Length too)
			var buffer = new byte [assemblyStream.Length];

			using (assemblyStream) {
				int length = buffer.Length;
				int offset = 0;
				while (length > 0) {
					int read = assemblyStream.Read (buffer, offset, length);
					if (read == 0)
						break;

					length -= read;
					offset += read;
				}
			}

			return buffer;
		}

		public Assembly Load (Stream assemblyStream)
		{
			// SL2 throws a NullReferenceException if assemblyStream is null
			// but returns null if the stream is empty (e.g. Stream.Null)
			if (assemblyStream.Length == 0)
				return null;

			byte[] buffer = null;
			// avoid extra step for MemoryStream (but not any stream that inherits from it)
			if (assemblyStream.GetType () == MemoryStreamType)
				buffer = (assemblyStream as MemoryStream).ToArray ();
			else
				buffer = StreamToBuffer (assemblyStream);

			try {
				Assembly result = Assembly.Load (buffer);
				if (!Deployment.Current.Assemblies.Contains (result))
					Deployment.Current.Assemblies.Add (result);	
				return result;
			}
			catch {
				return null;
			}
		}
	}
}
