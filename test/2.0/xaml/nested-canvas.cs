
using System;
using System.Windows;
using System.Windows.Controls;

public class NestedCanvas : Canvas {

	public void Canvas_Loaded (object sender, EventArgs e)
	{
		Canvas child_one = FindName ("child_one") as Canvas;
		Canvas child_two = FindName ("child_two") as Canvas;

		Console.WriteLine ("child one:  '{0}'", child_one);
		Console.WriteLine ("child two:  '{0}'", child_two);

		TextBlock child_one_text = FindName ("child_one_text") as TextBlock;
		TextBlock child_two_text = FindName ("child_two_text") as TextBlock;

		Console.WriteLine ("child one text:  {0}", child_one_text);
		Console.WriteLine ("child two text:  {0}", child_two_text);

		child_one_text = child_one.FindName ("child_one_text") as TextBlock;
		child_two_text = child_two.FindName ("child_two_text") as TextBlock;

		Console.WriteLine ("child one text:  '{0}'", child_one_text);
		Console.WriteLine ("child two text:  '{0}'", child_two_text);
	}
}

