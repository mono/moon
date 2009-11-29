/*
 * ruler.cs: Screen ruler with theming support.
 *
 * Author:
 *   Everaldo Canuto (ecanuto@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.IO;

namespace Desklets.Ruler
{
	public partial class Monitor : Canvas 
	{
		public Monitor() {
			InitializeComponent();
		}

		static readonly Brush LinesColor = new SolidColorBrush (Colors.Black);
		static readonly Brush ActiveColor = new SolidColorBrush (Color.FromArgb (255, 30, 30, 30));
		static readonly Brush SelectedColor = new SolidColorBrush (Colors.Red);
		
		public void MouseDown (object sender, MouseEventArgs e)
		{
			Shape s = sender as Shape;

			if (s == null)
				return;
			
			s.Fill = SelectedColor;
			s.Stroke = SelectedColor;
		}

		public void MouseUp (object sender, EventArgs e)
		{
			Shape s = sender as Shape;

			if (s == null)
				return;
			
			s.Fill = LinesColor;
			s.Stroke = LinesColor;
		}

		public void MouseEnter (object sender, EventArgs e)
		{
			Shape s = sender as Shape;

			if (s == null)
				return;
			
			s.Fill = ActiveColor;
			s.Stroke = ActiveColor;
		}

		public void MouseLeft (object sender, EventArgs e)
		{
			Shape s = sender as Shape;
			if (s == null)
				return;
			
			s.Fill = LinesColor;
			s.Stroke = LinesColor;
		}
		
		private void DrawRuler ()
		{
			TextBlock label;
			double left;
			
			for (int i = 0; i <= 500; i += 2) {
				left = i + 15.0;
				Rectangle line = new Rectangle ();
				line.Fill = LinesColor;
				line.Stroke = null;
				Canvas.SetLeft (line, left);
				Canvas.SetTop (line, 2);
				line.Width = 1;

				if ((i == 0) || (i%50) == 0) {
					line.Height = 10;
					label = new TextBlock ();
					label.Text = i.ToString ();
					Canvas.SetLeft (label, left);
					Canvas.SetTop (label, 14);
					label.Foreground = LinesColor;
					label.FontSize = 8;
					Children.Add (label);
				} else if ((i%10) == 0)
					line.Height = 7;
				else
					line.Height = 4;

				Children.Add (line);
			}
		}

		public void PageLoaded (object o, EventArgs e)
		{
			DrawRuler ();

			Moonlight.Gtk.Desklet.SetupToolbox(this);
			
			storyboard = FindName ("storyboard") as Storyboard;
			
			Shape close_button = FindName ("desklet_close") as Shape;
			close_button.MouseLeftButtonDown += new MouseButtonEventHandler (MouseDown);
			close_button.MouseLeftButtonUp += new MouseButtonEventHandler (MouseUp);
			close_button.MouseEnter += new MouseEventHandler (MouseEnter);
			close_button.MouseLeave += new MouseEventHandler (MouseLeft);
		}
	}
}
