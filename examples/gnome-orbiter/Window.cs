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

using System;
using Gtk;
using Gtk.Moonlight;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Controls;
using System.Windows.Input;
using System.IO;
using System.Collections.Generic;
using Application = Gtk.Application;

namespace GnomeOrbiter {

	public class Window : Gtk.Window {

		Throbber throbber;
		PlanetFetcher fetcher;
		GtkSilver silver;
		List <Entry> injectedList = new List <Entry> ();
		int currentHead = 0;

		Button prevButton;
		Button nextButton;

		/* CONSTRUCTOR */
		public Window () : base ("Gnome orbiter")
		{
			silver = new GtkSilver (320, 420);
			silver.Attach (new Canvas ());

			fetcher = new PlanetFetcher ();
			fetcher.FetchDone += OnFetchDone;
			fetcher.Start ();

			silver.Canvas.Children.Add (new Background (silver));
			
			throbber = new Throbber (silver);
			silver.Canvas.Children.Add (throbber);

			CreateButtons ();
			
			Add (silver);

			RectangleGeometry r = new RectangleGeometry ();
			r.Rect = new Rect (10.0, 10.0, 300.0, 400.0);
			silver.Canvas.Clip = r;

			silver.SetSizeRequest (320, 420);
		}

		protected override bool OnDeleteEvent (Gdk.Event evnt)
		{
			Application.Quit ();
			return true;
		}

		private void CreateButtons ()
		{
			prevButton = new Button (silver);
			nextButton = new Button (silver);

			prevButton.Text = "Previous";
			prevButton.SetValue (Canvas.TopProperty, 370);
			prevButton.SetValue (Canvas.LeftProperty, 20);
			prevButton.Clicked += OnPrevClicked;
			prevButton.Disabled = true;

			nextButton.Text = "Next";
			nextButton.SetValue (Canvas.TopProperty, 370);
			nextButton.SetValue (Canvas.LeftProperty, 170);
			nextButton.Clicked += OnNextClicked;
			nextButton.Disabled = true;

			silver.Canvas.Children.Add (prevButton);
			silver.Canvas.Children.Add (nextButton);
		}

		private void PopulateNew (bool fromLeft)
		{
			double pos = 20.0; 
			injectedList = new List <Entry> ();

			for (int i = currentHead; i < fetcher.List.Count; i++) {
				
				Entry entry = fetcher.List [i].ToEntryElement (silver);
				
				if (pos + entry.Height + 10 < 370.0) {
					pos += entry.Height + 10;
					injectedList.Add (entry);
				} else
					break;
			}

			double padding = (370.0 - pos) / injectedList.Count;
			pos = 20.0;

			foreach (Entry entry in injectedList) {
				entry.ExtraPad = padding;
				entry.SetValue (Canvas.TopProperty, pos);
				entry.SetValue (Canvas.LeftProperty, 20);
				if (fromLeft) 
					entry.AnimateInFromLeft ();
				else
					entry.AnimateInFromRight ();

				silver.Canvas.Children.Add (entry);

				pos += entry.Height + 10;
			}
		}

		private void UpdateButtonStatus ()
		{
			if (currentHead + injectedList.Count < fetcher.List.Count)
				nextButton.Disabled = false;
			else
				nextButton.Disabled = true;

			if (currentHead > 0)
				prevButton.Disabled = false;
			else
				prevButton.Disabled = true;
		}

		private void OnFetchDone (object sender, EventArgs args)
		{
			throbber.AnimateOut ();
			PopulateNew (false);
			UpdateButtonStatus ();
		}

		private void OnNextClicked (object sender, EventArgs args)
		{
			currentHead += injectedList.Count;

			foreach (Entry e in injectedList) 
				e.AnimateOutToLeft ();

			PopulateNew (false);
			UpdateButtonStatus ();
		}

		private void OnPrevClicked (object sender, EventArgs args)
		{
			currentHead -= injectedList.Count;

			foreach (Entry e in injectedList) 
				e.AnimateOutToRight ();

			PopulateNew (true);
			UpdateButtonStatus ();
		}

	}

}




