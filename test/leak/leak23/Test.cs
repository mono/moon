using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Windows.Markup;

namespace Leak
{
	public partial class Page
	{
		void RunTest ()
		{
			Control = (Control) XamlReader.Load (@"
<ContentControl  xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"">
	<ContentControl.Template>
		<ControlTemplate>
			<Grid Width=""{Binding Path=Width, RelativeSource={RelativeSource TemplatedParent}}"" />
		</ControlTemplate>
	</ContentControl.Template>
</ContentControl>");

			Control.ApplyTemplate ();
			WeakSubtree = (FrameworkElement) VisualTreeHelper.GetChild (Control, 0);
			Control.Template = null;
			
			GCAndInvoke (() => {
				if (WeakSubtree != null)
					Fail ("The subtree should be collected");

				// The subtree which databound to this property should be GC'ed
				// so lets make sure we don't invoke a GC'ed delegate
				Control.Width = 1;
	
				GCAndInvoke (() => {

					// One more try for good measure.
					Control.Width = 1;
	
					// If we haven't crashed, great!
					Succeed ();
				});
			});
		}
	}
}
