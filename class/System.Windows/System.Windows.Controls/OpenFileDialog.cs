//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2007-2008, 2010 Novell, Inc (http://www.novell.com)
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
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;

using Mono;

namespace System.Windows.Controls
{
	// It simply opens a native file dialog

	public sealed class OpenFileDialog
	{
		FileInfo[] files;
		
		bool allow_multiple_selection;
		string filter = String.Empty;
		int filter_index;
		
		public OpenFileDialog ()
		{
			EnsureMainThread ();
		}

		// it's not clear from doc or tests when this can return null
		public bool? ShowDialog ()
		{
			EnsureMainThread ();

			// the dialog is displayed only if the action leading to this call was initiated directly from the user
			if (!Helper.IsUserInitiated ())
				throw new SecurityException ("Action was not initiated by the user");

			IntPtr windowing_system = NativeMethods.runtime_get_windowing_system ();
			IntPtr result = NativeMethods.moon_windowing_system_show_open_file_dialog (windowing_system,
				"Open",
				allow_multiple_selection,
				filter,
				filter_index);

			if (result == IntPtr.Zero){
				// when called several times the previous results are still available after a Cancel
				return false;
			}

			uint inc = (uint) IntPtr.Size;
			IntPtr p;
			int n = 0;

			for (uint ofs = 0; (p = Marshal.ReadIntPtr ((IntPtr)((ulong)result + ofs))) != IntPtr.Zero; ofs += inc)
				n++;

			files = new FileInfo [n];
			for (uint i = 0, ofs = 0; (p = Marshal.ReadIntPtr ((IntPtr)((ulong)result + ofs))) != IntPtr.Zero; ofs += inc)
				files [i++] = new FileInfo (Marshal.PtrToStringAnsi (p));
			
			NativeMethods.g_free_pinvoke (result);
			return true;
		}

		// selection results

		public FileInfo File {
			get {
				EnsureMainThread ();
				return ((files == null) || (files.Length == 0)) ? null : files [0];
			}
		}

		public IEnumerable<FileInfo> Files {
			get { 
				EnsureMainThread ();
				return files;
			}
		}

		// dialog options

		public bool Multiselect {
			get {
				EnsureMainThread ();
				return allow_multiple_selection;
			}
			set {
				EnsureMainThread ();
				allow_multiple_selection = value;
			}
		}

		public string Filter {
			get { 
				EnsureMainThread ();
				return filter;
			}
			set {
				EnsureMainThread ();
				if (String.IsNullOrEmpty (value)) {
					filter = String.Empty;
				} else {
					// an odd number of '|' must be present (so an exact number of name+pattern pairs exists)
					int count = 0;
					for (int i=0; i < value.Length; i++) {
						if (value [i] == '|')
							count++;
					}
					if ((count & 1) == 0)
						throw new ArgumentException ("Filter");
					filter = value;
				}
			}
		}

		public int FilterIndex {
			get {
				EnsureMainThread ();
				return filter_index;
			}
			set {
				EnsureMainThread ();
				// note: the value isn't semi-validated (no maximum check wrt filters)
				if (value <= 0)
					throw new ArgumentOutOfRangeException ("FilterIndex");
				filter_index = value;
			}
		}

		static void EnsureMainThread ()
		{
			if (!Helper.CheckAccess ())
				throw new InvalidOperationException ("Must be called from the main thread");
		}
	}
}
