//
// Arranging Unit Tests
//
// Author:
//   Moonlight Team (moonlight-list@lists.ximian.com)
// 
// Copyright 2009 Novell, Inc. (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

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
using MoonTest.System.Windows.Controls;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Microsoft.Silverlight.Testing;
using Mono.Moonlight.UnitTesting;

namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class ArrangingTest : SilverlightTest
	{
		static readonly Size infinity = new Size (double.PositiveInfinity, double.PositiveInfinity);

		[TestMethod]
		[MoonlightBug]
		public void ArrangeNoMeasure ()
		{
			LayoutPoker poker = new LayoutPoker ();
			poker.Arrange (new Rect (0, 0, 100, 100));
			Assert.IsTrue (poker.Arranged, "#1");
			Assert.IsFalse (poker.Measured, "#2");
		}

		[TestMethod]
		public void ArrangeOverride_Constraints ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 50, 50);

			LayoutPoker poker = new LayoutPoker {
				MeasureOverrideResult = new Size (50, 50),
				ArrangeOverrideResult = new Size (50, 50)
			};

			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (100, 100), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#2");
			Assert.AreEqual (new Size (50, 50), poker.ArrangeOverrideArg, "#3");
			Assert.AreEqual (new Size (50, 50), poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), new Size (poker.ActualWidth, poker.ActualHeight), "#5");
		}	

		[TestMethod]
		public void ArrangeOverride_Constraints2 ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				MeasureOverrideResult = new Size (50, 50),
				ArrangeOverrideResult = new Size (100, 100)
			};

			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (100, 100), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#2");
			Assert.AreEqual (new Size (100, 100), poker.ArrangeOverrideArg, "#3");
			Assert.AreEqual (new Size (100, 100), poker.RenderSize, "#4");
			Assert.AreEqual (new Size (100, 100), new Size (poker.ActualWidth, poker.ActualHeight), "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void InfMeasure_Unconstrained ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (infinity);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (infinity, poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void InfMeasure_Unconstrained_NoStretch ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (infinity);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (infinity, poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void InfMeasure_Constrained_Smaller ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				Width = 50,
				Height = 50,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (infinity);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (50, 50), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void InfMeasure_Constrained_Smaller_NoStretch ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Width = 50,
				Height = 50,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (infinity);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (50, 50), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void InfMeasure_Constrained_Larger ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (infinity);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (200, 200), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (150, 150), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void InfMeasure_Constrained_Larger_NoStretch ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Width = 150,
				Height = 150,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (infinity);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (150, 150), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (100, 100), poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger2 ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = new Size (50, 50),
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (100, 100), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger2a ()
		{
			Size measureSize = new Size (50, 50);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = new Size (50, 50),
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger2b ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = new Size (50, 50),
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (100, 100), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		public void LargerArrange_Constrained_Larger2c ()
		{
			Size measureSize = new Size (150, 150);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = new Size (50, 50),
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (150, 150), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger2d ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = new Size (50, 50),
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (150, 150), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger3 ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = new Size (150,150),
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (100, 100), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger4 ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = new Size (200, 200),
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (200, 200), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (100, 100), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger5 ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = new Size (250, 250),
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (250, 250), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (100, 100), poker.DesiredSize, "#5");
		}


		[TestMethod]
		public void LargerArrange_Unconstrained ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (measureSize, poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (200, 200), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		public void LargerArrange_Unconstrained_NoStretch ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (measureSize, poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Smaller ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				Width = 50,
				Height = 50,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (50, 50), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Smaller_NoStretch ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Width = 50,
				Height = 50,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (50, 50), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void LargerArrange_Constrained_Larger_NoStretch ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 200, 200);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Width = 150,
				Height = 150,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (100, 100), poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void LargerMeasure_Unconstrained ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (measureSize, poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerMeasure_Unconstrained_NoStretch ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (measureSize, poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void LargerMeasure_Constrained_Smaller ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				Width = 50,
				Height = 50,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (50, 50), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerMeasure_Constrained_Smaller_NoStretch ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Width = 50,
				Height = 50,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (50, 50), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void LargerMeasure_Constrained_Larger ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (200, 200), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (150, 150), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void LargerMeasure_Constrained_Larger_NoStretch ()
		{
			Size measureSize = new Size (200, 200);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Width = 150,
				Height = 150,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (150, 150), poker.DesiredSize, "#5");
		}

		[TestMethod]
		public void SameMeasureAndArrange_Unconstrained ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (measureSize, poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		public void SameMeasureAndArrange_Unconstrained_NoStretch ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (measureSize, poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void SameMeasureAndArrange_Constrained_Larger ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				Width = 150,
				Height = 150,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void SameMeasureAndArrange_Constrained_Larger_NoStretch ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Width = 150,
				Height = 150,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (150, 150), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (new Size (150, 150), poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (measureSize, poker.DesiredSize, "#5");
		}

		[TestMethod]
		[MoonlightBug]
		public void SameMeasureAndArrange_Constrained_Smaller ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				Width = 50,
				Height = 50,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (50, 50), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void SameMeasureAndArrange_Constrained_Smaller_NoStretch ()
		{
			Size measureSize = new Size (100, 100);
			Rect arrangeRect = new Rect (0, 0, 100, 100);

			LayoutPoker poker = new LayoutPoker {
				HorizontalAlignment = HorizontalAlignment.Center,
				VerticalAlignment = VerticalAlignment.Center,
				Width = 50,
				Height = 50,
				MeasureOverrideResult = measureSize,
				ArrangeOverrideResult = measureSize
			};
			poker.Measure (measureSize);
			poker.Arrange (arrangeRect);

			Assert.AreEqual (new Size (50, 50), poker.MeasureOverrideArg, "#1");
			Assert.AreEqual (measureSize, poker.ArrangeOverrideArg, "#2");
			Assert.AreEqual (measureSize, new Size (poker.ActualWidth, poker.ActualHeight), "#3");
			Assert.AreEqual (measureSize, poker.RenderSize, "#4");
			Assert.AreEqual (new Size (50, 50), poker.DesiredSize, "#5");
		}

		class LayoutPoker : Panel
		{
			public bool Arranged { get; private set; }
			public bool Measured { get; private set; }

			public Size ArrangeOverrideArg {
				get; private set;
			}
			public Size ArrangeOverrideResult {
				get; set;
			}
			public Size MeasureOverrideArg {
				get; private set;
			}
			public Size MeasureOverrideResult {
				get; set;
			}

			protected override Size ArrangeOverride (Size finalSize)
			{
				Arranged = true;
				ArrangeOverrideArg = finalSize;
				return ArrangeOverrideResult;
			}

			protected override Size MeasureOverride (Size availableSize)
			{
				Measured = true;
				MeasureOverrideArg = availableSize;
				return MeasureOverrideResult;
			}
		}
	}
}

