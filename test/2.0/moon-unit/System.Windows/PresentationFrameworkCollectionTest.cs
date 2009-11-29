using System;
using System.Collections;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Shapes;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class PresentationFrameworkCollectionTest
	{
		[TestMethod]
		public void RemoveAt ()
		{
			var canvas = new Canvas ();
			var children = canvas.Children;

			children.Add (new Rectangle ());
			children.RemoveAt (0);

			Assert.Throws<ArgumentOutOfRangeException> (() => children.RemoveAt (1));
		}

		[TestMethod]
		public void ArgumentChecks ()
		{
			Canvas canvas = new Canvas ();
			UIElementCollection c = canvas.Children;

			Assert.Throws<ArgumentNullException> (() => c.Add (null), "Add");
			Assert.Throws<ArgumentNullException> (() => c.Insert (0, null), "Insert null");
			Assert.Throws<ArgumentOutOfRangeException> (() => c.Insert (-1, new Rectangle ()), "Insert negative");
			Assert.Throws<ArgumentOutOfRangeException> (() => c.RemoveAt (-1), "RemoveAt negative");
			Assert.Throws<ArgumentOutOfRangeException> (() => Console.WriteLine (c [-1] == null), "this getter");
			Assert.Throws<ArgumentOutOfRangeException> (() => c [-1] = new Rectangle (), "this setter");

			Assert.IsFalse (c.Contains (null), "Contains");
			Assert.IsFalse (c.Remove (null), "Remove");

			Assert.AreEqual (-1, c.IndexOf (null), "IndexOf");
		}

		[TestMethod]
		public void NonGenericEnumeratorDisposable ()
		{
			Canvas canvas = new Canvas ();
			UIElementCollection c = canvas.Children;
			Assert.IsTrue ((c as IList).GetEnumerator () is IDisposable);
		}
	}
}
