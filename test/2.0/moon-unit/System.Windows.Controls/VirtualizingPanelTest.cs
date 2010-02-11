//
// VirtualizingPanel Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using Microsoft.Silverlight.Testing;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using MoonTest;
using System.Windows.Markup;
using System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class VirtualizingPanelTest : ItemContaineGeneratorTest_PanelBase
	{
		public VirtualizingPanelPoker Panel
		{
			get { return Control.FindFirstChild<VirtualizingPanelPoker> (); }
		}

		public IItemContainerGenerator PanelGenerator
		{
			get { return Panel.ItemContainerGenerator; }
		}

		public override void Initialize ()
		{
			base.Initialize ();
			Control.ItemsPanel = (ItemsPanelTemplate) XamlReader.Load (@"
<ItemsPanelTemplate xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
			xmlns:clr=""clr-namespace:MoonTest.System.Windows.Controls;assembly=moon-unit"">
		<clr:VirtualizingPanelPoker x:Name=""Virtual"" />
</ItemsPanelTemplate>");
		}

		[TestMethod]
		[Asynchronous]
		public void ClearChildren ()
		{
			bool fresh;
			bool changed = false;
			bool cleared = false;
			CreateAsyncTest (Control,
				() => {
					using (IGenerator.StartAt (IGenerator.GeneratorPositionFromIndex (0), GeneratorDirection.Forward, true))
						Panel.Children.Add ((FrameworkElement) PanelGenerator.GenerateNext (out fresh));

					Panel.OnClearChildrenAction = delegate {
						Assert.AreEqual (0, Panel.Children.Count, "#3");
						Assert.IsFalse (changed, "#4");
						cleared = true;
					};
					Panel.OnItemsChangedAction = delegate {
						Assert.AreEqual (0, Panel.Children.Count, "#1");
						Assert.IsTrue (cleared, "#2");
						changed = true;
					};
					Control.Items.Clear ();

					// Events should be raised synchronously
					Assert.IsTrue (changed && cleared, "#5");
				}
			);
		}
	}

	public class VirtualizingPanelPoker : VirtualizingPanel
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

		protected override void OnItemsChanged (object sender, global::System.Windows.Controls.Primitives.ItemsChangedEventArgs args)
		{
			if (OnItemsChangedAction != null)
				OnItemsChangedAction ();

			base.OnItemsChanged (sender, args);
		}
	}
}
