using System;
using System.Diagnostics;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Text;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;

public partial class MoonLogProvider : System.Web.UI.Page
{
	protected void Page_Load (object sender, EventArgs e)
	{
		// Console.WriteLine ("MoonLogProvider");
		try {
			string filename, directory, path;
			byte [] bytes = new byte [1024];
			int read;

			if (Request.HttpMethod != "POST") {
				Log ("MoonLogProvider: Invalid method. Expected 'POST'.");
				return;
			}
			
			filename = Request ["filename"];
			directory = Environment.GetEnvironmentVariable ("MOONLIGHT_UNIT_HARNESS_LOG_DIRECTORY");
			
			if (string.IsNullOrEmpty (directory) && Environment.OSVersion.Platform == PlatformID.Win32NT)
				directory = @"C:\";

			if (string.IsNullOrEmpty (directory)) {
				directory = Environment.CurrentDirectory;
				Log ("MoonLogProvider: No log directory was set with MOONLIGHT_UNIT_HARNESS_LOG_DIRECTORY, writing to current directory '{0}'.", directory);
			}

			if (string.IsNullOrEmpty (filename)) {
				Log ("MoonLogProvider: No log file was provided.");
				return;
			}

			if (filename.IndexOfAny (Path.GetInvalidFileNameChars ()) != -1) {
				Log ("MoonLogProvider: The filename '{0}' contains invalid characters.", filename);
				return;
			}

			path = Path.Combine (directory, filename);
			
			Log ("MoonLogProvider: Saving file to: {0}", path);
			
			using (BinaryReader reader = new BinaryReader (Request.InputStream)) {
				using (FileStream fs = new FileStream (path, FileMode.Append, FileAccess.Write, FileShare.ReadWrite)) {
					while (true) {
						read = reader.Read (bytes, 0, bytes.Length);
						// Log ("MoonLogProvider: Read {0} bytes.", read);
						// Log ("Read: {0}", System.Text.ASCIIEncoding.ASCII.GetString (bytes, 0, read));
						if (read == 0)
							break;
						fs.Write (bytes, 0, read);
					}
				}
			}
		} catch (Exception ex) {
			Log ("MoonLogProvider: Exception while handling log message: {0}", ex.ToString ());
		}
	}

	static void Log (string msg, params string [] args)
	{
		Console.WriteLine (msg, args);
		if (Environment.OSVersion.Platform == PlatformID.Win32NT)
			Debug.WriteLine (string.Format (msg, args));
	}
}
