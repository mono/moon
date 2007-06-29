using System;
using System.IO;
using System.Xml;
using System.Xml.Xsl;
using System.Reflection;


namespace svg2xaml
{
	class MainClass
	{
		public static void Main(string[] args)
		{		
			if (args.Length == 0)
			{
				Console.WriteLine ("Usage: svg2xaml input.svg [output.xaml].\n");
				Console.WriteLine ("   If you don't specify an output file, it will be created");
				Console.WriteLine ("   as 'input.svg.xaml' in the location of the svg file.");
				return;
			}
			
			string input = args[0];
			if (!File.Exists (input))
			{
				Console.WriteLine ("Error: file {0} does not exist.");
				Console.WriteLine ("This might work better if you try and convert an svg that actually exists.");
				return;
			}
			
			string output = input + ".xaml";
			if (args.Length > 1)
				output = args[1];
			
			Console.WriteLine ("Converting file {0} into {1}...", input, output);
			
			XmlDocument xsltdoc = new XmlDocument();
			try
			{
				Stream s = Assembly.GetExecutingAssembly().GetManifestResourceStream("svg2xaml.xslt");
				xsltdoc.Load (s);
			}
			catch (FileNotFoundException e)
			{
				Console.WriteLine ("Ooops, can't find the transformation stylesheet. That won't work :p");
				return;
			}
			catch (Exception ex)
			{
				Console.WriteLine (ex.Message);
				return;
			}
			
			try
			{
				XslTransform t = new XslTransform();
				t.Load (xsltdoc);
				t.Transform (input, output);
			}
			catch (Exception ex)
			{
				Console.WriteLine ("Houston, we have a problem!");
				Exception a = ex;
				Console.WriteLine (a.Message);
				Console.WriteLine (a.StackTrace);
				while (a != null)
				{
					Console.WriteLine (a.Message);
					a = ex.InnerException;
				}
			}
			
			Console.WriteLine ("Conversion done, file {0} created. Have a nice day :)", output);
		}
	}
}
