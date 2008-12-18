//
// Moonlight.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
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
using System;
using System.Reflection;
using System.IO;

namespace Mono {

	/// <summary>
	///    This helper class used to configure Moonlight's backend
	/// </summary>
	/// <remarks>
	///    Applications might want to configure Moonlight's backend,
	///    this class is the public API that might be used to do so.
	///
	///    This class exists in agmono, so it is possible to configure
	///    desktop usage of Moonlight without having to expose other
	///    entry points in agclr, or resorting to reflection hacks. 
	/// </remarks>
	internal static class Moonlight {
		static LoaderCallback loader_callback;
		static ResourceLoaderCallback resource_loader_callback;

		/// <summary>
		///    Signature for clients providing custom assembly loading.
		/// </summary>
		/// <remarks>
		///    This method should return an assembly or null on failure.
		/// </remarks>
		public delegate Assembly LoaderCallback (string asm_path);

		/// <summary>
		///    Signature for clients providing custom resource loading.
		/// </summary>
		/// <remarks>
		///    This method should return a Stream or null on failure.
		/// </remarks>
		public delegate Stream ResourceLoaderCallback (string path);

		
		/// <summary>
		///    Registers assembly and resource loaders. 
		/// </summary>
		/// <remarks>
		///    If you want to customize the way assemblies or resources are loaded, invoke
		///    this method providing your own implementations. 
		/// </remarks>
		static public void RegisterLoader (LoaderCallback cb, ResourceLoaderCallback rcb)
		{
			loader_callback = cb;
			resource_loader_callback = rcb;
		}

		//
		// Documentation pending until we resolve why this code no longer calls Assembly.LoadFile
		//
		static public Assembly LoadFile (string asm_path)
		{
			if (loader_callback != null)
				return loader_callback (asm_path);

			return null;
		}

		static public Stream LoadResource (string path)
		{
			if (resource_loader_callback != null)
				return resource_loader_callback (path);

			return null;
		}
	}
}
