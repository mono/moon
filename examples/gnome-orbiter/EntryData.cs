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
using Gtk.Moonlight;

namespace GnomeOrbiter {

	public class EntryData {

		string title;
		string summary;
		string author;

		public bool IsComplete {
			get { return (summary != String.Empty &&
						  author != String.Empty  &&
						  title != String.Empty); 
			}
		}
		
		/* CONSTRUCTOR */
		public EntryData (XmlNode node)
		{
			title = node ["title"].InnerText;
			if (node ["author"] != null && node ["author"] ["name"] != null)
				author = node ["author"] ["name"].InnerText;
			else
				author = String.Empty;

			if (node ["summary"] != null) {
				summary = node ["summary"].InnerText;
				summary = summary.Replace ("\n", "");
				summary = summary.Replace ("\r", "");
				
				if (summary.Length > 100) 
					summary = summary.Substring (0, 100);

				summary = summary.Trim ();

				summary += "...";
			} else 
				summary = String.Empty;
		}

		public void PrintToConsole ()
		{
			Console.WriteLine ("* {0}", title);
			Console.WriteLine ("  {0}\n", summary);
		}

		public Entry ToEntryElement (GtkSilver silver)
		{
			Entry e = new Entry (silver);
			e.Header = String.Format ("{0}: {1}", author, title);
			e.Text = summary;

			return e;
		}

	}

}


