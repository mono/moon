//
// HeapUtil.cs
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

#if HEAPVIZ

using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;

namespace Mono {

	internal class HeapRef {
		public HeapRef (INativeEventObjectWrapper referent)
			: this (true, referent, "")
		{
		}

		public HeapRef (bool strong,
				INativeEventObjectWrapper referent,
				string name)
		{
			Strong = strong;
			Referent = referent;
			Name = name;
		}

		public bool Strong;
		public INativeEventObjectWrapper Referent;
		public string Name;
	}

	
	internal class HeapUtil {

		[StructLayout (LayoutKind.Explicit)]
		struct Foo {
			[FieldOffset(0)] public object obj;
			[FieldOffset(0)] public IntPtr intptr;
		}

		static string skipEdges = "TemplateOwner,Deployment,VisualParent,Mentor";

		static bool dumpGraphs = Environment.GetEnvironmentVariable ("MOON_DUMP_HEAP") != null;

		static int dumpDepth = Environment.GetEnvironmentVariable ("MOON_DUMP_DEPTH") == null ? 0 : Int32.Parse (Environment.GetEnvironmentVariable("MOON_DUMP_DEPTH"));

		static Dictionary<IntPtr,IntPtr> visited = new Dictionary<IntPtr,IntPtr>();

		public static void GraphManagedHeap (string name)
		{
			if (!dumpGraphs)
				return;

			try {

				Console.WriteLine ("starting to graph heap name {0}", name);

				StreamWriter writer = File.CreateText ("/tmp/" + name);

				writer.WriteLine ("digraph {0} {{", name);

				lock (NativeDependencyObjectHelper.objects) {
					foreach (IntPtr nativeref in NativeDependencyObjectHelper.objects.Keys) {
						Foo targetInfo;
						GCHandle handle = NativeDependencyObjectHelper.objects[nativeref];

						if (handle.Target == null)
							continue;

						targetInfo.obj = handle.Target;

#if true
						writer.WriteLine ("  unmanaged0x{0:x} -> managed0x{1:x} [color=green];",
								  (int)nativeref,
								  (int)targetInfo.intptr);
						writer.WriteLine ("  unmanaged0x{0:x} [label=\"unmanaged0x{0:x}\",fillcolor=green,style=filled];",
								  (int)nativeref);

						if (handle.Target is IRefContainer && !visited.ContainsKey (targetInfo.intptr))
							OutputManagedRefs ((IRefContainer)handle.Target, targetInfo.intptr, writer, 1);
#else
						if (!(tref.reference is WeakReference) || false/*output_weak*/) {
							writer.WriteLine ("  unmanaged0x{0:x} -> managed0x{1:x} [label=\"{2}\",color={3}];",
									  (int)nativeref,
									  (int)targetInfo.intptr,
									  tref.reference is WeakReference ? "weak" : "strong",
									  tref.reference is WeakReference ? "green" : "red");
							writer.WriteLine ("  unmanaged0x{0:x} [label=\"unmanaged0x{0:x}\",fillcolor={1},style=filled];",
									  (int)nativeref,
									  tref.reference is WeakReference ? "green" : "red");

							if (tref.Target is IRefContainer && !visited.ContainsKey (targetInfo.intptr))
								OutputManagedRefs ((IRefContainer)tref.Target, targetInfo.intptr, writer, 1);
						}
#endif
					}					
				}

				writer.WriteLine ("}");

				writer.Close ();

				Console.WriteLine ("done graphing heap");
			}
			catch (Exception e) {
				Console.WriteLine ("Exception graphing heap: {0}", e);
			}

			visited.Clear ();
		}

		public static void OutputManagedRefs (IRefContainer eo, IntPtr intptr, StreamWriter writer, int depth)
		{
			visited[intptr] = intptr;
			writer.WriteLine ("  managed0x{0:x} [label=\"{1}\"];", (int)intptr, eo.GetType().Name);

			if (dumpDepth > 0 && depth > dumpDepth)
				return;

			foreach (HeapRef href in eo.GetManagedRefs()) {
				if (href.Referent == null)
					continue;

				if (!string.IsNullOrEmpty (href.Name) && skipEdges.Contains (href.Name))
					continue;

				string edge_label = "";

				if (!href.Strong) {
					edge_label += "weak";
					if (href.Name != "")
						edge_label += "\n" + href.Name;
				}
				else if (href.Name != "")
					edge_label = href.Name;

				Foo targetInfo;

				targetInfo.obj = href.Referent;

				writer.WriteLine ("  managed0x{0:x} -> managed0x{1:x} [label=\"{2}\"];",
						  (int)intptr,
						  (int)targetInfo.intptr,
						  edge_label);

				if (href.Referent is IRefContainer && !visited.ContainsKey (targetInfo.intptr)) {
					OutputManagedRefs ((IRefContainer)href.Referent,
							   targetInfo.intptr, writer, depth + 1);
				}
			}
		}
		
	}
}

#endif