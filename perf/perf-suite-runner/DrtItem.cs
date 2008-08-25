/*
 * Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
 *
 * Contact:
 *  Moonlight List (moonlight-list@lists.ximian.com)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

using System;
using System.Xml;
using System.IO;
using System.Diagnostics;

namespace PerfSuiteRunner {

	public class DrtItem {

		public int StartTime = 0;
		public int EndTime = 5000;
		public int Interval = 40;
		public int Timeout = 35000;
		public int Runs = 3;
		public int Width = 400;
		public int Height = 400;
		public string InputFile = String.Empty;
		public string UniqueId = String.Empty;
		public string Name = String.Empty;

		public static string ItemsDirectory = "perf-suite-set";

		public string FullFileName { 
			get { 
				return String.Format ("{0}/{1}", ItemsDirectory, InputFile);
			}
		}

		/* CONSTRUCTOR */
		public DrtItem (XmlNode node)
		{
			if (node.Attributes ["startTime"] != null)
				StartTime = Convert.ToInt32 (node.Attributes ["startTime"].Value);

			if (node.Attributes ["endTime"] != null)
				EndTime = Convert.ToInt32 (node.Attributes ["endTime"].Value);

			if (node.Attributes ["interval"] != null)
				Interval = Convert.ToInt32 (node.Attributes ["interval"].Value);

			if (node.Attributes ["inputFile"] != null)
				InputFile = node.Attributes ["inputFile"].Value;

			if (node.Attributes ["uniqueId"] != null)
				UniqueId = node.Attributes ["uniqueId"].Value;

			if (node.Attributes ["name"] != null)
				Name = node.Attributes ["name"].Value;
		
			if (node.Attributes ["timeout"] != null)
				Timeout = Convert.ToInt32 (node.Attributes ["timeout"].Value);

			if (node.Attributes ["runs"] != null)
				Runs = Convert.ToInt32 (node.Attributes ["runs"].Value);

			if (node.Attributes ["width"] != null)
				Width = Convert.ToInt32 (node.Attributes ["width"].Value);

			if (node.Attributes ["height"] != null)
				Height = Convert.ToInt32 (node.Attributes ["height"].Value);
		}

		public bool IsValid ()
		{
			if (StartTime < 0)
				return false;

			if (EndTime < 0)
				return false;

			if (EndTime < StartTime)
				return false;

			if (InputFile == String.Empty)
				return false;

			if (UniqueId == String.Empty)
				return false;

			if (Interval < 0)
				return false;

			if (Name == String.Empty)
				return false;

			if (Timeout < 1000)
				return false;

			if (Width < 1 || Height < 1)
				return false;

			if (Runs < 1)
				return false;

			return true;
		}

		public DrtResult Run ()
		{
			DrtResult result = null;
			Process proc = new Process ();

			try {
				string tmpFileName = Path.GetTempFileName ();

				string arguments = String.Format ("-f \"{0}\" -s \"{1}\" -e \"{2}\" -i \"{3}\" -n \"{4}\" -r \"{5}\" -t \"{6}\" -w \"{7}\" -h \"{8}\"", 
								  FullFileName, 
								  StartTime, 
								  EndTime, 
								  Interval, 
								  Runs, 
							 	  tmpFileName, 
								  Timeout, 
								  Width, 
								  Height);

				proc.EnableRaisingEvents = false; 
				proc.StartInfo.FileName = "perf-tool";
				proc.StartInfo.Arguments = arguments;
				proc.Start();
				proc.WaitForExit();

				if (proc.ExitCode == 0)
					result = new DrtResult (tmpFileName);
			} catch {
				result = null;
			} finally {
				proc.Close ();
				proc.Dispose ();
			}

			return result;
		}

		public override string ToString ()
		{
			return String.Format ("{0} - {1}", UniqueId, Name);
		}

	}

}


