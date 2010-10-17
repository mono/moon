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
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Windows.Controls.Primitives;
using System.Collections.Generic;
using System.Collections.Specialized;

namespace MoonTest.System.Windows.Controls {
	[TestClass]
	public class ItemContainerGeneratorTest_StandardPanel : ItemContaineGeneratorTest_PanelBase {

		public override void Initialize ()
		{
			base.Initialize ();
			Control.ItemsPanel = CreateStandardPanel ();
		}

		[TestMethod]
		[Asynchronous]
		public void CheckIndicesAfterRemoval ()
		{
			CreateAsyncTest (Control, () => {
				Generator.ContainerFromIndex (0);

				object second = Generator.ContainerFromIndex (1);

				// If we remove the first realized item, all the existing realized
				// items are updated with new indices
				Control.Items.RemoveAt (0);

				Assert.AreSame (second, Generator.ContainerFromIndex (0), "#1");
				Assert.AreEqual (new GeneratorPosition (0, 0), Generator.GeneratorPositionFromIndex (0), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ClearItemsList ()
		{
			int count = Control.Items.Count;
			CreateAsyncTest (Control, () => {
				for (int i = 0; i < count; i++)
					Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (i), "#1");
				Control.Items.Clear ();
				for (int i = 0; i < count; i++)
					Assert.IsNull (Generator.ContainerFromIndex (i), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ClearItemsList_CheckContainerFromIndex ()
		{
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					List<DependencyObject> containers = new List<DependencyObject> ();
					for (int i = 0; i < Control.Items.Count; i++)
						containers.Add (Generator.ContainerFromIndex (0));
					Control.Items.Clear ();
					for (int i = 0; i < containers.Count; i++)
						Assert.IsNull (Generator.ContainerFromIndex (i));
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ClearItemsList_CheckIndexFromContainer ()
		{
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					List<DependencyObject> containers = new List<DependencyObject> ();
					for (int i = 0; i < Control.Items.Count; i++)
						containers.Add (Generator.ContainerFromIndex (0));
					Control.Items.Clear ();
					for (int i = 0; i < containers.Count; i++)
						Assert.AreEqual (-1, Generator.IndexFromContainer (containers[i]));
				}
			);
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
					//Generate (i, 1);
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
		public void ClearItemsList_Twice ()
		{
			int count = Control.Items.Count;
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					// Check that all items are realized then unrealized
					for (int i = 0; i < count; i++)
						Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (i), "#1");
					Control.Items.Clear ();
					for (int i = 0; i < count; i++)
						Assert.IsNull (Generator.ContainerFromIndex (i), "#2");
					// Recreate some new items and verify they get realized/unrealized
					for (int i = 0; i < count; i++)
						Control.Items.Add (new object ());
				}, () => {
					for (int i = 0; i < count; i++)
						Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (i), "#3");
					Control.Items.Clear ();
					for (int i = 0; i < count; i++)
						Assert.IsNull (Generator.ContainerFromIndex (i), "#4");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void CreateAndRemoveContainer_IsOwnContainer ()
		{
			Control.Items.Clear ();
			Control.Items.Add (new ContentPresenter ());
			CreateAsyncTest (Control, () => {
				Assert.AreEqual (Control.Items[0], Generator.ContainerFromIndex (0), "#1");
				Control.Items.RemoveAt (0);
				Assert.IsNull (Generator.ContainerFromIndex (0), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void CreateAndRemoveContainer_IsNotOwnContainer ()
		{
			Control.Items.Clear ();
			Control.Items.Add (new object ());
			CreateAsyncTest (Control, () => {
				Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (0), "#1");
				Control.Items.RemoveAt (0);
				Assert.IsNull (Generator.ContainerFromIndex (0), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void OwnContainerIsInGenerator_StandardPanel ()
		{
			var item = new ContentPresenter ();
			Control.Items.Clear ();
			Control.ItemsPanel = CreateStandardPanel ();
			CreateAsyncTest (Control, () => {
				Control.Items.Add (item);
				Assert.AreSame (item, Control.ItemContainerGenerator.ContainerFromIndex (0), "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void OwnContainerRemoveFromGenerator_StandardPanel ()
		{
			var item = new ContentPresenter ();
			Control.Items.Clear ();
			Control.ItemsPanel = CreateStandardPanel ();
			CreateAsyncTest (Control, () => {
				Control.Items.Add (item);
				IGenerator.Remove (new GeneratorPosition (0, 0), 1);
				Assert.IsNull (Control.ItemContainerGenerator.ContainerFromIndex (0), "#1");
				Assert.AreEqual (1, Control.Items.Count, "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveItemsList ()
		{
			int count = Control.Items.Count;
			CreateAsyncTest (Control, () => {
				for (int i = 0; i < count; i++)
					Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (i), "#1");
				for (int i = 0; i < count; i++)
					Control.Items.RemoveAt (0);
				for (int i = 0; i < count; i++)
					Assert.IsNull (Generator.ContainerFromIndex (i), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveItemsList_Backwards ()
		{
			int count = Control.Items.Count;
			CreateAsyncTest (Control, () => {
				for (int i = 0; i < count; i++)
					Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (i), "#1");

				for (int i = 0; i < count; i++)
					Control.Items.RemoveAt (Control.Items.Count - 1);

				for (int i = 0; i < count; i++)
					Assert.IsNull (Generator.ContainerFromIndex (i), "#2");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveItemsList_CheckContainerFromIndex ()
		{
			int count = Control.Items.Count;
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					List<DependencyObject> containers = new List<DependencyObject> ();
					for (int i = 0; i < count; i++)
						containers.Add (Generator.ContainerFromIndex (0));
					for (int i = 0; i < count; i++)
						Control.Items.RemoveAt (0);
					for (int i = 0; i < count; i++)
						Assert.IsNull (Generator.ContainerFromIndex (i));
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveItemsList_CheckIndexFromContainer ()
		{
			int count = Control.Items.Count;
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					List<DependencyObject> containers = new List<DependencyObject> ();
					for (int i = 0; i < count; i++)
						containers.Add (Generator.ContainerFromIndex (0));
					for (int i = 0; i < count; i++)
						Control.Items.RemoveAt (0);
					for (int i = 0; i < count; i++)
						Assert.AreEqual (-1, Generator.IndexFromContainer (containers[i]));
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveItemsList_CheckIndexFromContainer_Backwards ()
		{
			int count = Control.Items.Count;
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					List<DependencyObject> containers = new List<DependencyObject> ();
					for (int i = 0; i < count; i++)
						containers.Add (Generator.ContainerFromIndex (0));
					for (int i = 0; i < count; i++)
						Control.Items.RemoveAt (Control.Items.Count - 1);
					for (int i = 0; i < count; i++)
						Assert.AreEqual (-1, Generator.IndexFromContainer (containers[i]));
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void RemoveItemsList_Twice ()
		{
			int count = Control.Items.Count;
			CreateAsyncTest (Control,
				() => Control.ApplyTemplate (),
				() => {
					// Check that all items are realized then unrealized
					for (int i = 0; i < count; i++)
						Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (i), "#1");
					for (int i = 0; i < count; i++)
						Control.Items.RemoveAt (Control.Items.Count - 1);
					for (int i = 0; i < count; i++)
						Assert.IsNull (Generator.ContainerFromIndex (i), "#2");

					// Recreate some new items and verify they get realized/unrealized
					for (int i = 0; i < count; i++)
						Control.Items.Add (new object ());
				}, () => {
					for (int i = 0; i < count; i++)
						Assert.IsInstanceOfType<ContentPresenter> (Generator.ContainerFromIndex (i), "#3");
					for (int i = 0; i < count; i++)
						Control.Items.RemoveAt (Control.Items.Count - 1);
					for (int i = 0; i < count; i++)
						Assert.IsNull (Generator.ContainerFromIndex (i), "#4");
				}
			);
		}

		[TestMethod]
		[Asynchronous]
		public void ReplaceRealizedItem ()
		{
			Control.Items.Clear ();
			Control.Items.Add (new object ());
			CreateAsyncTest (Control, () => {
				var container = Generator.ContainerFromIndex (0);
				Assert.IsNotNull (container, "#1");
				Control.Items [0] = new object ();
				Assert.IsNotNull (Generator.ContainerFromIndex (0), "#2");
				Assert.AreNotSame (container, Generator.ContainerFromIndex (0), "#3");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void ReplaceRealizedItem_CheckEventArgs ()
		{
			// Replacing a realized item results in a new
			// container being automatically generated
			Control.Items.Clear ();
			Control.Items.Add (new object ());

			CreateAsyncTest (Control, () => {
				var container = Generator.ContainerFromIndex (0);
				Assert.IsNotNull (container, "#1");
				Control.ItemContainerGenerator.ItemsChanged += (o, e) => {
					Assert.AreNotSame (container, Generator.ContainerFromIndex (0), "#1");
				};
				Control.Items [0] = new object ();
			});
		}
	}
}