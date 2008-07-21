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
using System.Windows.Shapes;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using System.Windows.Input;
using System;
using System.IO;

namespace GnomeOrbiter {

	public class Entry : Control {

		private FrameworkElement realControl;
		private double extraPad = 0.0;

		public double ExtraPad {
			set { extraPad = value; UpdateLayout (); }
		}

		public string Text {
			set { 
				TextBlock txtBlock = realControl.FindName ("EntryText") as TextBlock;
				txtBlock.Text = value;
				UpdateLayout ();
			}
		}

		public string Header {
			set { 
				TextBlock txtBlock = realControl.FindName ("EntryHeader") as TextBlock;
				txtBlock.Text = value;
				UpdateLayout ();
			}
		}
		
		public new double Height {
			get { 
				Rectangle entryFrame = realControl.FindName ("EntryFrame") as Rectangle;
				return entryFrame.Height;
			}
		}

		/* CONSTRUCTOR */
		public Entry (GtkSilver silver)
		{
			string xaml = File.ReadAllText (GnomeOrbiter.ThemeDirectory + "/" + "Entry.xaml");
			realControl = silver.InitializeFromXaml (xaml, this);

			realControl.MouseEnter += RealControlMouseEnterHandler;
			realControl.MouseLeave += RealControlMouseLeaveHandler;

			UpdateLayout ();
		}

		public void AnimateInFromLeft ()
		{
			Storyboard s = realControl.FindName ("AnimateInFromLeft") as Storyboard;
			if (s != null)
				s.Begin ();
		}

		public void AnimateInFromRight ()
		{
			Storyboard s = realControl.FindName ("AnimateInFromRight") as Storyboard;
			if (s != null)
				s.Begin ();
		}


		public void AnimateOutToLeft ()
		{
			Storyboard s = realControl.FindName ("AnimateOutToLeft") as Storyboard;
			if (s != null) {
				s.Completed += RemoveOnCompleted;
				s.Begin ();
			}
		}

		public void AnimateOutToRight ()
		{
			Storyboard s = realControl.FindName ("AnimateOutToRight") as Storyboard;
			if (s != null) {
				s.Completed += RemoveOnCompleted;
				s.Begin ();
			}
		}

		private void RealControlMouseEnterHandler (object o, MouseEventArgs e)
		{
			Storyboard s = realControl.FindName ("Highlight") as Storyboard;
			if (s != null)
				s.Begin ();
		}

		private void RealControlMouseLeaveHandler (object o, EventArgs e)
		{
			Storyboard s = realControl.FindName ("DeHighlight") as Storyboard;
			if (s != null)
				s.Begin ();
		}

		private void UpdateLayout ()
		{
			TextBlock txtBlock = realControl.FindName ("EntryText") as TextBlock;
			TextBlock headerBlock = realControl.FindName ("EntryHeader") as TextBlock;
			Rectangle entryFrame = realControl.FindName ("EntryFrame") as Rectangle;
			
			LayoutFu.FitTextBlock (headerBlock);

			double h = 15.0 + extraPad; // Borders
			h += txtBlock.ActualHeight;
			h += headerBlock.ActualHeight;

			entryFrame.Height = h;
		}

		private void RemoveOnCompleted (object o, EventArgs args)
		{
			if (Parent != null && Parent is Canvas)
				((Canvas) Parent).Children.Remove (this);
		}

	}

}
