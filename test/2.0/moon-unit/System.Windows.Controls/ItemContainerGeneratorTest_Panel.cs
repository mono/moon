//
// ItemContainerGenerator Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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
using System.Windows.Controls.Primitives;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Windows.Markup;
using Mono.Moonlight.UnitTesting;
using System.Collections.Generic;
using System.Collections.Specialized;

namespace MoonTest.System.Windows.Controls
{
	public class CustomVirtualizingPanel : VirtualizingPanel
	{
		public Action OnClearChildrenAction {
			get; set;
		}

		public Action OnItemsChangedAction {
			get; set;
		}

		protected override void OnClearChildren ()
		{
			if (OnClearChildrenAction != null)
				OnClearChildrenAction ();
			base.OnClearChildren ();
		}

		protected override void OnItemsChanged (object sender, ItemsChangedEventArgs args)
		{
			if (OnItemsChangedAction != null)
				OnItemsChangedAction ();
			base.OnItemsChanged (sender, args);
		}
	}

	[TestClass]
	public partial class IItemContainerGeneratorTest : ItemContaineGeneratorTest_PanelBase {

		CustomVirtualizingPanel Panel {
			get {
				return (CustomVirtualizingPanel) VisualTreeHelper.GetChild (VisualTreeHelper.GetChild (Control, 0), 0);
			}
		}

