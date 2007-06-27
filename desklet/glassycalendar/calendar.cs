/*
 * calendar.cs: Glassy calendar desklet.
 *
 * Author:
 *   Everaldo Canuto (ecanuto@novell.com)
 *   based on Calendar from Rodrigo Kumpera <kumpera@novell.com>
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Globalization;

using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklet
{
	public class CalendarPanel : Canvas 
	{
		TextBlock monthText;
		TextBlock[,] days = new TextBlock[6,7];
		DateTime current;
		DateTime currentHour;

		static readonly Brush CURRENT_DAY = new SolidColorBrush (Colors.Yellow);
		static readonly Brush CURRENT_MONTH = new SolidColorBrush (Colors.White);
		//Dark grey
		static readonly Brush OTHERS = new SolidColorBrush (Color.FromRgb (44, 44, 44));

		static readonly Brush BUTTON_UP_BRUSH = new SolidColorBrush (Colors.White);
		//Grey
		static readonly Brush BUTTON_DOWN_BRUSH = new SolidColorBrush (Color.FromArgb (100, 150, 150, 150));

		void drawDay (int l, int c, int day, bool mainMonth, bool curDay) {
			days[l, c].Text = day.ToString ();
			days[l, c].Foreground = mainMonth ? (curDay ? CURRENT_DAY : CURRENT_MONTH) : OTHERS;
		}

		void UpdateTime ()
		{
			if (current.Month != currentHour.Month || current.Year != currentHour.Year)
				return;
			
			DateTime now = DateTime.Now;
			if (now.Day != currentHour.Day) {
				current = currentHour = now;
				drawCalendar ();
			} else
				current = currentHour = now;
		}

		void drawCalendar () {
	    	Calendar cal = new GregorianCalendar ();
	    	int currentMonthDays = cal.GetDaysInMonth (current.Year, current.Month);
	    	int lastMonthDays = current.Month > 1 ? cal.GetDaysInMonth (current.Year, current.Month - 1) : 31;
	    	int firstDayOfMonth = (int)current.AddDays (1 - current.Day).DayOfWeek;
	    	bool activeMonth = DateTime.Now.Month == current.Month && DateTime.Now.Year == current.Year;

			monthText.Text = current.ToString ("MMMM yyyy");	    	
	    	int first = lastMonthDays - firstDayOfMonth + 1;
			for (int i = 0; i < firstDayOfMonth; ++i)
				drawDay (0, i, first + i, false, false);
			
			int day = 1;
			for (int i = firstDayOfMonth; i < 7; ++i) {
				drawDay (0, i, day, true, day == current.Day && activeMonth);
				++day;
			}

			int line;
			bool curMonth = true;
			for (line = 1; line < 6; ++line) {
				int i;
				for (i = 0; i < 7 && day <= currentMonthDays; ++i) {
					drawDay (line, i, day, curMonth, day == current.Day && activeMonth);
					++day;
					if (day > currentMonthDays) {
						day = 1;
						curMonth = false;
					}
				}
			}
		}
		
		public void MouseDown (Object sender, MouseEventArgs e)
		{
			Shape shape = sender as Shape;
			shape.Fill = shape.Stroke = BUTTON_DOWN_BRUSH;
		}

		public void MouseLeft (Object sender, EventArgs e)
		{
			Shape shape = sender as Shape;
			shape.Fill = shape.Stroke = BUTTON_UP_BRUSH;
		}

		public void PageLoaded (object o, EventArgs e) 
	    {
			monthText = FindName ("month") as TextBlock;
			for (int i = 0; i < 6; ++i)
				for (int j = 0; j < 7; ++j)
					days [i, j] = FindName ("c"+i+"_l"+j) as TextBlock;


			Shape before = FindName ("priorButton") as Shape;
			before.MouseLeftButtonDown += new MouseEventHandler(this.MouseDown);
			before.MouseLeave += new EventHandler(this.MouseLeft);
			before.MouseLeftButtonUp += delegate {
				before.Fill = before.Stroke = BUTTON_UP_BRUSH;
				current = current.AddMonths (-1);
				drawCalendar ();
			};
			
			Shape after = FindName ("nextButton") as Shape;
			after.MouseLeftButtonDown += new MouseEventHandler(this.MouseDown);
			after.MouseLeave += new EventHandler(this.MouseLeft);
			after.MouseLeftButtonUp += delegate {
				after.Fill = after.Stroke = BUTTON_UP_BRUSH;
				current = current.AddMonths (1);
				drawCalendar ();
			};

			Storyboard sb = FindName ("run") as Storyboard;
			DoubleAnimation timer = new DoubleAnimation ();
			sb.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromSeconds (3600));

			sb.Completed += delegate {
				UpdateTime ();
				sb.Begin ();
			};
			sb.Begin ();

			currentHour = current = DateTime.Now;
			drawCalendar ();
		}

	}
}
