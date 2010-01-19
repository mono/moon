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

		void AddThenRemove_test (bool match_tick_numbers)
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

				if (match_tick_numbers) {
					// in moonlight the c.Children.Add posts the
					// loaded event emission hook for async use, but the
					// render callback is invoked right after the Enqueue
					// which happens from a DispatcherTimer (an
					// animation clock).

					// therefore the properties are updated after the
					// previous Enqueue, but before this one.
					//
					Assert.AreEqual (1, b_count, "#2");
					Assert.AreEqual (1, c_count, "#3");
				}
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
			Enqueue (() => {
				Assert.AreEqual (11, b_count, "#6");
				Assert.AreEqual (11, c_count, "#7");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void AddThenRemove ()
		{
			AddThenRemove_test (false);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("see comment in AddThenRemove_test")]
		public void AddThenRemove_strictOrdering ()
		{
			AddThenRemove_test (true);
		}

		[TestMethod]
		[Asynchronous]
		public void AddTwice ()
		{
			AddTwice_test (false);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("see comment in AddThenRemove_test")]
		public void AddTwice_strictOrdering ()
		{
			AddTwice_test (true);
		}

		public void AddTwice_test (bool match_tick_numbers)
		{
			// If we add a handler both before and after we add a templated item
			// to the tree, check to ensure that both handlers get called immediately
			// if the element is removed and added again.
			int before = 0;
			int after = 0;
			ComboBox box = new ComboBox ();
			box.Loaded += delegate { before++; };
			TestPanel.Children.Add (box);
			box.Loaded += delegate { after++; };
			Enqueue (() => {
				// The first handler should have emitted now.
				Assert.AreEqual (1, before, "#1");

				if (match_tick_numbers) {
					// The template has been applied this tick, so the
					// second handler will invoke during the next tick
					Assert.AreEqual (0, after, "#2");
				}
			});
			Enqueue (() => {
				Assert.AreEqual (1, before, "#3");
				Assert.AreEqual (1, after, "#4");

				// Both handlers should emit again now.
				TestPanel.Children.Clear ();
				TestPanel.Children.Add (box);
			});
			Enqueue (() => {
				Assert.AreEqual (2, before, "#5");
				Assert.AreEqual (2, after, "#6");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
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
		public void ChildForcesParentToEmitLoad ()
		{
			ChildForcesParentToEmitLoad_test (false);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("see comment in AddThenRemove_test")]
		public void ChildForcesParentToEmitLoad_strictOrdering ()
		{
			ChildForcesParentToEmitLoad_test (true);
		}

		public void ChildForcesParentToEmitLoad_test (bool match_tick_numbers)
		{
			bool loaded = false;
			TestPanel.Loaded += delegate { loaded = true; };
			TestPanel.Children.Add (new ComboBox ());
			Enqueue (() => {
				if (match_tick_numbers)
					Assert.IsFalse (loaded, "#1");
			});
			Enqueue (() => {
				// When the ComboBoxs Template expands, it forces the 
				// canvas to call any unemitted Loaded handlers.
				Assert.IsTrue (loaded, "#2");
			});
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
		public void EmitBeforeAndAfter ()
		{
			EmitBeforeAndAfter_test (false);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("see comment in AddThenRemove_test")]
		public void EmitBeforeAndAfter_strictOrdering ()
		{
			EmitBeforeAndAfter_test (true);
		}

		public void EmitBeforeAndAfter_test (bool match_tick_numbers)
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
				if (match_tick_numbers) {
					Assert.IsFalse (after_b, "#3");
					Assert.IsFalse (after_c, "#4");
				}
			});
			Enqueue (() => {
				Assert.IsTrue (after_b, "#3");
				Assert.IsTrue (after_c, "#4");
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
			
			// Calling 'ApplyTemplate' emits the Loaded
			// events for the template, so all handlers will
			// be raised during the next tick.
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
		[MoonlightBug ("we don't emit Loaded events in insertion order")]
		public void ForceEventEmission ()
		{
			// If we add a control which does *not* have a loaded handler
			// attached, then we don't force emission of other Loaded handlers in
			// the order in which they're added.
			ForceEventEmissionImpl (false);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("we don't emit Loaded events in insertion order")]
		public void ForceEventEmission2 ()
		{
			// If we add a control which *does* have a loaded handler
			// attached, then we force emission of other Loaded handlers in
			// the order in which they're added.
			ForceEventEmissionImpl (true);
		}

		public void ForceEventEmissionImpl (bool attachLoadedHandler)
		{

			string loaded = "";
			Canvas box = new Canvas ();// ComboBox box = new ComboBox { Style = null, Template = null };
			Canvas container = new Canvas ();

			Canvas main = new Canvas ();
			Canvas child = new Canvas ();
			Canvas baby = new Canvas ();

			main.Children.Add (child);
			child.Children.Add (baby);

			EnqueueWaitLoaded (container, "#1");
			EnqueueWaitLoaded (main, "#2");

			TestPanel.Children.Add (container);
			TestPanel.Children.Add (main);

			Enqueue (() => {
				// NOTE: The normal order of event emission is bottom->top but in
				// this case the handlers are emitted in the order they're added
				baby.Loaded += delegate { loaded += "baby"; };
				main.Loaded += delegate { loaded += "main"; };
				child.Loaded += delegate { loaded += "child"; };

				if (attachLoadedHandler)
					box.Loaded += delegate { };

				container.Children.Add (box);

				child.Loaded += delegate { loaded += "baby2"; };
				main.Loaded += delegate { loaded += "main2"; };
				baby.Loaded += delegate { loaded += "child2"; };
			});
			Enqueue (() => {
				Assert.AreEqual (attachLoadedHandler ? "babymainchild" : "", loaded, "#3");
			});
			Enqueue (() => {
				Assert.AreEqual (attachLoadedHandler ? "babymainchild" : "", loaded, "#4");
				//And force the second set
				loaded = "";
				Canvas c = new Canvas ();
				c.Loaded += delegate { };
				TestPanel.Children.Add (c);
			});
			Enqueue (() => {
				// If the first element had a Loaded handler attached, then we should only get the second set of handlers
				// Otherwise we get both sets.
				Assert.AreEqual (attachLoadedHandler ? "baby2main2child2" : "babymainchildbaby2main2child2", loaded, "#5");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void ForceEventEmission3 ()
		{
			// Add a canvas to the tree and wait for it to load.
			// Add a second Loaded handler which won't be invoked
			// initially and then poke it til it invokes.

			int count = 0;
			Canvas c = new Canvas ();
			EnqueueWaitLoaded (c, "#1");
			TestPanel.Children.Add (c);

			c.Loaded += delegate { count ++;};

			// Second handler hasn't invoked
			Enqueue (() => Assert.AreEqual (0, count, "#2"));

			Enqueue (() => {
				// This should not cause the second handler to invoke
				Assert.AreEqual (0, count, "#3");
				c = new Canvas ();
				TestPanel.Children.Add (c);
			});
			Enqueue (() => {
				// This will cause it to be invoked
				Assert.AreEqual (0, count, "#4");
				c = new Canvas ();
				c.Loaded += delegate { };
				TestPanel.Children.Add (c);
			});
			Enqueue (() => Assert.AreEqual (1, count, "#5"));
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
			bool loaded = false;
			c.Loaded += (o,e) => { loaded = true; };
			TestPanel.Children.Add (c);
			Assert.IsFalse (loaded, "#1");
 			EnqueueWaitLoaded (c, "#2");
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
		public void RemoveAfterAdded ()
		{
			// See what happens if we add/remove the same element
			// multiple times in a row.
			int loaded = 0;
			ComboBox box = new ComboBox ();

			box.Loaded += delegate { loaded++; };
			TestPanel.Children.Add (box);
			TestPanel.Children.Clear ();
			TestPanel.Children.Add (box);
			TestPanel.Children.Clear ();
			TestPanel.Children.Add (box);

			Enqueue (() => {
				Assert.AreEqual (3, loaded, "#1");
			});
			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveAfterTemplateLoaded ()
		{
			RemoveAfterTemplateLoaded_test (false);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("see comment in AddThenRemove_test")]
		public void RemoveAfterTemplateLoaded_strictOrdering ()
		{
			RemoveAfterTemplateLoaded_test (true);
		}

		public void RemoveAfterTemplateLoaded_test (bool match_tick_numbers)
		{
			int before = 0;
			int after = 0;
			ComboBox box = new ComboBox ();

			box.Loaded += delegate { before++; };
			TestPanel.Children.Add (box);
			box.Loaded += delegate { after++; };

			Enqueue (() => {
				Assert.AreEqual (1, before, "#1");
				if (match_tick_numbers) {
					Assert.AreEqual (0, after, "#2");
				}
				box.ApplyTemplate ();
				TestPanel.Children.Clear ();
			});
			Enqueue (() => {
				Assert.AreEqual (1, before, "#3");
				Assert.AreEqual (1, after, "#4");
			});
			Enqueue (() => {
				// Make sure that the values really aren't changing
				Assert.AreEqual (1, before, "#5");
				Assert.AreEqual (1, after, "#6");
			});

			EnqueueTestComplete ();
		}
		
		[TestMethod]
		[Asynchronous]
		public void RemoveBeforeTemplateLoaded ()
		{
			int before = 0;
			int after = 0;
			ComboBox box = new ComboBox ();

			box.Loaded += delegate { before++; };
			TestPanel.Children.Add (box);
			box.Loaded += delegate { after++; };
			TestPanel.Children.Clear ();

			Enqueue (() => {
				Assert.AreEqual (1, before, "#1");
				Assert.AreEqual (0, after, "#2");
			});
			Enqueue (() => {
				// Make sure that the second handler definitely hasn't being called
				Assert.AreEqual (1, before, "#3");
				Assert.AreEqual (0, after, "#4");

				// Apply the template and see if this causes Loaded emission
				Assert.IsTrue (box.ApplyTemplate (), "#5");
			});
			Enqueue (() => {
				Assert.AreEqual (1, before, "#6");
				Assert.AreEqual (0, after, "#7");
			});

			EnqueueTestComplete ();
		}

		[TestMethod]
		[Asynchronous]
		public void GrandChildLoaded ()
		{
			Grid a = new Grid ();
			Grid ab = new Grid ();
			UserControl b = new UserControl ();
			bool a_loaded = false;
			bool b_loaded = false;

			a.Children.Add (ab);
			TestPanel.Children.Add (a);
			a.Loaded += delegate { a_loaded = true; };
			b.Loaded += delegate { b_loaded = true; };
			ab.Children.Add (b);

			EnqueueConditional (() => a_loaded, "Loaded should be raised a");
			EnqueueConditional (() => b_loaded, "Loaded should be raised b");
			EnqueueTestComplete ();
		}

	}
}
