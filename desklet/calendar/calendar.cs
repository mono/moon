using System;
using System.Globalization;

using System.Windows;
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
		
		static readonly Brush CURRENT_DAY = new SolidColorBrush (Color.FromRgb (255, 255, 0));
		static readonly Brush CURRENT_MONTH = new SolidColorBrush (Color.FromRgb (255, 255, 255));
		static readonly Brush OTHERS = new  SolidColorBrush (Color.FromRgb (44, 44, 44));

		void drawDay (int l, int c, int day, bool mainMonth, bool curDay) {
			days[l, c].Text = day.ToString ();
			days[l, c].Foreground = mainMonth ? (curDay ? CURRENT_DAY : CURRENT_MONTH) : OTHERS;
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

		public void PageLoaded (object o, EventArgs e) 
	    {
			Console.WriteLine ("page loaded");
			monthText = FindName ("month") as TextBlock;
			for (int i = 0; i < 6; ++i)
				for (int j = 0; j < 7; ++j)
					days [i, j] = FindName ("c"+i+"_l"+j) as TextBlock;

			current = DateTime.Now; //.AddMonths ( -1);
			drawCalendar ();
			
			(FindName ("beforeButton") as UIElement).MouseLeftButtonUp += delegate {
			current = current.AddMonths (-1);
				drawCalendar ();
			};
			
			(FindName ("afterButton") as UIElement).MouseLeftButtonUp += delegate {
				current = current.AddMonths (1);
				drawCalendar ();
			};

			Console.WriteLine ("widgets loaded");
		}

	}
}
