using System;
using System.Globalization;

using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklet
{
	public class Calculator : Canvas 
	{
		private string operA = "",operB = "", oper = ""; 
		private string result = "0";
		private bool isOperA = true;
		private bool haveDot = false;
		private bool isRunning = true;

		public Shape button9;
		public TextBlock button9T;
		public Shape button8;
		public TextBlock button8T;
		public Shape button7;
		public TextBlock button7T;
		public Shape button6;
		public TextBlock button6T;
		public Shape button5;
		public TextBlock button5T;
		public Shape button4;
		public TextBlock button4T;
		public Shape button3;
		public TextBlock button3T;
		public Shape button2;
		public TextBlock button2T;
		public Shape button1;
		public TextBlock button1T;
		public Shape button0;
		public TextBlock button0T;
		public Shape buttonPlus;
		public TextBlock buttonPlusT;
		public Shape buttonMinus;
		public TextBlock buttonMinusT;
		public Shape buttonMul;
		public TextBlock buttonMulT;
		public Shape buttonDiv;
		public TextBlock buttonDivT;
		public Shape buttonDot;
		public TextBlock buttonDotT;
		public Shape buttonEqual;
		public TextBlock buttonEqualT;
		public Shape buttonClear;
		public TextBlock buttonClearT;
		public TextBlock Result;

		Polygon closeButton;
		
		Brush buttonHilite = new SolidColorBrush (Color.FromArgb (0xAA, 0xFF, 0xFF, 0xFF));
		Brush buttonNormal = new SolidColorBrush (Color.FromArgb (0x66, 0xFF, 0xFF, 0xFF));
		
		public void PageLoaded (object o, EventArgs e) 
		{
			Mono.Desklets.Desklet.SetupToolbox (this);
			
			button9 = FindName ("9ButtonFrame") as Shape;
			button8 = FindName ("8ButtonFrame") as Shape;
			button7 = FindName ("7ButtonFrame") as Shape;
			button6 = FindName ("6ButtonFrame") as Shape;
			button5 = FindName ("5ButtonFrame") as Shape;
			button4 = FindName ("4ButtonFrame") as Shape;
			button3 = FindName ("3ButtonFrame") as Shape;
			button2 = FindName ("2ButtonFrame") as Shape;
			button1 = FindName ("1ButtonFrame") as Shape;
			button0 = FindName ("0ButtonFrame") as Shape;
			buttonPlus = FindName ("PlusButtonFrame") as Shape;
			buttonMinus = FindName ("MinusButtonFrame") as Shape;
			buttonMul = FindName ("MulButtonFrame") as Shape;
			buttonDiv = FindName ("DivButtonFrame") as Shape;
			buttonDot = FindName ("DotButtonFrame") as Shape;
			buttonEqual = FindName ("EqualButtonFrame") as Shape;
			buttonClear = FindName ("ClearFrame") as Shape;

			button9T = FindName ("9Button") as TextBlock;
			button8T = FindName ("8Button") as TextBlock;
			button7T = FindName ("7Button") as TextBlock;
			button6T = FindName ("6Button") as TextBlock;
			button5T = FindName ("5Button") as TextBlock;
			button4T = FindName ("4Button") as TextBlock;
			button3T = FindName ("3Button") as TextBlock;
			button2T = FindName ("2Button") as TextBlock;
			button1T = FindName ("1Button") as TextBlock;
			button0T = FindName ("0Button") as TextBlock;
			buttonPlusT = FindName ("PlusButton") as TextBlock;
			buttonMinusT = FindName ("MinusButton") as TextBlock;
			buttonMulT = FindName ("MulButton") as TextBlock;
			buttonDivT = FindName ("DivButton") as TextBlock;
			buttonDotT = FindName ("DotButton") as TextBlock;
			buttonEqualT = FindName ("EqualButton") as TextBlock;
			buttonClearT = FindName ("Clear") as TextBlock;

			closeButton = FindName ("desklet-close") as Polygon;
			closeButton.MouseEnter += delegate {
				HighlightButton (closeButton);
			};

			closeButton.MouseLeave += delegate {
				UnhighlightButton (closeButton);
			};
			
			Result = FindName ("ResultFrame") as TextBlock;

			this.MouseEvents ();

			Storyboard sb = FindName ("run") as Storyboard;
			DoubleAnimation timer = new DoubleAnimation ();
			sb.Children.Add (timer);
			timer.Duration = new Duration (TimeSpan.FromSeconds (3600));

			sb.Completed += delegate {
				sb.Begin ();
			};
			sb.Begin ();
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

		private void MouseEvents ()
		{
			button9.MouseLeftButtonUp += delegate {
				Method ("9",false);
			};
			button8.MouseLeftButtonUp += delegate {
				Method ("8",false);
			};
			button7.MouseLeftButtonUp += delegate {
				Method ("7",false);
			};
			button6.MouseLeftButtonUp += delegate {
				Method ("6",false);
			};
			button5.MouseLeftButtonUp += delegate {
				Method ("5",false);
			};
			button4.MouseLeftButtonUp += delegate {
				Method ("4",false);
			};
			button3.MouseLeftButtonUp += delegate {
				Method ("3",false);
			};
			button2.MouseLeftButtonUp += delegate {
				Method ("2",false);
			};
			button1.MouseLeftButtonUp += delegate {
				Method ("1",false);
			};       
			button0.MouseLeftButtonUp += delegate {
				Method ("0",false);
			};
			buttonPlus.MouseLeftButtonUp += delegate {
				Method ("+",true);
			};
			buttonMinus.MouseLeftButtonUp += delegate {
				Method ("-",true);
			};
			buttonMul.MouseLeftButtonUp += delegate {
				Method ("*",true);
			};       
			buttonDiv.MouseLeftButtonUp += delegate {
				Method ("/",true);
			};
			buttonDot.MouseLeftButtonUp += delegate {
				Method (".",false);
			};
			buttonEqual.MouseLeftButtonUp += delegate {
				Equal ();
			};
			buttonClear.MouseLeftButtonUp += delegate {
				Clear ();
			};
			button9T.MouseLeftButtonUp += delegate {
				Method ("9",false);
			};
			button8T.MouseLeftButtonUp += delegate {
				Method ("8",false);
			};
			button7T.MouseLeftButtonUp += delegate {
				Method ("7",false);
			};
			button6T.MouseLeftButtonUp += delegate {
				Method ("6",false);
			};
			button5T.MouseLeftButtonUp += delegate {
				Method ("5",false);
			};
			button4T.MouseLeftButtonUp += delegate {
				Method ("4",false);
			};
			button3T.MouseLeftButtonUp += delegate {
				Method ("3",false);
			};
			button2T.MouseLeftButtonUp += delegate {
				Method ("2",false);
			};
			button1T.MouseLeftButtonUp += delegate {
				Method ("1",false);
			};       
			button0T.MouseLeftButtonUp += delegate {
				Method ("0",false);
			};
			buttonPlusT.MouseLeftButtonUp += delegate {
				Method ("+",true);
			};
			buttonMinusT.MouseLeftButtonUp += delegate {
				Method ("-",true);
			};
			buttonMulT.MouseLeftButtonUp += delegate {
				Method ("*",true);
			};       
			buttonDivT.MouseLeftButtonUp += delegate {
				Method ("/",true);
			};
			buttonDotT.MouseLeftButtonUp += delegate {
				Method (".",false);
			};
			buttonEqualT.MouseLeftButtonUp += delegate {
				Equal ();
			};
			buttonClearT.MouseLeftButtonUp += delegate {
				Clear ();
			};
		}
	}
}
