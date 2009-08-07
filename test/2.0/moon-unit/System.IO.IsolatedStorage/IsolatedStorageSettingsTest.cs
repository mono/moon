//
// Unit tests for System.IO.IsolatedStorage.IsolatedStorageSettings
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2008 Novell, Inc (http://www.novell.com)
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
using System.IO.IsolatedStorage;
using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.IO.IsolatedStorage {

	[TestClass]
	public class IsolatedStorageSettingsTest {

		void CheckICollection (IsolatedStorageSettings settings)
		{
			ICollection c = (settings as ICollection);
			Assert.AreEqual (0, c.Count, "ICollection.Count");
			Assert.IsFalse (c.IsSynchronized, "ICollection.IsSynchronized");
			Assert.IsNotNull (c.SyncRoot, "ICollection.SyncRoot");
			Assert.IsNotNull (c.GetEnumerator (), "ICollection.GetEnumerator");
		}

		void CheckICollectionKeyPairValue (IsolatedStorageSettings settings)
		{
			ICollection<KeyValuePair<string, object>> c = (settings as ICollection<KeyValuePair<string, object>>);
			Assert.AreEqual (0, c.Count, "Count");
			Assert.IsFalse (c.IsReadOnly, "IsReadOnly");
			Assert.IsNotNull (c.GetEnumerator (), "GetEnumerator");

			KeyValuePair<string,object> kvp = new KeyValuePair<string,object> ("key", "value");
			c.Add (kvp);
			Assert.AreEqual (1, c.Count, "Add/Count");
			Assert.Throws (delegate { c.Add (new KeyValuePair<string, object> (null, "value")); }, typeof (ArgumentNullException), "Add(KVP(null))");
			Assert.Throws (delegate { c.Add (new KeyValuePair<string, object> ("key", "value")); }, typeof (ArgumentException), "Add(twice)");

			Assert.IsTrue (c.Contains (kvp), "Contains(kvp)");
			Assert.IsTrue (c.Contains (new KeyValuePair<string, object> ("key", "value")), "Contains(new)");
			Assert.IsFalse (c.Contains (new KeyValuePair<string, object> ("value", "key")), "Contains(bad)");

			c.Remove (kvp);
			Assert.IsFalse (c.Contains (kvp), "Remove/Contains(kvp)");
			Assert.AreEqual (0, c.Count, "Remove/Count");

			c.Add (kvp);
			c.Clear ();
			Assert.AreEqual (0, c.Count, "Clear/Count");
		}

		void CheckIDictionary (IsolatedStorageSettings settings)
		{
			IDictionary d = (settings as IDictionary);
			Assert.IsFalse (d.IsFixedSize, "Empty-IsFixedSize");
			Assert.IsFalse (d.IsReadOnly, "Empty-IsReadOnly");

			object key = new object ();

			d.Add ("key", "string");
			Assert.AreEqual (1, d.Count, "Add/Count");
			Assert.Throws (delegate { d.Add (key, "object"); }, typeof (ArgumentException), "Add(object)");
			Assert.Throws (delegate { d.Add (null, "null"); }, typeof (ArgumentNullException), "Add(null)");
			Assert.Throws (delegate { d.Add ("key", "another string"); }, typeof (ArgumentException), "Add(twice)");

			d.Remove ("value");
			Assert.AreEqual (1, d.Count, "Remove/Bad/Count");
			d.Remove ("key");
			Assert.AreEqual (0, d.Count, "Remove/Count");
			d.Remove (key); // no exception
			Assert.Throws (delegate { d.Remove (null); }, typeof (ArgumentNullException), "Remove(null)");

			d.Add ("key", null);
			Assert.AreEqual (1, d.Count, "Add2/Count");
			Assert.IsTrue (d.Contains ("key"), "Contains(key)");
			Assert.IsFalse (d.Contains (key), "Contains(object)");
			Assert.Throws (delegate { d.Contains (null); }, typeof (ArgumentNullException), "Contains(null)");

			Assert.IsNull (d ["key"], "this[key]"); // since we added null :)
			Assert.IsNull (d [key], "this[object]");
			Assert.Throws (delegate { d[null].ToString (); }, typeof (ArgumentNullException), "d[null] get");

			d ["key"] = "value"; // replace
			Assert.AreEqual (1, d.Count, "Replace/Count");
			Assert.Throws (delegate { d [key] = key; }, typeof (ArgumentException), "d[key] set");
			Assert.Throws (delegate { d [null] = null; }, typeof (ArgumentNullException), "d[null] set");

			d.Clear ();
			Assert.AreEqual (0, d.Count, "Clear/Count");
		}

		void CheckIDictionaryStringObject (IsolatedStorageSettings settings)
		{
			IDictionary<string, object> d = (settings as IDictionary<string, object>);
			Assert.AreEqual (0, d.Keys.Count, "Keys.Count");
			Assert.AreEqual (0, d.Values.Count, "Values.Count");
		}

		public void CheckSettings (IsolatedStorageSettings settings)
		{
			Assert.AreEqual (0, settings.Count, "Empty-Count");
			Assert.AreEqual (0, settings.Keys.Count, "Empty-Keys.Count");
			Assert.AreEqual (0, settings.Values.Count, "Empty-Values.Count");

			settings.Add ("key", "value");
			Assert.Throws (delegate { settings.Add (null, "x"); }, typeof (ArgumentNullException), "Add(null,x)");
			Assert.Throws (delegate { settings.Add ("key", "another string"); }, typeof (ArgumentException), "Add(twice)");

			Assert.AreEqual (1, settings.Count, "Count");
			Assert.AreEqual (1, settings.Keys.Count, "Keys.Count");
			Assert.AreEqual (1, settings.Values.Count, "Values.Count");
			Assert.AreEqual (1, (settings as ICollection).Count, "ICollection.Count");

			Assert.IsTrue (settings.Contains ("key"), "Contains-key");
			Assert.IsFalse (settings.Contains ("value"), "Contains-value");
			Assert.Throws (delegate { settings.Contains (null); }, typeof (ArgumentNullException), "Contains(null)");

			Assert.AreEqual ("value", settings ["key"], "this[key]");
			settings ["key"] = null;
			Assert.IsNull (settings ["key"], "this[key]-null");
			Assert.Throws (delegate { Console.WriteLine (settings ["unexisting"]); }, typeof (KeyNotFoundException), "this[unexisting]");
			Assert.Throws (delegate { settings [null] = null; }, typeof (ArgumentNullException), "this[null] set");

			settings.Remove ("key");
			Assert.AreEqual (0, settings.Count, "Remove/Count");
			Assert.IsFalse (settings.Remove ("unexisting"), "Remove(unexisting)");
			Assert.Throws (delegate { settings.Remove (null); }, typeof (ArgumentNullException), "Remove(null)");

			settings.Add ("key", "value");
			Assert.AreEqual (1, settings.Count, "Add2/Count");

			string s;
			Assert.IsTrue (settings.TryGetValue<string> ("key", out s), "TryGetValue(key)");
			Assert.AreEqual ("value", s, "out value");
			object o;
			Assert.IsTrue (settings.TryGetValue<object> ("key", out o), "TryGetValue(object)");
			Assert.AreEqual ("value", s, "out value/object");
			Assert.IsFalse (settings.TryGetValue<string> ("value", out s), "TryGetValue(value)");
			Assert.Throws (delegate { settings.TryGetValue<string> (null, out s); }, typeof (ArgumentNullException), "TryGetValue(null)");

			settings.Clear ();
			Assert.AreEqual (0, settings.Count, "Clear/Count");
		}

		private void CheckAll (IsolatedStorageSettings settings)
		{
			settings.Clear ();
			try {
				CheckSettings (settings);
				CheckICollection (settings);
				CheckICollectionKeyPairValue (settings);
				CheckIDictionary (settings);
				CheckIDictionaryStringObject (settings);
			}
			finally {
				settings.Clear ();
			}
		}

		[TestMethod]
		public void ApplicationSettingsTest ()
		{
			// Fails in Silverlight 3
			CheckAll (IsolatedStorageSettings.ApplicationSettings);
		}

		[TestMethod]
		public void SiteSettingsTest ()
		{
			CheckAll (IsolatedStorageSettings.SiteSettings);
		}
	}
}
