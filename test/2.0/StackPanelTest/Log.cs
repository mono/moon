using System;
using System.Windows.Controls;
using System.Windows.Media;

namespace DebugLog
{
	public class Log : Canvas
	{
		static Log standard;
		TextBlock display;

		public static Log Standard 
		{
			get {
				if (standard == null)
					standard = new Log ();
				
				return standard;
			}
		}

		public Log ()
		{
			display = new TextBlock ();
			display.Foreground = new SolidColorBrush (Colors.Black);
			Children.Add (display);
		}

	        void WriteLine (string message)
		{
			message += "\n";
			Console.Write (message);
			display.Text += message;
		}

		public static void WriteLine (string format, params object [] args)
		{
			Standard.WriteLine (String.Format (format, args));
		}

		public static void WriteLine (object obj)
		{
			Standard.WriteLine (obj.ToString ());
		}
	}
}
