using System.Windows.Shapes;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows;
using System;

public class MyCanvas : Canvas {
	public void PageLoaded (object sender, EventArgs e) {

		Rectangle reflected = FindName ("Reflected") as Rectangle;
		Canvas c = FindName ("Normal") as Canvas;

		VisualBrush vb = new VisualBrush ();
		TransformGroup tg = new TransformGroup ();

		MatrixTransform mt = new MatrixTransform ();
		mt.Matrix = new Matrix (1, 0, 0, -1, 0, 0);
		tg.Children.Add (mt);

		TranslateTransform tt = new TranslateTransform ();
		tt.Y = c.Height;
		tg.Children.Add (tt);

		vb.Transform = tg;

		vb.Visual = c;

		reflected.Fill = vb;
	}
}

