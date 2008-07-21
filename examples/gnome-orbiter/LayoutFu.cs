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

	public static class LayoutFu {

		public static void FitTextBlock (TextBlock blk)
		{
			string txt = blk.Text;
			double w = blk.Width;
			
			blk.Text = txt;

			if (blk.ActualWidth <= w)
				return;

			for (int i = txt.Length; i >= 0; i--) {
				string mod = txt.Substring (0, i) + "...";
				blk.Text = mod;
				if (blk.ActualWidth <= w)
					return;
			}
		}

		public static void CenterTextBlock (TextBlock blk, FrameworkElement element)
		{
			double x = (element.Width - blk.ActualWidth) / 2;
			double y = (element.Height - blk.ActualHeight) / 2;

			blk.SetValue (Canvas.TopProperty, y);
			blk.SetValue (Canvas.LeftProperty, x);
		}

		public static void CenterTextBlock (string blkName, FrameworkElement element)
		{
			TextBlock blk = element.FindName (blkName) as TextBlock;
			CenterTextBlock (blk, element);
		}


	}

}
