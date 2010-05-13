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
using MoonTest.System.Windows.Controls;
using System.Windows.Controls.Primitives;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using System.Windows.Markup;

namespace MoonTest.System.Windows.Controls {
	public abstract class ItemContaineGeneratorTest_PanelBase : SilverlightTest {

		protected ItemsControlPoker Control {
			get; set;
		}

		protected ItemContainerGenerator Generator {
			get { return (ItemContainerGenerator) Control.ItemContainerGenerator; }
		}

		protected IRecyclingItemContainerGenerator IGenerator {
			get { return Control.ItemContainerGenerator; }
		}

		[TestInitialize]
		public virtual void Initialize ()
		{
			Control = new ItemsControlPoker ();
			Control.ItemsPanel = CreateVirtualizingPanel ();
			for (int i = 0; i < 5; i++)
				Control.Items.Add (i.ToString ());
		}

		protected ItemsPanelTemplate CreateStandardPanel ()
		{
			return (ItemsPanelTemplate) XamlReader.Load (@"
<ItemsPanelTemplate xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
		<StackPanel x:Name=""Virtual"" />
</ItemsPanelTemplate>");
		}

		protected ItemsPanelTemplate CreateVirtualizingPanel ()
		{
			return (ItemsPanelTemplate) XamlReader.Load (@"
<ItemsPanelTemplate xmlns=""http://schemas.microsoft.com/client/2007""
            xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
			xmlns:clr=""clr-namespace:MoonTest.System.Windows.Controls;assembly=moon-unit"">
		<clr:CustomVirtualizingPanel x:Name=""Virtual"" />
</ItemsPanelTemplate>");
		}


		protected void Generate (int index, int count)
		{
			bool realized;
			var p = IGenerator.GeneratorPositionFromIndex (index);
			using (var d = IGenerator.StartAt (p, GeneratorDirection.Forward, false))
				while (count-- > 0)
					IGenerator.GenerateNext (out realized);
		}
	}
}