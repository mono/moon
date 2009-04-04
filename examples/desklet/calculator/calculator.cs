using System;
using System.Globalization;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

using Moonlight.Gtk;

namespace Desklet.Calculator
{
	public partial class Calculator : Canvas 
	{
		private string operA = "",operB = "", oper = ""; 
		private string result = "0";
		private bool isOperA = true;
		private bool haveDot = false;
		private bool isRunning = true;

		Brush buttonHilite = new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
		Brush buttonNormal = new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));

		public Calculator ()
		{
			InitializeComponent ();

			Moonlight.Gtk.Desklet.SetupToolbox (this);
			
			desklet_close.MouseEnter += delegate {
				HighlightButton (desklet_close);
			};

			desklet_close.MouseLeave += delegate {
				UnhighlightButton (desklet_close);
			};
			
			this.AddMouseEvents ();

			DataTemplate dataTemplate = (DataTemplate)Resources["hithere"];

			DependencyObject obj = dataTemplate.LoadContent ();
			Console.WriteLine ("called dataTemplate.LoadContent.  result was {0}", obj);
		}

		void HighlightButton (Polygon button)
		{
			button.Stroke = buttonHilite;
		}

		void UnhighlightButton (Polygon button)
		{
			button.Stroke = buttonNormal;
		}
		
		private void Method (string str, bool op)
		{
			if (!isRunning) return;
			if (op)
			{
				isOperA = false;
				oper = str;
				haveDot = false;
				return;
			}
			if (isOperA)
			{
				if ((str == ".")&&(haveDot)) return;
				if (str == ".") haveDot = true;
				operA += str;
				Result.Text = operA;
			}
			else 
			{
				if ((str == ".")&&(haveDot)) return;
				if (str == ".") haveDot = true;   
				operB += str;
				Result.Text = operB;
			}
			//Console.WriteLine (operA + " " + oper + " " + operB);

		}

		private void Clear ()
		{
			isOperA = true;
			haveDot = false;
			operA = operB = oper = "";
			result = "0";
			Result.Text = result;
			isRunning = true;
		}

		private void Equal ()
		{
			isRunning = false;
			float res = 0;
			switch (oper)
			{
				case "+": res = Convert.ToSingle (operA) + Convert.ToSingle (operB); break;
				case "-": res = Convert.ToSingle (operA) - Convert.ToSingle (operB); break;
				case "*": res = Convert.ToSingle (operA) * Convert.ToSingle (operB); break;
				case "/": res = Convert.ToSingle (operA) / Convert.ToSingle (operB); break;
			}
			result = res.ToString ("R",null);
			//Console.WriteLine (result);
			Result.Text = result;
		}

		private void AddMouseEvents ()
		{
			Button9.MouseLeftButtonUp += delegate {
				Method ("9",false);
			};
			Button8.MouseLeftButtonUp += delegate {
				Method ("8",false);
			};
			Button7.MouseLeftButtonUp += delegate {
				Method ("7",false);
			};
			Button6.MouseLeftButtonUp += delegate {
				Method ("6",false);
			};
			Button5.MouseLeftButtonUp += delegate {
				Method ("5",false);
			};
			Button4.MouseLeftButtonUp += delegate {
				Method ("4",false);
			};
			Button3.MouseLeftButtonUp += delegate {
				Method ("3",false);
			};
			Button2.MouseLeftButtonUp += delegate {
				Method ("2",false);
			};
			Button1.MouseLeftButtonUp += delegate {
				Method ("1",false);
			};       
			Button0.MouseLeftButtonUp += delegate {
				Method ("0",false);
			};
			ButtonPlus.MouseLeftButtonUp += delegate {
				Method ("+",true);
			};
			ButtonMinus.MouseLeftButtonUp += delegate {
				Method ("-",true);
			};
			ButtonMult.MouseLeftButtonUp += delegate {
				Method ("*",true);
			};       
			ButtonDiv.MouseLeftButtonUp += delegate {
				Method ("/",true);
			};
			ButtonDot.MouseLeftButtonUp += delegate {
				Method (".",false);
			};
			ButtonEqual.MouseLeftButtonUp += delegate {
				Equal ();
			};
			ButtonClear.MouseLeftButtonUp += delegate {
				Clear ();
			};
		}
	}
}
