/*
 * Zip.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;

class ZipContent {
	string unzipped_filename;
	
	public Zip Zip { get; set; }
	public string Type { get { return Path.GetExtension (Filename); } }
	public string Filename { get; set; }
	public string UnzippedFilename {
		get {
			if (unzipped_filename == null) {
				Zip.Unzip ();
				unzipped_filename = Path.Combine (Zip.UnzipDirectory, Filename);
			}
			return unzipped_filename;
		}
	}
}

class Zip : IDisposable
{
	List<ZipContent> files;
	string unzip_directory;
	bool unzipped;
	
	public string FileName { get; set; }
	public string UnzipDirectory {
		get {
			if (unzip_directory == null) {
				unzip_directory = Path.Combine (Path.GetTempPath (), "munxap");
				unzip_directory = Path.Combine (unzip_directory, Path.GetFileName (FileName));
				unzip_directory = Path.Combine (unzip_directory, "pid-" + System.Diagnostics.Process.GetCurrentProcess ().Id.ToString ());
				Directory.CreateDirectory (unzip_directory);
			}
			return unzip_directory;
		}
	}
	public Zip (string filename)
	{
		FileName = filename;
	}
	
	~Zip ()
	{
		Dispose ();
	}

	public void Dispose ()
	{
		if (unzip_directory != null) {
			try {
				Directory.Delete (unzip_directory);
			} catch {
				// Ignore any exceptions
			}
		}
	}
	
	public void Unzip ()
	{
		if (unzipped)
			return;
		
		using (System.Diagnostics.Process p = new System.Diagnostics.Process ()) {
			p.StartInfo.FileName = "unzip";
			p.StartInfo.Arguments = string.Format ("\"{0}\" -d \"{1}\" ", FileName, UnzipDirectory);
			p.StartInfo.RedirectStandardOutput = true;
			p.StartInfo.UseShellExecute = false;
			p.Start ();
		 	string output = p.StandardOutput.ReadToEnd ();
			p.WaitForExit ();
			if (p.ExitCode != 0)
				throw new Exception (output);
		}

		unzipped = true;
	}
	
	public List<ZipContent> Files {
		get {
			if (files == null) {
				files = new List<ZipContent> ();
				
				using (System.Diagnostics.Process p = new System.Diagnostics.Process ()) {
				 	p.StartInfo.FileName = "unzip";
					p.StartInfo.Arguments = "-lqq \"" + FileName + "\"";
					p.StartInfo.RedirectStandardOutput = true;
					p.StartInfo.UseShellExecute = false;
					p.Start ();
					string output = p.StandardOutput.ReadToEnd ();
					foreach (string line in output.Split (new char [] {'\n', '\r'}, StringSplitOptions.RemoveEmptyEntries)) {
						string [] args = line.Split (new char [] { ' '}, StringSplitOptions.RemoveEmptyEntries);
						if (args.Length < 4)
							continue;
		
						ZipContent content = new ZipContent ();
						content.Zip = this;
						content.Filename = args [3];
						files.Add (content);
					}
				}
			}
			return files;
		}
	}

}
