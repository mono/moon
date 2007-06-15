using System.Windows.Controls;
using System.Windows.Shapes;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows;
using System;

public abstract class TestBase
{
	public Canvas Canvas
	{
		get { return canvas; } 
	}
	protected Canvas canvas;
}
