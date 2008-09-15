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
using System.Xml;
using System.Collections.Generic;

namespace PerfSuiteRunner {

	public class DrtStore {

		public List <DrtItem> Items = new List <DrtItem> ();

		/* CONSTRUCTOR */
		public DrtStore (string fileName)
		{
			int count = 0;
			Console.WriteLine ("*** Loading drt from file {0}", fileName);

			try {
				using (StreamReader streamReader = File.OpenText (fileName)) {
					XmlDocument document = new XmlDocument ();
					document.Load (streamReader);
                
					foreach (XmlNode node in document.GetElementsByTagName ("DrtItem")) {
						DrtItem item = new DrtItem (node);
						if (item.IsValid () == false) {
							string error = String.Format ("Drt Item with id {0} is invalid!", item.UniqueId);
							throw new Exception (error);
						} else {
							Items.Add (item);
							count++;
						}
					}
				}

			} catch (Exception e) { 
				throw new Exception ("Error while loading drt list", e);
			}

			Console.WriteLine ("*** Loaded {0} items from drt list", count);
		}

		public DrtItem GetDrtItemForId (string id)
		{
			foreach (DrtItem item in Items) {
				if (item.UniqueId == id)
					return item;
			}

			return null;
		}

	}

}


