//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
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
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using System.Windows.Markup;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows
{
	[TestClass]
	public class LoadedTest : SilverlightTest
	{
		public class MyComboBox : ComboBox
		{
			public bool AppliedTemplate { get; private set; }

			public override void OnApplyTemplate ()
			{
				AppliedTemplate = true;
				base.OnApplyTemplate ();
			}
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void AddThenRemove ()
		{
			int b_count = 0;
			int c_count = 0;

			Canvas c = new Canvas ();
			MyComboBox b = new MyComboBox ();
			MyComboBox second = new MyComboBox ();

			b.Loaded += delegate { b_count++; };
			c.Loaded += delegate { c_count++; };
			
			c.Children.Add (b);

			TestPanel.Children.Add (c);
			EnqueueConditional (() => b_count == 1 && c_count == 1, "#1");

			// If an item with delayed loaded emission is added to the tree, the entire tree
			// will be walked to find Loaded handlers which have not been emitted and will emit
			// those. The handlers will be invoked the tick *after* the template is applied.
			Enqueue (() => {
				b.Loaded += delegate { b_count += 10; };
				c.Loaded += delegate { c_count += 10; };
				
				c.Children.Add (second);
				Assert.IsFalse (second.AppliedTemplate, "#AppliedTemplate 1");
			});
			Enqueue (() => {
				Assert.IsTrue (second.AppliedTemplate, "#AppliedTemplate 2");
				Assert.AreEqual (1, b_count, "#2");
				Assert.AreEqual (1, c_count, "#3");
			});

			// Adding an item with immediate loaded emission doesn't invoke new
			// Loaded handlers on its parent UIElements
			Enqueue (() => {
				Assert.AreEqual (11, b_count, "#4");
				Assert.AreEqual (11, c_count, "#5");

				b.Loaded += delegate { b_count += 100; };
				c.Loaded += delegate { c_count += 100; };
				c.Children.Add (new Canvas ());
			});
			Enqueue (() => { });
			Enqueue (() => { });
			Enqueue (() => {
				Assert.AreEqual (11, b_count, "#4");
				Assert.AreEqual (11, c_count, "#5");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void AsyncLoaded ()
		{
			// Attach the handler before adding the control to the tree.
			bool loaded = false;
			Border b = new Border ();

			b.Loaded += delegate { loaded = true; };
			TestPanel.Children.Add (b);

			// Loaded is not raised synchronously
			Assert.IsFalse (loaded, "Not syncronous");

			EnqueueConditional (() => loaded, "Loaded should be raised");
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[ExpectedException (typeof (Exception))]
		public void AsyncLoaded2 ()
		{
			// Attach the handler after the control is added to the tree
			bool loaded = false;
			Border b = new Border ();

			TestPanel.Children.Add (b);
			b.Loaded += delegate { loaded = true; };

			// The handler is never invoked - this is a standard async event.
			Assert.IsFalse (loaded, "Not syncronous");

			EnqueueConditional (() => loaded, TimeSpan.FromSeconds (1), "This should time out");
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ControlLoaded ()
		{
			bool loaded = false;
			ItemsControl c = new ItemsControl ();
			c.Loaded += delegate { loaded = true; };
			TestPanel.Children.Add (c);
			EnqueueConditional (() => loaded);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[ExpectedException (typeof (Exception))]
		public void ControlLoaded2 ()
		{
			bool loaded = false;
			ItemsControl c = new ItemsControl ();
			TestPanel.Children.Add (c);
			c.Loaded += delegate { loaded = true; };
			EnqueueConditional (() => loaded);
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void DefaultStyleApply ()
		{
			ComboBox box = new ComboBox ();
			Assert.IsNull (box.Style, "#1");
			Assert.IsNull (box.Template, "#2");
			TestPanel.Children.Add (box);
			Assert.IsNull (box.Style, "#3");
			Assert.IsNotNull (box.Template, "#4");
			Assert.IsUnset (box, Control.TemplateProperty, "#5");

			Enqueue (() => {
				Assert.IsNull (box.Style, "#5");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void EmitBeforeAndAfter ()
		{
			// Adding a templated control to the visual tree results in a second Loaded
			// being emitted *after* the template expands for that control.
			bool before_b = false;
			bool before_c = false;
			bool after_b = false;  
			bool after_c = false;

			Canvas c = new Canvas ();
			ComboBox b = new ComboBox ();
			c.Children.Add (b);

			c.Loaded += delegate { before_c = true; };
			b.Loaded += delegate { before_b = true; };
			TestPanel.Children.Add (c);
			c.Loaded += delegate { after_c = true; };
			b.Loaded += delegate { after_b = true; };

			Enqueue (() => {
				Assert.IsTrue (before_b, "#1");
				Assert.IsTrue (before_c, "#2");
				Assert.IsFalse (after_b, "#3");
				Assert.IsFalse (after_c, "#4");
			});
			Enqueue (() => {
				Assert.IsTrue (before_b, "#3");
				Assert.IsTrue (before_c, "#4");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void EmitBeforeAndAfter2 ()
		{
			// If we call ApplyTemplate (), the second set of Loaded
			// events is emitted immediately.
			bool before_b = false;
			bool before_c = false;
			bool after_b = false;
			bool after_c = false;

			Canvas c = new Canvas ();
			ComboBox b = new ComboBox ();
			c.Children.Add (b);

			c.Loaded += delegate { before_c = true; };
			b.Loaded += delegate { before_b = true; };
			TestPanel.Children.Add (c);
			c.Loaded += delegate { after_c = true; };
			b.Loaded += delegate { after_b = true; };
			b.ApplyTemplate ();

			Enqueue (() => {
				Assert.IsTrue (before_b, "#1");
				Assert.IsTrue (before_c, "#2");
				Assert.IsTrue (after_b, "#3");
				Assert.IsTrue (after_c, "#4");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void LoadedOrdering ()
		{
			List<string> ordering = new List<string> ();
			RoutedEventHandler loadedHandler = (o, e) => ordering.Add (((FrameworkElement) o).Name);

			Canvas a = new Canvas { Name = "a" };
			Canvas a1 = new Canvas { Name = "a1" };
			a.Children.Add (a1);

			Canvas b = new Canvas { Name = "b" };
			Canvas b1 = new Canvas { Name = "b1" };
			b.Children.Add (b1);

			a.Loaded += loadedHandler;
			a1.Loaded += loadedHandler;
			b.Loaded += loadedHandler;
			b1.Loaded += loadedHandler;

			TestPanel.Children.Add (a);
			a1.Children.Add (b);

			Enqueue (() => {
				Assert.AreEqual (4, ordering.Count, "#1");
				Assert.AreEqual ("a1", ordering [0], "#2");
				Assert.AreEqual ("a", ordering [1], "#3");
				Assert.AreEqual ("b1", ordering [2], "#4");
				Assert.AreEqual ("b", ordering [3], "#5");

			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[ExpectedException (typeof (Exception))]
		public void LoadedA1 ()
		{
			// Adding the handler after adding the item to the panel will
			// prevent the handler being invoked
			var box = new ItemsControl ();
			TestPanel.Children.Add (box);
			EnqueueWaitLoaded (box, "#1");
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void LoadedA2 ()
		{
			// If the applied default style contains no 'template', the loaded
			// handler is invoked inside ElementAdded as a regular async event
			var box = new ItemsControl ();
			EnqueueWaitLoaded (box, "#1");
			TestPanel.Children.Add (box);
			Enqueue (() => Assert.IsNull (box.Template, "#2"));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[ExpectedException (typeof (Exception))]
		public void LoadedA3 ()
		{
			// A user set Template should result in standard behaviour
			ItemsControl c = (ItemsControl) XamlReader.Load (@"
<ItemsControl
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<ItemsControl.Template>
		<ControlTemplate>
			<Grid />
		</ControlTemplate>
	</ItemsControl.Template>
</ItemsControl>
");
			TestPanel.Children.Add (c);
			EnqueueWaitLoaded (c, "#1");
			EnqueueTestComplete ();
		}


		[TestMethod]
		[Asynchronous]
		public void LoadedB1 ()
		{
			// The ComboBox default style contains a Template, therefore we delay
			// the WalkTreeForLoaded call until after the default style is applied
			// and then do our tree walk. This allows us to add a handler *after*
			// the element is added to the tree and still have it invoked
			var box = new ComboBox ();
			TestPanel.Children.Add (box);
			EnqueueWaitLoaded (box, "#1");
			Enqueue (() => Assert.IsNotNull (box.Template, "#2"));
			Enqueue (() => Assert.IsUnset (box, Control.TemplateProperty, "#3"));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[ExpectedException (typeof (Exception))]
		public void LoadedB2 ()
		{
			// If we explicitly put in 'null' as the template, it overrides the one
			// provided in the default style, so we treat this as an untemplated
			// item and do the regular treewalk.
			var box = new ComboBox ();
			box.Template = null;
			TestPanel.Children.Add (box);
			EnqueueWaitLoaded (box, "#1");
			Enqueue (() => Assert.IsNotNull (box.Template, "#2"));
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[ExpectedException (typeof (Exception))]
		[MoonlightBug]
		public void LoadedB3 ()
		{
			// A user set Template should result in standard behaviour
			ComboBox c = (ComboBox) XamlReader.Load (@"
<ComboBox
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<ComboBox.Template>
		<ControlTemplate>
			<Grid />
		</ControlTemplate>
	</ComboBox.Template>
</ComboBox>
");
			TestPanel.Children.Add (c);
			EnqueueWaitLoaded (c, "#1");
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void LoadedC1 ()
		{
			List<string> ordering = new List<string> ();
			Canvas container = new Canvas ();

			Canvas c = new Canvas ();
			ComboBox b = new ComboBox ();
			b.Loaded += delegate { ordering.Add ("box"); };
			c.Loaded += delegate { ordering.Add ("canvas"); };

			EnqueueWaitLoaded (container, "#1");
			TestPanel.Children.Add (container);

			Enqueue (() => {
				TestPanel.Children.Add (b);
				container.Children.Add (c);
			});

			EnqueueWaitLoaded (b, "#2");
			EnqueueWaitLoaded (c, "#3");

			Enqueue (() => {
				Assert.AreEqual ("box", ordering [0], "#4");
				Assert.AreEqual ("canvas", ordering [1], "#5");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void LoadedC2 ()
		{
			List<string> ordering = new List<string> ();

			Canvas container = new Canvas ();
			Canvas last = new Canvas ();
			Canvas first = new Canvas ();
			ComboBox box = new ComboBox ();


			container.Loaded += (o, e) => ordering.Add ("container");
			last.Loaded += (o, e) => ordering.Add ("last");
			first.Loaded += (o, e) => ordering.Add ("first");
			box.Loaded += (o, e) => ordering.Add ("box");
			container.Children.Add (box);

			TestPanel.Children.Add (first);
			TestPanel.Children.Add (container);
			TestPanel.Children.Add (last);

			EnqueueWaitLoaded (container, "#1");
			EnqueueWaitLoaded (box, "#2");
			Enqueue (() => {
				Assert.AreEqual ("first", ordering [0], "#3");
				Assert.AreEqual ("box", ordering [1], "#4");
				Assert.AreEqual ("container", ordering [2], "#5");
				Assert.AreEqual ("last", ordering [3], "#6");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void LoadedC3 ()
		{
			Canvas c = new Canvas ();
			ComboBox b = new ComboBox ();
			c.Children.Add (b);

			TestPanel.Children.Add (c);
			EnqueueWaitLoaded (c, "#1");
			EnqueueWaitLoaded (b, "#2");
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void LoadedD ()
		{
			List<string> ordering = new List<string> ();
			Canvas c = new Canvas ();
			ComboBox b = new ComboBox ();

			c.Loaded += delegate { ordering.Add ("canvas"); };
			b.Loaded += delegate { ordering.Add ("box"); };

			EnqueueWaitLoaded (c, "#1");
			EnqueueWaitLoaded (b, "#2");

			TestPanel.Children.Add (b);
			TestPanel.Children.Add (c);

			Enqueue (() => {
				Assert.AreEqual ("box", ordering [0], "#4");
				Assert.AreEqual ("canvas", ordering [1], "#3");
			});
			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		[MoonlightBug]
		public void LoadedSeveralTimes ()
		{
			bool loaded = false;
			TestPanel.Loaded += delegate { loaded = true; };
			TestPanel.Children.Add (new ComboBox ());
			Enqueue (() => {
				Assert.IsFalse (loaded);
			});
			Enqueue (() => {
				Assert.IsTrue (loaded);
			});
			EnqueueTestComplete ();
		}
	}
}