		[TestMethod]
		[Asynchronous]
		public void AllowStartAtRealized_True ()
		{
			// Create all the containers, then try to create the one at index 0.
			bool fresh;
			object first, second;
			CreateAsyncTest (Control, () => {
				var position = IGenerator.GeneratorPositionFromIndex (0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, true))
					for (int i = 0; i < Control.Items.Count; i++)
						IGenerator.GenerateNext (out fresh);

				first = Generator.ContainerFromIndex (0);
				position = IGenerator.GeneratorPositionFromIndex (0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, true))
					second = IGenerator.GenerateNext (out fresh);
				Assert.AreSame (first, second, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void AllowStartAtUnrealized_False ()
		{
			// Create all the containers, then try to create the one at index 0.
			bool fresh;
			object first, second;
			CreateAsyncTest (Control, () => {
				var position = IGenerator.GeneratorPositionFromIndex (0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, true))
					for (int i = 0; i < Control.Items.Count; i++)
						IGenerator.GenerateNext (out fresh);

				first = Generator.ContainerFromIndex (0);
				position = IGenerator.GeneratorPositionFromIndex (0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false))
					second = IGenerator.GenerateNext (out fresh);

				Assert.AreNotSame (first, second, "#1");
				Assert.AreSame (Generator.ContainerFromIndex (1), second, "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void AllowStartAtUnrealized_False_PositiveOffset ()
		{
			// Create all the containers, then try to create the one at index 0.
			bool fresh;
			object first, second;
			CreateAsyncTest (Control, () => {
				var position = IGenerator.GeneratorPositionFromIndex (0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, true))
					for (int i = 0; i < Control.Items.Count; i++)
						IGenerator.GenerateNext (out fresh);

				first = Generator.ContainerFromIndex (0);
				position = new GeneratorPosition (0, 1);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false))
					second = IGenerator.GenerateNext (out fresh);

				Assert.AreNotSame (first, second, "#1");
				Assert.AreSame (Generator.ContainerFromIndex (1), second, "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void AllowStartAtUnrealized_False_ZeroOffset ()
		{
			// Create all the containers, then try to create the one at index 0.
			bool fresh;
			object first, second;
			CreateAsyncTest (Control, () => {
				var position = IGenerator.GeneratorPositionFromIndex (0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, true))
					for (int i = 0; i < Control.Items.Count; i++)
						IGenerator.GenerateNext (out fresh);

				first = Generator.ContainerFromIndex (0);
				position = new GeneratorPosition (0, 0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false))
					second = IGenerator.GenerateNext (out fresh);

				Assert.AreNotSame (first, second, "#1");
				Assert.AreSame (Generator.ContainerFromIndex (1), second, "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void AllowStartAtUnrealized_False_Backwards ()
		{
			// Create all the containers, then try to create the one at index 0.
			bool fresh;
			object first, second;
			CreateAsyncTest (Control, () => {
				var position = IGenerator.GeneratorPositionFromIndex (0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, true))
					for (int i = 0; i < Control.Items.Count; i++)
						IGenerator.GenerateNext (out fresh);

				first = Generator.ContainerFromIndex (1);
				position = IGenerator.GeneratorPositionFromIndex (1);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Backward, false))
					second = IGenerator.GenerateNext (out fresh);

				Assert.AreNotSame (first, second, "#1");
				Assert.AreSame (Generator.ContainerFromIndex (0), second, "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ContainerFromIndex_ClearHostPanel ()
		{
			// If we clear the panel, then we can no longer link the container to the
			// control. Do we check container.Parent and retrieve the TemplateOwner from there?
			var c = new ItemsControl ();
			c.Items.Add (new object ());

			CreateAsyncTest (c, () => {
				var container = c.ItemContainerGenerator.ContainerFromIndex (0);

				var panel = c.GetChild<ItemsPresenter> (0).GetChild<Panel> (0);
				panel.Children.Clear ();
				Assert.AreSame (container, c.ItemContainerGenerator.ContainerFromIndex (0), "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GeneratedStaggeredTest ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				// Start at 0, generate 1
				var position = IGenerator.GeneratorPositionFromIndex (0);
				Assert.AreEqual (new GeneratorPosition (-1, 1), position, "#1");
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false))
					Assert.IsNotNull (IGenerator.GenerateNext (out fresh), "#2");

				// Start at 2, generate 6
				position = IGenerator.GeneratorPositionFromIndex (2);
				Assert.AreEqual (new GeneratorPosition (0, 2), position, "#3");
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false)) {
					for (int i = 0; i < 100; i++) {
						if (i < 3)
							Assert.IsNotNull (IGenerator.GenerateNext (out fresh), "#4." + i);
						else
							Assert.IsNull (IGenerator.GenerateNext (out fresh), "#4." + i);
					}
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GeneratedStaggeredTest2 ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				// Start at 0, generate 1
				var position = IGenerator.GeneratorPositionFromIndex (0);
				Assert.AreEqual (new GeneratorPosition (-1, 1), position, "#1");
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false))
					Assert.IsNotNull (IGenerator.GenerateNext (out fresh), "#2");

				// Start at 2, and generate 3
				position = new GeneratorPosition (0, 2);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false)) {
					for (int i = 0; i < 3; i++)
						Assert.IsNotNull (IGenerator.GenerateNext (out fresh), "#4." + i);
				}
				for (int i = 0; i < Control.Items.Count; i++)
					if (i == 1)
						Assert.IsNull (Generator.ContainerFromIndex (i), "#5." + i);
					else
						Assert.IsNotNull (Generator.ContainerFromIndex (i), "#5." + i);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GenerateAllTest ()
		{
			bool realised;
			CreateAsyncTest (Control, () => {
				using (var g = IGenerator.StartAt (IGenerator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, true)) {
					// Make all 5
					for (int i = 0; i < Control.Items.Count; i++) {
						var container = IGenerator.GenerateNext (out realised);

						Assert.AreSame (container, Generator.ContainerFromItem (Control.Items [i]), "#1." + i);
						Assert.AreSame (container, Generator.ContainerFromIndex (i), "#2." + i);
						Assert.AreEqual (i, Generator.IndexFromContainer (container), "#3." + i);
					}
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GenerateSameThrice ()
		{
			bool fresh;
			object first;
			object second;
			object third;
			CreateAsyncTest (Control, () => {
				var position = Generator.GeneratorPositionFromIndex (0);
				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false))
					first = IGenerator.GenerateNext (out fresh);
				Assert.IsTrue (fresh, "#1");

				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false))
					second = IGenerator.GenerateNext (out fresh);
				Assert.IsFalse (fresh, "#2");

				using (var g = IGenerator.StartAt (position, GeneratorDirection.Forward, false))
					third = IGenerator.GenerateNext (out fresh);
				Assert.IsFalse (fresh, "#3");

				Assert.IsNotNull (first, "#4");
				Assert.AreSame (first, second, "#5");
				Assert.AreSame (second, third, "#6");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GenerateThree_RemoveMiddle ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				var containers = new List<FrameworkElement> ();
				using (IGenerator.StartAt (new GeneratorPosition (-1, 1), GeneratorDirection.Forward, true)) {
					for (int i = 0; i < 3; i++)
						containers.Add ((FrameworkElement) IGenerator.GenerateNext (out fresh));
					IGenerator.Remove (new GeneratorPosition (1, 0), 1);

					Assert.IsNotNull (Generator.ContainerFromIndex (0), "#1");
					Assert.IsNull (Generator.ContainerFromIndex (1), "#2");
					Assert.IsNotNull (Generator.ContainerFromIndex (2), "#3");
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GenerateThree_RemoveAll ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				var containers = new List<FrameworkElement> ();
				using (IGenerator.StartAt (new GeneratorPosition (-1, 1), GeneratorDirection.Forward, true)) {
					for (int i = 0; i < 3; i++)
						containers.Add ((FrameworkElement) IGenerator.GenerateNext (out fresh));

					// Remove the second one, the third one, then the first one
					IGenerator.Remove (new GeneratorPosition (1, 0), 1);
					IGenerator.Remove (new GeneratorPosition (1, 0), 1);
					IGenerator.Remove (new GeneratorPosition (0, 0), 1);

					for (int i = 0; i < 3; i++)
						Assert.IsNull (Generator.ContainerFromIndex (i), "#" + i);
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GenerateZeroZero ()
		{
			CreateAsyncTest (Control, () => {
				bool fresh;
				Assert.IsNull (Generator.ContainerFromIndex (0), "#1");
				using (IGenerator.StartAt (new GeneratorPosition (0, 0), GeneratorDirection.Forward, true))
					IGenerator.GenerateNext (out fresh);
				Assert.IsNull (Generator.ContainerFromIndex (0), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void GeneratorTypeTest ()
		{
			CreateAsyncTest (Control, () => {
				Assert.IsInstanceOfType<ItemContainerGenerator> (IGenerator, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IndexFromPosition_CrossRealisedTest ()
		{
			// Realise elements 0 and 2 and then try to get the index of an element as if we hadn't
			// realised element 2
			bool realised;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, false))
					IGenerator.GenerateNext (out realised);
				using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (2), GeneratorDirection.Forward, false))
					IGenerator.GenerateNext (out realised);
				
				// Get the index of the 3'rd unrealised element starting at the first realised element
				int index = Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 4));
				Assert.AreEqual (4, index, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IndexFromPosition_LandOnRealisedTest ()
		{
			// Realise elements 0 and 2 and then try to get the index of an element as if we hadn't
			// realised element 2
			bool realised;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, false))
					IGenerator.GenerateNext (out realised);
				using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (2), GeneratorDirection.Forward, false))
					IGenerator.GenerateNext (out realised);

				// Get the index of the 3'rd unrealised element starting at the first realised element
				int index = Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 2));
				Assert.AreEqual (2, index, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IndexFromPosition_NoRealisedTest ()
		{
			CreateAsyncTest (Control, () => {
				Assert.AreEqual (4, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, -1)), "#1");
				Assert.AreEqual (1, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 2)), "#2");
				Assert.AreEqual (2, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 3)), "#3");
				Assert.AreEqual (3, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 4)), "#4");
				Assert.AreEqual (4, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 5)), "#5");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IndexFromPosition_OutOfRangeTest ()
		{
			CreateAsyncTest (Control, () => {
				Assert.AreEqual (-3, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-2, -1)), "#1");
				Assert.AreEqual (3, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, -2)), "#2");

				Assert.AreEqual (-1, Generator.IndexFromGeneratorPosition (new GeneratorPosition (10, -1)), "#3");
				Assert.AreEqual (-1, Generator.IndexFromGeneratorPosition (new GeneratorPosition (10, 0)), "#4");
				Assert.AreEqual (9, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 10)), "#5");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IndexFromPosition_FirstRealisedTest ()
		{
			bool realised;
			CreateAsyncTest (Control,
				() => {
					using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, true))
						IGenerator.GenerateNext (out realised);
				}, () => {
					Assert.AreEqual (0, Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 0)), "#1");
					Assert.AreEqual (1, Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 1)), "#2");
					Assert.AreEqual (2, Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 2)), "#3");
					Assert.AreEqual (3, Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 3)), "#4");
					Assert.AreEqual (4, Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 4)), "#5");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void IndexFromPosition_StartOrEndTest ()
		{
			CreateAsyncTest (Control, () => {
				Assert.AreEqual (2, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 3)), "#0");
				Assert.AreEqual (1, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 2)), "#1");
				Assert.AreEqual (0, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 1)), "#2");
				Assert.AreEqual (-1, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, 0)), "#3");
				Assert.AreEqual (4, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, -1)), "#4");
				Assert.AreEqual (3, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, -2)), "#5");
				Assert.AreEqual (2, Generator.IndexFromGeneratorPosition (new GeneratorPosition (-1, -3)), "#5");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IndexFromPosition_First_ThirdRealisedTest ()
		{
			bool realised;
			CreateAsyncTest (Control,
				() => {
					using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, true))
						IGenerator.GenerateNext (out realised);
					using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (2), GeneratorDirection.Forward, true))
						IGenerator.GenerateNext (out realised);
				}, () => {
					Assert.AreEqual (0, Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 0)), "#1");
					Assert.AreEqual (1, Generator.IndexFromGeneratorPosition (new GeneratorPosition (0, 1)), "#2");
					Assert.AreEqual (2, Generator.IndexFromGeneratorPosition (new GeneratorPosition (1, 0)), "#3");
					Assert.AreEqual (3, Generator.IndexFromGeneratorPosition (new GeneratorPosition (1, 1)), "#4");
					Assert.AreEqual (4, Generator.IndexFromGeneratorPosition (new GeneratorPosition (1, 2)), "#5");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("If we copy UIElements to the DataContext we end up with horrible infinite recursion.")]
		public void ListBox_ItemCopiedToDataContext ()
		{
			bool realized;
			ListBox box = new ListBox ();
			box.ItemsPanel = CreateVirtualizingPanel ();
			CreateAsyncTest (box,
				() => {
					box.Items.Add (new Rectangle ());
				}, () => {
					ListBoxItem container;
					var panel = box.FindFirstChild<CustomVirtualizingPanel> ();
					using (panel.ItemContainerGenerator.StartAt (new GeneratorPosition (-1, 1), GeneratorDirection.Forward, true))
						container = (ListBoxItem) panel.ItemContainerGenerator.GenerateNext (out realized);
					Assert.IsTrue (realized, "#1");
					Assert.AreEqual (box.Items [0], container.DataContext, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ListBox_ItemCopiedToDataContext2 ()
		{
			bool realized;
			ListBox box = new ListBox ();
			box.ItemsPanel = CreateVirtualizingPanel ();
			CreateAsyncTest (box,
				() => {
					box.Items.Add (new Rectangle ());
				}, () => {
					ListBoxItem container;
					var panel = box.FindFirstChild<CustomVirtualizingPanel> ();
					using (panel.ItemContainerGenerator.StartAt (new GeneratorPosition (-1, 1), GeneratorDirection.Forward, true))
						container = (ListBoxItem) panel.ItemContainerGenerator.GenerateNext (out realized);

					Assert.IsNotNull (container, "#1");
					panel.ItemContainerGenerator.PrepareItemContainer (container);
					Assert.AreEqual (box.Items[0], container.Content, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("Supposedly this works but it shouldn't! I've created a container with one generator and Prepared it with another!")]
		public void ListBox_ItemCopiedToDataContext3 ()
		{
			bool realized;
			ListBox box = new ListBox ();
			box.ItemsPanel = CreateVirtualizingPanel ();
			CreateAsyncTest (box,
				() => {
					box.Items.Add (new Rectangle ());
				}, () => {
					ListBoxItem container;
					var panel = box.FindFirstChild<CustomVirtualizingPanel> ();
					using (panel.ItemContainerGenerator.StartAt (new GeneratorPosition (-1, 1), GeneratorDirection.Forward, true))
						container = (ListBoxItem) panel.ItemContainerGenerator.GenerateNext (out realized);
					Assert.IsNotNull (container, "#1");
					IGenerator.PrepareItemContainer (container);
					Assert.AreEqual (box.Items [0], container.Content, "#2");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void IsNewlyRealized_Create_NotOwnContainer ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				IGenerator.Remove (new GeneratorPosition (0, 0), 1);
				using (IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					IGenerator.GenerateNext (out fresh);
				Assert.IsTrue (fresh, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IsNewlyRealized_Create_OwnContainer ()
		{
			Control.Items.Clear ();
			Control.Items.Add (new Rectangle ());
			bool fresh;
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				IGenerator.Remove (new GeneratorPosition (0, 0), 1);
				using (IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					IGenerator.GenerateNext (out fresh);
				Assert.IsTrue (fresh, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IsNewlyRealized_Recycled_NotOwnContainer ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				var originalContainer = Generator.ContainerFromIndex (0);
				IGenerator.Recycle (new GeneratorPosition (0, 0), 1);
				using (IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					Assert.AreSame (originalContainer, IGenerator.GenerateNext (out fresh));
				Assert.IsFalse (fresh, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IsNewlyRealized_Recycled_OwnContainer ()
		{
			Control.Items.Clear ();
			Control.Items.Add (new Rectangle ());
			bool fresh;
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				IGenerator.Recycle (new GeneratorPosition (0, 0), 1);
				using (IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					IGenerator.GenerateNext (out fresh);
				Assert.IsTrue (fresh, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IsNewlyRealized_AlreadyRealized_NotOwnContainer ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				var originalContainer = Generator.ContainerFromIndex (0);
				using (IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					Assert.AreSame (originalContainer, IGenerator.GenerateNext (out fresh));
				Assert.IsFalse (fresh, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IsNewlyRealized_AlreadyRealized_OwnContainer ()
		{
			Control.Items.Clear ();
			Control.Items.Add (new Rectangle ());
			bool fresh;
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				using (IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					IGenerator.GenerateNext (out fresh);
				Assert.IsFalse (fresh, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("ItemFromContainer fails to retrieve the Item but ContainerFromItem works fine with the same setup")]
		public void ItemFromContainerTest ()
		{
			bool realised;
			CreateAsyncTest (Control, () => {
				using (var g = IGenerator.StartAt (IGenerator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, true)) {
					var container = IGenerator.GenerateNext (out realised);
					var item = Generator.ItemFromContainer (container);
					Assert.IsNotNull (item, "#1.");
					Assert.AreEqual (typeof (object), item.GetType (), "#2");
					Assert.AreNotEqual (item, Control.Items [0], "#3");
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_AddItems_Unrealized ()
		{
			// Generating containers does not raise ItemsChanged
			Control.Items.Clear ();
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				for (int i = 0; i < 5; i++) {
					Control.Items.Add (new object ());
					Assert.IsNotNull (result, "#1." + i);
					Assert.AreEqual (NotifyCollectionChangedAction.Add, result.Action, "#3." + i);
					Assert.AreEqual (1, result.ItemCount, "#4." + i);
					Assert.AreEqual (0, result.ItemUICount, "#5." + i);
					Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#6." + i);
					Assert.AreEqual (new GeneratorPosition (-1, 1), result.Position, "#7." + i);
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_AddItems_Realized ()
		{
			// Generating containers does not raise ItemsChanged
			Control.Items.Clear ();
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				for (int i = 0; i < 5; i++) {
					Control.Items.Add (new object ());
					Generate (i, 1);
					Assert.IsNotNull (result, "#1." + i);
					Assert.AreEqual (NotifyCollectionChangedAction.Add, result.Action, "#3." + i);
					Assert.AreEqual (1, result.ItemCount, "#4." + i);
					Assert.AreEqual (0, result.ItemUICount, "#5." + i);
					Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#6." + i);

					if (i == 0)
						Assert.AreEqual (new GeneratorPosition (-1, 1), result.Position, "#7." + i);
					else
						Assert.AreEqual (new GeneratorPosition (i - 1, 1), result.Position, "#8." + i);
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_InsertItemsAtStart_Unrealized ()
		{
			// Generating containers does not raise ItemsChanged
			Control.Items.Clear ();
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				for (int i = 0; i < 5; i++) {
					Control.Items.Insert(0, new object ());
					Assert.IsNotNull (result, "#1");
					Assert.AreEqual (NotifyCollectionChangedAction.Add, result.Action, "#3");
					Assert.AreEqual (1, result.ItemCount, "#4");
					Assert.AreEqual (0, result.ItemUICount, "#5");
					Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#6");
					Assert.AreEqual (new GeneratorPosition (-1, 1), result.Position, "#7");
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_InsertItemsAtStart_Realized ()
		{
			// Generating containers does not raise ItemsChanged
			Control.Items.Clear ();
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				for (int i = 0; i < 5; i++) {
					Control.Items.Insert (0, new object ());
					Generate (0, 1);
					Assert.IsNotNull (result, "#1");
					Assert.AreEqual (NotifyCollectionChangedAction.Add, result.Action, "#3");
					Assert.AreEqual (1, result.ItemCount, "#4");
					Assert.AreEqual (0, result.ItemUICount, "#5");
					Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#6");
					Assert.AreEqual (new GeneratorPosition (-1, 1), result.Position, "#7");
				}
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_RemoveFirstItem_Unrealized ()
		{
			// Replacing unrealized items does not raise this event
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				Control.Items.RemoveAt (0);
				Assert.IsNotNull (result, "#1");
				Assert.AreEqual (NotifyCollectionChangedAction.Remove, result.Action, "#3");
				Assert.AreEqual (1, result.ItemCount, "#4");
				Assert.AreEqual (0, result.ItemUICount, "#5");
				Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#6");
				Assert.AreEqual (new GeneratorPosition (-1, 1), result.Position, "#7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_RemoveFirstItem_Realized ()
		{
			// Replacing unrealized items does not raise this event
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				Generate (0, Control.Items.Count);
				Control.Items.RemoveAt (0);
				Assert.IsNotNull (result, "#1");
				Assert.AreEqual (NotifyCollectionChangedAction.Remove, result.Action, "#3");
				Assert.AreEqual (1, result.ItemCount, "#4");
				Assert.AreEqual (1, result.ItemUICount, "#5");
				Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#6");
				Assert.AreEqual (new GeneratorPosition (0, 0), result.Position, "#7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_RemoveLastItem_Unrealized ()
		{
			// Replacing unrealized items does not raise this event
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				Control.Items.RemoveAt (Control.Items.Count - 1);
				Assert.IsNotNull (result, "#1");
				Assert.AreEqual (NotifyCollectionChangedAction.Remove, result.Action, "#3");
				Assert.AreEqual (1, result.ItemCount, "#4");
				Assert.AreEqual (0, result.ItemUICount, "#5");
				Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#6");
				Assert.AreEqual (new GeneratorPosition (-1, 5), result.Position, "#7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_RemoveLastItem_Realized ()
		{
			// Replacing unrealized items does not raise this event
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				Generate (0, Control.Items.Count);
				Control.Items.RemoveAt (Control.Items.Count - 1);
				Assert.IsNotNull (result, "#1");
				Assert.AreEqual (NotifyCollectionChangedAction.Remove, result.Action, "#3");
				Assert.AreEqual (1, result.ItemCount, "#4");
				Assert.AreEqual (1, result.ItemUICount, "#5");
				Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#6");
				Assert.AreEqual (new GeneratorPosition (4, 0), result.Position, "#7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_ReplaceItem_Unrealized ()
		{
			// Replacing unrealized items does not raise this event
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control, () => {
				Control.Items[0] = new object();
				Assert.IsNull (result, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_ReplaceItem_Realized ()
		{
			// Replacing unrealized items does not raise this event
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control,
				() => {
					Generate (0, 1);
				}, () => {
					Control.Items [0] = new object ();
					Assert.IsNotNull (result, "#1");
					Assert.AreEqual (NotifyCollectionChangedAction.Replace, result.Action, "#2");
					Assert.AreEqual (1, result.ItemCount, "#3");
					Assert.AreEqual (1, result.ItemUICount, "#4");
					Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#5");
					Assert.AreEqual (new GeneratorPosition (0, 0), result.Position, "#6");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_ReplaceSecondItem_Realized ()
		{
			// Replacing unrealized items does not raise this event
			ItemsChangedEventArgs result = null;
			Generator.ItemsChanged += (o, e) => result = e;

			CreateAsyncTest (Control,
				() => {
					Generate (0, 2);
				}, () => {
					Control.Items [1] = new object ();
					Assert.IsNotNull (result, "#1");
					Assert.AreEqual (NotifyCollectionChangedAction.Replace, result.Action, "#2");
					Assert.AreEqual (1, result.ItemCount, "#3");
					Assert.AreEqual (1, result.ItemUICount, "#4");
					Assert.AreEqual (new GeneratorPosition (-1, 0), result.OldPosition, "#5");
					Assert.AreEqual (new GeneratorPosition (1, 0), result.Position, "#6");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsChanged_GenerateContainer ()
		{
			// Generating containers does not raise ItemsChanged
			var results = new List<ItemsChangedEventArgs>();
			Generator.ItemsChanged += (o, e) => results.Add (e);

			CreateAsyncTest (Control, () => {
				Assert.AreEqual (0, results.Count, "#1");
				Generate (0, 1);
				Assert.AreEqual (0, results.Count, "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ItemsControlOwnsGenerator ()
		{
			IItemContainerGenerator original = null;
			CreateAsyncTest (Control,
				() => original = Generator,
				() => Control.ItemsPanel = CreateVirtualizingPanel (),
				() => Assert.AreSame (original, Generator, "#1")
			);
		}

		[TestMethod]
		[Asynchronous]
		public void Panel_OnItemsChangedEventOrder1 ()
		{
			// Hook into everything before the template is applied
			var ordering = new List<string> ();
			ItemsControlPoker c = new ItemsControlPoker ();
			c.ItemsPanel = CreateVirtualizingPanel ();
			c.OnItemsChangedAction = (e) => ordering.Add ("ItemsControl.OnItemsChanged");
			c.ItemContainerGenerator.ItemsChanged += (o, e) => ordering.Add ("ICG.ItemsChanged");
			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					var panel = c.FindFirstChild<CustomVirtualizingPanel> ();
					panel.OnItemsChangedAction = () => ordering.Add ("Panel.OnItemsChanged");
				}, () => {
					c.Items.Add (new object ());
					Assert.AreEqual (2, ordering.Count, "#1");
					Assert.AreEqual ("ICG.ItemsChanged", ordering [0], "#2");
					Assert.AreEqual ("ItemsControl.OnItemsChanged", ordering [1], "#3");
				}, () => {
					ordering.Clear ();
					c.Items.Add (new object ());
					Assert.AreEqual (2, ordering.Count, "#4");
					Assert.AreEqual ("ICG.ItemsChanged", ordering [0], "#7");
					Assert.AreEqual ("ItemsControl.OnItemsChanged", ordering [1], "#5");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void Panel_OnItemsChangedEventOrder2 ()
		{
			// Hook into everything after the template is applied
			var ordering = new List<string> ();
			ItemsControlPoker c = new ItemsControlPoker ();
			c.ItemsPanel = CreateVirtualizingPanel ();

			CreateAsyncTest (c,
				() => c.ApplyTemplate (),
				() => {
					c.FindFirstChild<CustomVirtualizingPanel> ().OnItemsChangedAction = () => ordering.Add ("Panel.OnItemsChanged");
					c.OnItemsChangedAction = (e) => ordering.Add ("ItemsControl.OnItemsChanged");
					c.ItemContainerGenerator.ItemsChanged += (o, e) => ordering.Add ("ICG.ItemsChanged");
				}, () => {
					c.Items.Add (new object ());
					Assert.AreEqual (2, ordering.Count, "#1");
					Assert.AreEqual ("ICG.ItemsChanged", ordering [0], "#2");
					Assert.AreEqual ("ItemsControl.OnItemsChanged", ordering [1], "#3");
				}, () => {
					ordering.Clear ();
					c.Items.Add (new object ());
					Assert.AreEqual (2, ordering.Count, "#4");
					Assert.AreEqual ("ICG.ItemsChanged", ordering [0], "#5");
					Assert.AreEqual ("ItemsControl.OnItemsChanged", ordering [1], "#6");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("we aren't raising the event properly")]
		public void Panel_OnItemsChanged_TouchGenerator ()
		{
			bool raised = false;
			var control = new ItemsControlPoker ();
			control.ItemsPanel = CreateVirtualizingPanel ();

			CreateAsyncTest (control, () => {
				var panel = control.FindFirstChild<CustomVirtualizingPanel>();
				panel.OnItemsChangedAction = () => raised = true;
				control.Items.Add (new object ());
				Assert.IsTrue (raised, "#1");
			});
		}

		[TestMethod]
		public void Panel_NullGeneratorByDefault ()
		{
			Assert.Throws<InvalidOperationException> (() => {
				var g = new CustomVirtualizingPanel ().ItemContainerGenerator;
				GC.KeepAlive (g);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void PositionFromIndex_NoRealisedTest ()
		{
			CreateAsyncTest (Control, () => {
				Assert.AreEqual (new GeneratorPosition (-1, 1), Generator.GeneratorPositionFromIndex (0), "#1");
				Assert.AreEqual (new GeneratorPosition (-1, 2), Generator.GeneratorPositionFromIndex (1), "#2");
				Assert.AreEqual (new GeneratorPosition (-1, 3), Generator.GeneratorPositionFromIndex (2), "#3");
				Assert.AreEqual (new GeneratorPosition (-1, 4), Generator.GeneratorPositionFromIndex (3), "#4");
				Assert.AreEqual (new GeneratorPosition (-1, 5), Generator.GeneratorPositionFromIndex (4), "#5");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void PositionFromIndex_FirstRealisedTest ()
		{
			bool realised;
			CreateAsyncTest (Control,
				() => {
					using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, true))
						IGenerator.GenerateNext (out realised);
				}, () => {
					Assert.AreEqual (new GeneratorPosition (0, 0), Generator.GeneratorPositionFromIndex (0), "#1");
					Assert.AreEqual (new GeneratorPosition (0, 1), Generator.GeneratorPositionFromIndex (1), "#2");
					Assert.AreEqual (new GeneratorPosition (0, 2), Generator.GeneratorPositionFromIndex (2), "#3");
					Assert.AreEqual (new GeneratorPosition (0, 3), Generator.GeneratorPositionFromIndex (3), "#4");
					Assert.AreEqual (new GeneratorPosition (0, 4), Generator.GeneratorPositionFromIndex (4), "#5");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void PositionFromIndex_First_ThirdRealisedTest ()
		{
			bool realised;
			CreateAsyncTest (Control,
				() => {
					using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, true))
						IGenerator.GenerateNext (out realised);
					using (var v = IGenerator.StartAt (Generator.GeneratorPositionFromIndex (2), GeneratorDirection.Forward, true))
						IGenerator.GenerateNext (out realised);
				}, () => {
					Assert.AreEqual (new GeneratorPosition (0, 0), Generator.GeneratorPositionFromIndex (0), "#1");
					Assert.AreEqual (new GeneratorPosition (0, 1), Generator.GeneratorPositionFromIndex (1), "#2");
					Assert.AreEqual (new GeneratorPosition (1, 0), Generator.GeneratorPositionFromIndex (2), "#3");
					Assert.AreEqual (new GeneratorPosition (1, 1), Generator.GeneratorPositionFromIndex (3), "#4");
					Assert.AreEqual (new GeneratorPosition (1, 2), Generator.GeneratorPositionFromIndex (4), "#5");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void Recycle_OneContainer ()
		{
			bool fresh;
			object first, second;
			
			CreateAsyncTest (Control, () => {
				using (var g = IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, false))
					first = IGenerator.GenerateNext (out fresh);
				Assert.IsTrue (fresh, "#1");

				IGenerator.Recycle(new GeneratorPosition (0, 0), 1);
				Assert.IsNull (Generator.ContainerFromIndex (0), "#2");

				using (var g = IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, false))
					second = IGenerator.GenerateNext (out fresh);
				
				Assert.IsFalse (fresh, "#3");
				Assert.AreSame (first, second, "#4");

			});
		}

		[TestMethod]
		[Asynchronous]
		public void Recycle_CountOutOfRange ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				using (var g = IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, false))
					IGenerator.GenerateNext (out fresh);

				// You can't Recycle elements which have not been realised.
				Assert.Throws<InvalidOperationException> (() => {
					IGenerator.Recycle (new GeneratorPosition (0, 0), 5);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Recycle_NegativeOffset ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				using (var g = IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, false))
					IGenerator.GenerateNext (out fresh);

				// Offset must be zero as we have to refer to a realized element.
				Assert.Throws<ArgumentException> (() => {
					IGenerator.Recycle (new GeneratorPosition (0, -1), 5);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Recycle_PositiveOffset ()
		{
			bool fresh;
			CreateAsyncTest (Control, () => {
				using (var g = IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, false))
					IGenerator.GenerateNext (out fresh);

				// Offset must be zero as we have to refer to a realized element.
				Assert.Throws<ArgumentException> (() => {
					IGenerator.Recycle (new GeneratorPosition (0, 1), 5);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void SameGenerator ()
		{
			CreateAsyncTest (Control,
				() => Assert.AreSame (Control.ItemContainerGenerator, Panel.ItemContainerGenerator, "#1")
			);
		}

		[TestMethod]
		[Asynchronous]  
		public void StartAt_NegativeTest ()
		{
			CreateAsyncTest (Control, () => {
				IGenerator.StartAt (new GeneratorPosition (-100, -100), GeneratorDirection.Forward, true);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void StartAt_Negative_GenerateTest ()
		{
			bool realized;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (new GeneratorPosition (-100, -100), GeneratorDirection.Forward, true))
					Assert.IsNull(IGenerator.GenerateNext (out realized));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void StartAt_LastUnrealised_GenerateTest ()
		{
			// Generates the last unrealized item
			bool realized;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (new GeneratorPosition (-1, -1), GeneratorDirection.Forward, true))
					Assert.IsInstanceOfType<ContentPresenter> (IGenerator.GenerateNext (out realized), "#1");
				Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (4), "#2");
			});
		}

		[TestMethod] 
		[Asynchronous]
		public void StartAt_FirstUnrealised_GenerateTest ()
		{
			// Generates the last unrealized item
			bool realized;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					Assert.IsInstanceOfType<ContentPresenter> (IGenerator.GenerateNext (out realized), "#1");
				Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (0), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void StartAt_Middle_Then_FirstTest ()
		{
			// Generates the last unrealized item
			bool realized;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (new GeneratorPosition (-1, 3), GeneratorDirection.Forward, true))
					Assert.IsInstanceOfType<ContentPresenter> (IGenerator.GenerateNext (out realized), "#1");

				using (var v = IGenerator.StartAt (new GeneratorPosition (0, -2), GeneratorDirection.Forward, true))
					Assert.IsInstanceOfType<ContentPresenter> (IGenerator.GenerateNext (out realized), "#2");

				Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (0), "#3");
				Assert.IsNull (Generator.ContainerFromIndex (1), "#4");
				Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (2), "#5");
				Assert.IsNull (Generator.ContainerFromIndex (3), "#6");
				Assert.IsNull (Generator.ContainerFromIndex (4), "#7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void StartAt_Middle_Then_First2Test ()
		{
			// Generates the last unrealized item
			bool realized;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					Assert.IsInstanceOfType<ContentPresenter> (IGenerator.GenerateNext (out realized), "#1");
				using (var v = IGenerator.StartAt (new GeneratorPosition (0, 2), GeneratorDirection.Forward, true))
					Assert.IsInstanceOfType<ContentPresenter> (IGenerator.GenerateNext (out realized), "#2");

				Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (0), "#3");
				Assert.IsNull (Generator.ContainerFromIndex (1), "#4");
				Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (2), "#5");
				Assert.IsNull (Generator.ContainerFromIndex (3), "#6");
				Assert.IsNull (Generator.ContainerFromIndex (4), "#7");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void StartAt_FirstTest ()
		{
			// Generates the last unrealized item
			bool realized;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (new GeneratorPosition (-1, 0), GeneratorDirection.Forward, true))
					Assert.IsInstanceOfType<ContentPresenter> (IGenerator.GenerateNext (out realized), "#1");
				Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (0), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void StartAt_TwiceTest ()
		{
			CreateAsyncTest (Control, () => {
				IGenerator.StartAt (new GeneratorPosition (-100, -100), GeneratorDirection.Forward, true);
				Assert.Throws<InvalidOperationException> (() => {
					IGenerator.StartAt (new GeneratorPosition (-100, -100), GeneratorDirection.Forward, true);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Remove_NegativeOffsetTest ()
		{
			CreateAsyncTest (Control, () => {
				Assert.Throws<ArgumentException> (() => {
					IGenerator.Remove (new GeneratorPosition (0, -1), 1);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Remove_PositiveOffsetTest ()
		{
			CreateAsyncTest (Control, () => {
				Assert.Throws<ArgumentException> (() => {
					IGenerator.Remove (new GeneratorPosition (0, 1), 1);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("The correct exception should be an InvalidOperation, not NullRef")]
		public void Remove_BeforeGenerateTest ()
		{
			// If you try to remove an item *before* you generate any containers
			// you hit a null ref.
			CreateAsyncTest (Control, () => {
				Assert.Throws<NullReferenceException> (() => {
					IGenerator.Remove (new GeneratorPosition (0, 0), 1);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Remove_0Elements_1RealizedTest ()
		{
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				Assert.IsNotNull (Generator.ContainerFromIndex (0), "#1");
				IGenerator.Remove (new GeneratorPosition (0, 0), 1);
				Assert.IsNull (Generator.ContainerFromIndex (0), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Remove_1Element_1RealizedTest ()
		{
			CreateAsyncTest (Control, () => {
				AddElements (1);
				Generate (0, 1);
				Assert.IsNotNull (Generator.ContainerFromIndex (0), "#1");
				IGenerator.Remove (new GeneratorPosition (0, 0), 1);
				Assert.IsNull (Generator.ContainerFromIndex (0), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("The correct exception should be an InvalidOperation, not NullRef")]
		public void Remove_BeyondRealizedRangeTest ()
		{
			// Removing an item beyond the range which has been generated
			// throws a null ref.
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				Assert.Throws<NullReferenceException> (() => {
					IGenerator.Remove (new GeneratorPosition (4, 0), 1);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("The correct exception should be an InvalidOperation, not NullRef")]
		public void Remove_BeyondRealizedRangeTest2 ()
		{
			// Removing an item beyond the range which has been generated
			// throws a null ref.
			CreateAsyncTest (Control, () => {
				Generate (0, 10);
				IGenerator.RemoveAll ();
				Assert.Throws<NullReferenceException> (() => {
					IGenerator.Remove (new GeneratorPosition (4, 0), 1);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Remove_NotRealized_SpanTest ()
		{
			// You can't remove items which don't exist
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				Assert.Throws<InvalidOperationException> (() => {
					IGenerator.Remove (new GeneratorPosition (0, 0), 2);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void Remove_DoesntExistTest ()
		{
			// You can't remove items which don't exist
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				Assert.Throws<InvalidOperationException> (() => {
					IGenerator.Remove (new GeneratorPosition (0, 0), 2);
				});
			});
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveAll_NonGeneratedTest ()
		{
			CreateAsyncTest (Control, () => {
				IGenerator.RemoveAll ();
			});
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveAll_OneGeneratedTest ()
		{
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				IGenerator.RemoveAll ();
				Assert.IsNull (Generator.ContainerFromIndex (0), "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveAll_ManyGeneratedTest ()
		{
			CreateAsyncTest (Control, () => {
				Generate (0, 1);
				Generate (2, 6);
				Generate (9, 1);
				IGenerator.RemoveAll ();
				for (int i = 0; i < 15; i++)
					Assert.IsNull (Generator.ContainerFromIndex (i), "#" + i);
			});
		}

		void AddElements (int count)
		{
			while (count-- > 0)
				Panel.Children.Add (new Rectangle { Name = count.ToString () }); ;
		}

		new void Generate (int index, int count)
		{
			bool realized;
			var p = IGenerator.GeneratorPositionFromIndex (index);
			using (var d = IGenerator.StartAt (p, GeneratorDirection.Forward, false))
				while (count-- > 0)
					IGenerator.GenerateNext (out realized);
		}
	}
}
