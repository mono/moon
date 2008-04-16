//
// OpenFileDialog.cs
//
// Authors:
//	Atsushi Enomoto  <atsushi@ximian.com>
//	Miguel de Icaza  <miguel@ximian.com>
//	Sebastien Pouliot  <sebastien@ximian.com>
//
// Copyright (C) 2007 Novell, Inc (http://www.novell.com)
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
using System.Runtime.InteropServices;

namespace System.Windows.Controls
{
	// It simply opens a native file dialog

	public sealed class OpenFileDialog : IDisposable 
	{
		FileDialogFileInfo [] files;
		
		bool allow_multiple_selection;
		string filter = String.Empty;
		int filter_index;
		string title;
		
		public OpenFileDialog ()
		{
		}

		public void Dispose ()
		{
			// note: Dispose doesn't clear the SelectedFile or SelectedFiles properties
			GC.SuppressFinalize (this);
		}

		public DialogResult ShowDialog ()
		{
			IntPtr result = open_file_dialog_show (
				String.IsNullOrEmpty (title) ? "Open" : title,
				allow_multiple_selection,
				filter,
				filter_index);

			if (result == IntPtr.Zero){
				// when called several times the previous results are still available after a Cancel
				return DialogResult.Cancel;
			}

			uint inc = (uint) IntPtr.Size;
			IntPtr p;
			int n = 0;

			for (uint ofs = 0; (p = Marshal.ReadIntPtr ((IntPtr)((ulong)result + ofs))) != IntPtr.Zero; ofs += inc)
				n++;

			files = new FileDialogFileInfo [n];
			for (uint i = 0, ofs = 0; (p = Marshal.ReadIntPtr ((IntPtr)((ulong)result + ofs))) != IntPtr.Zero; ofs += inc)
				files [i++] = new FileDialogFileInfo (Marshal.PtrToStringAnsi (p));
			
			return DialogResult.OK;
		}

		// selection results

		public FileDialogFileInfo SelectedFile {
			get {
				return files == null ? null : files [0];
			}
		}

		public IEnumerable<FileDialogFileInfo> SelectedFiles {
			get {
				if (files == null)
					yield break;
				foreach (FileDialogFileInfo f in files)
					yield return f;
			}
		}

		// dialog options

		public bool EnableMultipleSelection {
			get { return allow_multiple_selection; }
			set { allow_multiple_selection = value; }
		}

		public string Filter {
			get { return filter; }
			set {
				if (String.IsNullOrEmpty (value))
					filter = String.Empty;
				// an odd number of '|' must be present (so an exact number of name+pattern pairs exists)
				int count = 0;
				for (int i=0; i < value.Length; i++) {
					if (value [i] == '|')
						count++;
				}
				if ((count & 1) == 1)
					throw new ArgumentException ("Filter");
				filter = value;
			}
		}

		public int FilterIndex {
			get { return filter_index; }
			set {
				// note: the value isn't semi-validated (no maximum check wrt filters)
				if (value < 0)
					throw new ArgumentOutOfRangeException ("FilterIndex");
				filter_index = value;
			}
		}

		public string Title {
			get { return title; }
			set { title = value; }
		}

		[DllImport ("moon")]
		static extern IntPtr open_file_dialog_show (string title, bool multsel, string filter, int idx);
	}
}
