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

namespace MoonTest.System.Windows.Controls
{
	public class CustomVirtualizingPanel : VirtualizingPanel { }

	[TestClass]
	public partial class IItemContainerGeneratorTest : SilverlightTest {
		ItemsControlPoker Control {
			get; set;
		}

		ItemContainerGenerator Generator {
			get { return (ItemContainerGenerator) Panel.ItemContainerGenerator; }
		}

		IItemContainerGenerator IGenerator {
			get { return Panel.ItemContainerGenerator; }
		}

		CustomVirtualizingPanel Panel {
			get {
				return (CustomVirtualizingPanel) VisualTreeHelper.GetChild (VisualTreeHelper.GetChild (Control, 0), 0);
			}
		}

		[TestInitialize]
		public void Initialize ()
		{
			Control = new ItemsControlPoker ();
			Control.ItemsPanel = (ItemsPanelTemplate) XamlReader.Load (@"
<ItemsPanelTemplate xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
			xmlns:clr=""clr-namespace:MoonTest.System.Windows.Controls;assembly=moon-unit"">
		<clr:CustomVirtualizingPanel x:Name=""Virtual"" />
</ItemsPanelTemplate>");
			for (int i = 0; i < 5; i++)
				Control.Items.Add (i.ToString ());
		}

		[TestMethod]
		[Asynchronous]
		public void GenerateAll ()
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
		public void GeneratorType ()
		{
			CreateAsyncTest (Control, () => {
				Assert.IsInstanceOfType<ItemContainerGenerator> (IGenerator, "#1");
			});
		}

		[TestMethod]
		[Asynchronous]
		public void IndexFromPosition_CrossRealised ()
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
		public void IndexFromPosition_LandOnRealised ()
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
		public void IndexFromPosition_NoRealised ()
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
		public void IndexFromPosition_OutOfRange ()
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
		public void IndexFromPosition_FirstRealised ()
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
		public void IndexFromPosition_StartOrEnd ()
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
		public void IndexFromPosition_First_ThirdRealised ()
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
		[MoonlightBug ("Initially there's no link between the item and it's container. ItemsControl creates this link somehow...")]
		public void ItemFromContainer ()
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
		public void PositionFromIndex_NoRealised ()
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
		public void PositionFromIndex_FirstRealised ()
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
		public void PositionFromIndex_First_ThirdRealised ()
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
		public void StartAt_Negative ()
		{
			CreateAsyncTest (Control, () => {
				IGenerator.StartAt (new GeneratorPosition (-100, -100), GeneratorDirection.Forward, true);
			});
		}

		[TestMethod]
		[Asynchronous]
		public void StartAt_Negative_Generate ()
		{
			bool realized;
			CreateAsyncTest (Control, () => {
				using (var v = IGenerator.StartAt (new GeneratorPosition (-100, -100), GeneratorDirection.Forward, true))
					Assert.IsNull(IGenerator.GenerateNext (out realized));
			});
		}

		[TestMethod]
		[Asynchronous]
		public void StartAt_LastUnrealised_Generate ()
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
		public void StartAt_FirstUnrealised_Generate ()
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
		public void StartAt_Middle_Then_First ()
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
		public void StartAt_Middle_Then_First2 ()
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
		public void StartAt_First ()
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
		public void StartAt_Twice ()
		{
			CreateAsyncTest (Control, () => {
				IGenerator.StartAt (new GeneratorPosition (-100, -100), GeneratorDirection.Forward, true);
				Assert.Throws<InvalidOperationException> (() => {
					IGenerator.StartAt (new GeneratorPosition (-100, -100), GeneratorDirection.Forward, true);
				});
			});
		}
	}
}
