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
using System.IO;
using System.Net;
using System.Xml;
using Gtk;
using Gtk.Moonlight;
using System.Collections.Generic;
using System.Threading;

namespace GnomeOrbiter {

	public class PlanetFetcher {

		private Thread fetchThread;
		private List <EntryData> entryList;

		public event EventHandler FetchDone;

		public List <EntryData> List {
			get { return entryList; }
		}

		/* CONSTRUCTOR */
		public PlanetFetcher ()
		{
			fetchThread = new Thread (FetchPlanetItems);
			entryList = new List <EntryData> ();
		}

		public void Start ()
		{
			fetchThread.Start ();
		}

		public void FetchPlanetItems ()
		{
			try {
				WebRequest request = WebRequest.Create ("http://planet.gnome.org/atom.xml");
				Stream stream = request.GetResponse ().GetResponseStream ();
				XmlDocument document = new XmlDocument ();
				document.Load (stream);

				foreach (XmlNode node in document.GetElementsByTagName ("entry")) {
					EntryData post = new EntryData (node);
					if (post.IsComplete) 
						entryList.Add (post);
				}

				GLib.Idle.Add (FetchDoneClosure);
			} catch {
			}
		}

		private bool FetchDoneClosure ()
		{
			Gdk.Threads.Enter ();

			if (FetchDone != null)
				FetchDone (this, null);

			Gdk.Threads.Leave ();

			return false;
		}

	}

}
