using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Reflection;
using System.Text;

using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Mono.Moonlight.UnitTesting
{
	public static class Tester
	{
		static StringBuilder test_output = new StringBuilder ();

		public static void Write (string text)
		{
			Console.Write (text);
			//test_output.Append (text);
		}

		public static void WriteLine (string line)
		{
			Console.WriteLine (line);
			//test_output.AppendLine (line);
		}
	}
}
