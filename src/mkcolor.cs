// mcs mkcolor.cs -pkg:dotnet
using System;
using System.Drawing;
using System.Reflection;
using System.Text;

class Program {
	static void Main ()
	{
		PropertyInfo [] properties = typeof (Color).GetProperties (BindingFlags.Public | BindingFlags.Static);
		// 	{ "black",		0xFF000000 },
		foreach (PropertyInfo pi in properties) {
			Color color = Color.FromName (pi.Name);
			Console.WriteLine ("\t{{ \"{0}\",\t\t0x{1:X2}{2:X2}{3:X2}{4:X2} }},", pi.Name.ToLower (), color.A, color.R, color.G, color.B);
		}
	}
}
