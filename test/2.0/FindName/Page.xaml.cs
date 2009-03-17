using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;


namespace FindName
{
	public partial class Page : UserControl
	{
		public Page()
		{
			InitializeComponent();

			Loaded += delegate {
				// debug_text.Text = "Doing lookups";
				try { DoLookups (this); } catch { }
				try { DoLookups (my_canvas); } catch { }
				try { DoLookups ((FrameworkElement) my_canvas.FindName ("sub_element")); } catch { }
				try { DoLookups ((FrameworkElement) my_canvas.FindName ("component_element")); } catch { }
			};

			my_textbox1.TemplateApplied += delegate {
				try { DoTemplateLookups (my_textbox1); } catch { }
			};

			my_textbox2.TemplateApplied += delegate {
				try { DoTemplateLookups (my_textbox2); } catch { }
			};

		}

		private void DoTemplateLookups (MyTextBox elem)
		{
			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("Template Lookups from {0} ({1})", elem.Name, elem) });

			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("{0}.textBoxCanvas: {1}", elem, elem.GetTemplateChild_ ("textBoxCanvas")) });
			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("{0}.ContentElement: {1}", elem, elem.GetTemplateChild_ ("ContentElement")) });
			debug_panel.Children.Add (new Canvas () { Height = 20 });
		}

		private void DoLookups (FrameworkElement elem)
		{
			debug_panel.Children.Add (new TextBlock () { Text =  String.Format ("Lookups from {0} ({1})", elem.Name, elem) });

			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("{0}.my_canvas: {1}", elem, elem.FindName ("my_canvas")) });
			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("{0}.sub_element: {1}", elem, elem.FindName ("sub_element")) });
			debug_panel.Children.Add (new TextBlock () { Text = String.Format ("{0}.component_element: {1}", elem, elem.FindName ("component_element")) });
			debug_panel.Children.Add (new Canvas () { Height = 20 });
		}
	}
}
