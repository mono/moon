//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009-2010 Novell, Inc (http://www.novell.com)
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

using System.IO;
using System.Security;

using Mono;

namespace System.Windows.Controls {

	public sealed class SaveFileDialog {

		string default_ext = String.Empty;
		string filter = String.Empty;
		int filter_index = 1;
		FileInfo file_info;
		string safe_file_name = String.Empty;

		public SaveFileDialog ()
		{
			EnsureMainThread ();
		}

		public string DefaultExt {
			get { 
				EnsureMainThread ();
				return default_ext;
			}
			set {
				EnsureMainThread ();
				if (String.IsNullOrEmpty (value)) {
					default_ext = String.Empty;
				} else if (value [0] == '.') {
					default_ext = value.Substring (1);
				} else {
					default_ext = value;
				}
			}
		}

		// note: required to run MS test suite
		private FileInfo File {
			get {
				EnsureMainThread ();
				return file_info;
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

		public Stream OpenFile ()
		{
			EnsureMainThread ();

			if (file_info == null)
				throw new InvalidOperationException ();

			return file_info.OpenWrite ();
		}

		public string SafeFileName {
			get {
				EnsureMainThread ();
				return safe_file_name;
			}
		}

		public bool? ShowDialog ()
		{
			EnsureMainThread ();

			// the dialog is displayed only if the action leading to this call was initiated directly from the user
			if (!Helper.IsUserInitiated ())
				throw new SecurityException ("Action was not initiated by the user");

			IntPtr windowing_system = NativeMethods.runtime_get_windowing_system ();
			string result = NativeMethods.moon_windowing_system_show_save_file_dialog (windowing_system,
												   "Save", filter, filter_index);

			if (result == null)
				return false;

			// if we have a "default extension" specified and that we're not replacing an existing file
			if (!String.IsNullOrEmpty (default_ext) && !System.IO.File.Exists (result)) {
				// and that the file has no extention, then we supply our default
				if (String.IsNullOrEmpty (Path.GetExtension (result)))
					result = Path.ChangeExtension (result, default_ext);
			}
			file_info = new FileInfo (result);
			safe_file_name = Path.GetFileName (result);
			
			return true;
		}

		static void EnsureMainThread ()
		{
			if (!Helper.CheckAccess ())
				throw new InvalidOperationException ("Must be called from the main thread");
		}
	}
}

