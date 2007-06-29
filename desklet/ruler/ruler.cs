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

namespace Desklets
{
	public class Monitor : Canvas 
	{
		Storyboard storyboard;

		static readonly Brush LinesColor = new SolidColorBrush (Colors.Black);
		static readonly Brush ActiveColor = new SolidColorBrush (Color.FromRgb (30, 30, 30));
		static readonly Brush SelectedColor = new SolidColorBrush (Colors.Red);

		public void MouseDown (object sender, MouseEventArgs e)
		{
			(sender as Shape).Fill = SelectedColor;
			(sender as Shape).Stroke = SelectedColor;
		}

		public void MouseUp (object sender, EventArgs e)
		{
			(sender as Shape).Fill = LinesColor;
			(sender as Shape).Stroke = LinesColor;
		}

		public void MouseEnter (object sender, EventArgs e)
		{
			(sender as Shape).Fill = ActiveColor;
			(sender as Shape).Stroke = ActiveColor;
		}

		public void MouseLeft (object sender, EventArgs e)
		{
			(sender as Shape).Fill = LinesColor;
			(sender as Shape).Stroke = LinesColor;
		}
		
		private void DrawRuler ()
		{
			for (int i = 0; i <= 500; i += 2) {
				Rectangle line = new Rectangle ();
				line.Fill = LinesColor;
				line.Stroke = null;
				line.SetValue (Canvas.LeftProperty,  i+15);
				line.SetValue (Canvas.TopProperty, 2); 
				line.Width = 1;

				if ((i == 0) || (i%50) == 0)
					line.Height = 10;
				else if ((i%10) == 0)
					line.Height = 7;
				else
					line.Height = 4;

				Children.Add (line);
			}
		}

		public void PageLoaded (object o, EventArgs e)
		{
			storyboard = FindName ("storyboard") as Storyboard;

			DrawRuler ();

			Shape close_button = FindName ("close_button") as Shape;
			close_button.MouseLeftButtonDown += new MouseEventHandler (MouseDown);
			close_button.MouseLeftButtonUp += new MouseEventHandler (MouseUp);
			close_button.MouseEnter += new MouseEventHandler (MouseEnter);
			close_button.MouseLeave += new EventHandler (MouseLeft);
		}
	}
}
