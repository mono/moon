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
using System.Xml; 
using System.Xml.XPath;
using System.Collections.Generic; 

using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklet
{
	public class Entry : IComparable <Entry> {
		private String title;
		private String location;
		private TimeSpan time;

		public Entry (String title, String location, TimeSpan time)
		{
			this.title = title;
			this.location = location;
			this.time = time;
		}

		public string Title { get { return title; } }
		public string Location { get { return location; } }
		public TimeSpan Time { get { return time; } }

		public int CompareTo (Entry other) {
			return this.time.CompareTo (other.time);
		}
	}

	public class DaySchedule
	{
		private List<Entry> entries = new List<Entry> ();
		private DateTime date;
	
		public DaySchedule (DateTime date)
		{
			this.date = date;
		}

		public List<Entry> Appointments { get { return entries; } }
		public DateTime Date { get { return date; } }

		public void Add (Entry entry)
		{
			this.entries.Add (entry);
		}

		public void Sort ()
		{
			this.entries.Sort ();
		}
	}

	public class CalendarPanel : Canvas 
	{
		TextBlock monthText;
		TextBlock detailText;
		TextBlock[,] days = new TextBlock[6,7];
		DateTime current;
		DateTime currentHour;
		List<Rectangle> rectangles = new List<Rectangle> ();
		Rectangle rect;
		Storyboard detailsStoryboard;
		DaySchedule selectedDay;

		static readonly Brush CURRENT_DAY = new SolidColorBrush (Colors.Yellow);
		static readonly Brush CURRENT_MONTH = new SolidColorBrush (Colors.White);
		//Dark grey
		static readonly Brush OTHERS = new SolidColorBrush (Color.FromRgb (44, 44, 44));

		static readonly Brush BUTTON_UP_BRUSH = new SolidColorBrush (Colors.White);
		//Grey
		static readonly Brush BUTTON_DOWN_BRUSH = new SolidColorBrush (Color.FromRgb (150, 150, 150));

		static readonly Brush CALENDAR_BRUSH = new SolidColorBrush (Colors.Red);
		static readonly Brush CALENDAR_FILL_BRUSH = new SolidColorBrush (Color.FromArgb (40, 255, 0, 0));

		Polygon closeButton;
                
                Brush buttonHilite = new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
                Brush buttonNormal = new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));
		
		void drawDay (int l, int c, int day, bool mainMonth, bool curDay) {
			days[l, c].Text = day.ToString ();
			days[l, c].Foreground = mainMonth ? (curDay ? CURRENT_DAY : CURRENT_MONTH) : OTHERS;
		}

		public void DrawCalendarDetails ()
		{
			String text = "";
			foreach (Entry entry in selectedDay.Appointments)
				text += entry.Time.ToString () + "\n" +entry.Title + "\n"+entry.Location+"\n";
			detailText.Text = text;
			Children.Add (detailText);
		}

		public static bool IsSameMonth (DateTime a, DateTime b)
		{
			return a.Month == b.Month && a.Year == b.Year;
		}

		void UpdateTime ()
		{
			if (!IsSameMonth (current, currentHour))
				return;
			
			DateTime now = DateTime.Now;
			if (now.Day != currentHour.Day) {
				current = currentHour = now;
				DrawCalendar ();
			} else
				current = currentHour = now;
		}

		public Dictionary<DateTime, DaySchedule> parseXml (String xml)
		{
			XmlDocument doc = new XmlDocument ();
			doc.LoadXml (xml);
			XmlNamespaceManager man = new XmlNamespaceManager (doc.NameTable);
			man.AddNamespace ("atom", "http://www.w3.org/2005/Atom");
			man.AddNamespace ("gd", "http://schemas.google.com/g/2005");

			Dictionary<DateTime, DaySchedule> dic = new Dictionary<DateTime, DaySchedule> ();
	
			foreach (XmlNode et in doc.SelectNodes ("/atom:feed/atom:entry", man)) {
				XmlNode n =  et.SelectSingleNode ("atom:title[@type='text']", man);
				String title = n != null ?  n.InnerText : "";

				String location = "";
				foreach (XmlNode w in et.SelectNodes ("gd:where/@valueString", man))
					location += (location.Length > 0 ? ", " : "") + w.Value;

				foreach (XmlNode w in et.SelectNodes ("gd:when/@startTime", man)) {
					DateTime time = DateTime.Parse (w.Value);
					DaySchedule sched;
					if (dic.ContainsKey (time.Date))
						sched = dic [time.Date];
					else
						dic.Add (time.Date, sched = new DaySchedule (time.Date));

					sched.Appointments.Add ( new Entry (title, location, time.TimeOfDay));
				}
			}

			foreach (DaySchedule ds in dic.Values) 
				ds.Sort ();
			return dic;
		}

		public void CalendarDownloadComplete (DateTime start, String xml)
		{
			if (!IsSameMonth (start, current))
				return;
			Dictionary<DateTime, DaySchedule> dic = parseXml (xml);
			int dayZero = (int)start.DayOfWeek;

			foreach (DaySchedule ds in dic.Values) {
				int day = ds.Date.Day;
				int x = (day + dayZero - 1) % 7;
				int y = (day + dayZero) / 7 - ((day + dayZero) % 7 == 0 ? 1 : 0);
				Rectangle dayRect = new Rectangle ();
				dayRect.Stroke = CALENDAR_BRUSH;
				dayRect.Fill = CALENDAR_FILL_BRUSH;
				dayRect.StrokeThickness = 1;
				dayRect.Width = 24;
				dayRect.Height = 15;
				dayRect.RadiusX = 4;
				dayRect.RadiusY = 4;
				dayRect.SetValue (Canvas.LeftProperty, 13 + x * 30);
				dayRect.SetValue (Canvas.TopProperty, 65 + y * 17);
				Children.Add (dayRect);
				rectangles.Add (dayRect);
				DaySchedule it = ds;
				dayRect.MouseLeftButtonUp += delegate { OpenCalendarDetailAnimation (x, y, it); };
				
			}			
		}
		
		public void UpdateCalendarData (DateTime start, DateTime end)
		{
			Console.WriteLine ("updating calendar "+start);
			Downloader downloader = new Downloader ();
			downloader.Completed += delegate {
				CalendarDownloadComplete (start.Date, downloader.GetResponseText (""));
			};
			
			String uri = "http://www.google.com/calendar/feeds/ca5i3j9j0gv2j0lln2htfb3tus%40group.calendar.google.com/public/full?start-min="
				+ start.ToString ("yyyy-MM-dd") + "&start-max=" + end.ToString ("yyyy-MM-dd");

			downloader.Open (
				"GET",
				new Uri (uri));
			downloader.Send ();
		}


		void DrawCalendar ()
		{
			Calendar cal = new GregorianCalendar ();
			int currentMonthDays = cal.GetDaysInMonth (current.Year, current.Month);
			int lastMonthDays = current.Month > 1 ? cal.GetDaysInMonth (current.Year, current.Month - 1) : 31;
			DateTime dayOne = current.AddDays (1 - current.Day);
			int firstDayOfMonth = (int)dayOne.DayOfWeek;
			bool activeMonth = IsSameMonth (DateTime.Now, current);

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

			foreach (Rectangle rec in rectangles)
				Children.Remove (rec);
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

		public void OpenCalendarDetailAnimation (int l, int c, DaySchedule ds) {
			int x = (18 + l * 30) + 6;
			int y = (65 + c * 17) + 7;
			rect.Width = 2;
			rect.Height = 2;
			rect.SetValue (Canvas.LeftProperty, x);
			rect.SetValue (Canvas.TopProperty, y);
			Children.Add (rect);

			DoubleAnimation animX = FindName ("anim_x") as DoubleAnimation;
			DoubleAnimation animY = FindName ("anim_y") as DoubleAnimation;
			DoubleAnimation animH = FindName ("anim_h") as DoubleAnimation;
			DoubleAnimation animW = FindName ("anim_w") as DoubleAnimation;
			animX.By = -x;
			animY.By = -y;
			animH.By = Height - rect.Height;
			animW.By = Width - rect.Width;
			selectedDay = ds;

			detailsStoryboard.Begin ();
		}

		void HighlightButton (Polygon button)
                {
                        button.Stroke = buttonHilite;
                }

                void UnhighlightButton (Polygon button)
                {
                        button.Stroke = buttonNormal;
                }
		
		public void PageLoaded (object o, EventArgs e) 
		{
			Gtk.Moonlight.Desklet.SetupToolbox (this);

                        closeButton = FindName ("desklet-close") as Polygon;
                        closeButton.MouseEnter += delegate {
                                HighlightButton (closeButton);
                        };

                        closeButton.MouseLeave += delegate {
                                UnhighlightButton (closeButton);
                        };
			
			monthText = FindName ("month") as TextBlock;
			detailText = FindName ("detail") as TextBlock;
			Children.Remove (detailText);			

			for (int i = 0; i < 6; ++i)
				for (int j = 0; j < 7; ++j)
					days [i, j] = FindName ("c"+i+"_l"+j) as TextBlock;

			this.rect = FindName ("calendar_rect") as Rectangle;
			rect.MouseLeftButtonUp += delegate {
				Children.Remove (detailText);
				Children.Remove (rect);
			};
			Children.Remove (rect);

			Storyboard mouseChange = FindName ("mouse_click") as Storyboard;
			mouseChange.Completed += delegate {
				DateTime dayOne = current.AddDays (1 - current.Day);
				UpdateCalendarData (dayOne, dayOne.AddMonths (1));
			};

			Shape before = FindName ("priorButton") as Shape;
			before.MouseLeftButtonDown += new MouseEventHandler(this.MouseDown);
			before.MouseLeave += new EventHandler(this.MouseLeft);
			before.MouseLeftButtonUp += delegate {
				before.Fill = before.Stroke = BUTTON_UP_BRUSH;
				current = current.AddMonths (-1);
				DrawCalendar ();
				mouseChange.Stop ();
				mouseChange.Begin ();
			};
			
			Shape after = FindName ("nextButton") as Shape;
			after.MouseLeftButtonDown += new MouseEventHandler(this.MouseDown);
			after.MouseLeave += new EventHandler(this.MouseLeft);
			after.MouseLeftButtonUp += delegate {
				after.Fill = after.Stroke = BUTTON_UP_BRUSH;
				current = current.AddMonths (1);
				DrawCalendar ();
				mouseChange.Stop ();
				mouseChange.Begin ();
			};

			Storyboard sb = FindName ("run") as Storyboard;
			sb.Completed += delegate {
				UpdateTime ();
				sb.Begin ();
			};


			detailsStoryboard = FindName ("detail_anim") as Storyboard;
			detailsStoryboard.Completed += delegate { DrawCalendarDetails (); };

			currentHour = current = DateTime.Now;
			DrawCalendar ();
			mouseChange.Begin ();
			sb.Begin ();
		}

	}
}
