/*
 * Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
 *
 * Contact:
 *  Moonlight List (moonlight-list@lists.ximian.com)
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */

using Gtk;
using Gtk.Moonlight;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using System.Windows.Input;
using System;
using System.IO;

namespace GnomeOrbiter {

	public class Button : Control {

		private FrameworkElement realControl = null;
		private bool disabled = false;

		public event EventHandler Clicked;

		public string Text {
			set { 
				TextBlock txtBlock = realControl.FindName ("ButtonText") as TextBlock;
				txtBlock.Text = value;
				UpdateLayout ();
			}
		}

		public bool Disabled {
			set {
				if (value == true && disabled == false) {
					Storyboard s = realControl.FindName ("Disable") as Storyboard;
					if (s != null)
						s.Begin ();
					disabled = true;
				} else if (value == false && disabled == true) {
					Storyboard s = realControl.FindName ("UnDisable") as Storyboard;
					if (s != null)
						s.Begin ();
					disabled = false;
				}
			}
		}

		/* CONSTRUCTOR */
		public Button (GtkSilver silver)
		{
			string xaml = File.ReadAllText (GnomeOrbiter.ThemeDirectory + "/" + "Button.xaml");
			realControl = silver.InitializeFromXaml (xaml, this);
			
			realControl.MouseEnter += RealControlMouseEnterHandler;
			realControl.MouseLeave += RealControlMouseLeaveHandler;
			realControl.MouseLeftButtonDown += RealControlLeftButtonDown;

			UpdateLayout ();
		}

		private void UpdateLayout ()
		{
			LayoutFu.CenterTextBlock ("ButtonText", realControl);
		}

		private void RealControlMouseEnterHandler (object o, MouseEventArgs e)
		{
			if (disabled)
				return;

			Storyboard s = realControl.FindName ("Highlight") as Storyboard;
			if (s != null)
				s.Begin ();
		}

		private void RealControlMouseLeaveHandler (object o, EventArgs e)
		{
			if (disabled)
				return;

			Storyboard s = realControl.FindName ("DeHighlight") as Storyboard;
			if (s != null)
				s.Begin ();
		}

		private void RealControlLeftButtonDown (object o, MouseEventArgs args)
		{
			if (disabled)
				return;

			Storyboard s = realControl.FindName ("Clicked") as Storyboard;
			if (s != null)
				s.Begin ();

			if (Clicked != null)
				Clicked (null, new EventArgs ());
		}

	}

}
