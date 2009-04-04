using System;
using System.IO;
using System.Globalization;

using System.Windows;
using System.Windows.Input;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklets.CpuMonitorPanel
{
	public struct CpuCounter {
		long user;
		long nice;
		long system;
		long idle;
		long iowait;
		long irq;
		long softirq;
		long steal;
		long total;
		
		public void Read (String line) {
			String[] parts = line.Split (new char[] {' '}, StringSplitOptions.RemoveEmptyEntries);
			total += (user = long.Parse (parts [1]));
			total += (nice = long.Parse (parts [2]));
			total += (system = long.Parse (parts [3]));
			total += (idle = long.Parse (parts [4]));
			total += (iowait = long.Parse (parts [5]));
			total += (irq = long.Parse (parts [6]));
			total += (softirq = long.Parse (parts [7]));
			total += (steal = long.Parse (parts [8]));
		}

		public CpuCounter Sub (ref CpuCounter other) {
			CpuCounter res = this;
			res.user -= other.user;
			res.nice -= other.nice;
			res.system -= other.system;
			res.idle -= other.idle;
			res.iowait -= other.iowait;
			res.irq -= other.irq;
			res.softirq -= other.softirq;
			res.steal -= other.steal;
			res.total -= other.total;
			return res;
		}

		public void FetchGlobalCounters() {
			using ( StreamReader sr = new StreamReader ("/proc/stat")) {
				String line = sr.ReadLine ();
				Read (line);
			}
		}
		
		public double CpuLoad () {
			return 100d * ((double)(total - idle) / total);
		}
	}

	public partial class CpuMonitorPanel : Canvas 
	{
		Shape circle;
		TextBlock load;
		CpuCounter last;
		ColorAnimation colorAnim;
		Storyboard colorSb;

		Polygon closeButton;
		
		Brush buttonHilite = new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
		Brush buttonNormal = new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));
		
		public void DrawLoad ()
		{
			CpuCounter cur = new CpuCounter ();
			cur.FetchGlobalCounters ();
			CpuCounter delta = cur.Sub (ref last);
			last = cur;
    		
			double num = Math.Round (delta.CpuLoad ());
			load.Text = ((int)num).ToString ();
			Color current = (circle.Fill as SolidColorBrush).Color;
			Color color = new Color ();

			if (num <= 50) {
				//interpolate (0,50) between green (0,255,0) and yellow (255,255,0)
				double red = num / (50d / 255);
				color = Color.FromArgb (255, (byte)red, 255, 0);
			} else {
				//interpolate (50,100) between yellow (255,255,0) and red (255,0,0)
				double green = (100d - num) / (50d / 255);
				color = Color.FromArgb (255, 255, (byte)green, 0);
			}

			colorAnim.From = current;
			colorAnim.To = color;
			colorSb.Begin ();
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
			Moonlight.Gtk.Desklet.SetupToolbox (this);

			closeButton = FindName ("desklet-close") as Polygon;

			closeButton.MouseEnter += delegate {
				HighlightButton (closeButton);
			};

			closeButton.MouseLeave += delegate {
				UnhighlightButton (closeButton);
			};
			
			circle = FindName ("Circle") as Shape;
			load = FindName ("Load") as TextBlock;
			colorSb = FindName ("color_sb") as Storyboard;
			colorAnim = FindName ("color_anim") as  ColorAnimation;
			last = new CpuCounter ();

			if (circle == null)
				Console.WriteLine ("no circle ="+(FindName ("Circle") != null));

			Storyboard sb = FindName ("run") as Storyboard;
			DoubleAnimation timer = new DoubleAnimation ();
			sb.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromMilliseconds (100));

			sb.Completed += delegate {
				DrawLoad ();
				sb.Begin ();
			};
			sb.Begin ();
			DrawLoad ();
		}
		public CpuMonitorPanel () {
			InitializeComponent();
		}
	}
}
