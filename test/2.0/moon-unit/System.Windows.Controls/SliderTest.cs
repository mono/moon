//
// Slider Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008,2009 Novell, Inc.
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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;

using Mono.Moonlight.UnitTesting;

using Microsoft.VisualStudio.TestTools.UnitTesting;
using MoonTest.System.Windows.Controls.Primitives;

namespace MoonTest.System.Windows.Controls {

	[TestClass]
	public class SliderTest {

		[TestMethod]
		public void DefaultProperties ()
		{
			Slider s = new Slider ();
			// default properties on Slider
			CheckDefaultProperties (s, 1.0);
		}

		public static void CheckDefaultProperties (Slider s, double maximum)
		{
			Assert.IsFalse (s.IsDirectionReversed, "IsDirectionReversed");
			Assert.IsFalse (s.IsFocused, "IsFocused");
			Assert.AreEqual (Orientation.Horizontal, s.Orientation, "Orientation");

			// default properties on RangeBase...
			RangeBaseTest.CheckDefaultProperties (s, maximum);
		}

		[TestMethod]
		public void Properties ()
		{
			Slider s = new Slider ();
			// default properties on Slider
			s.IsDirectionReversed = !s.IsDirectionReversed;

			Assert.AreEqual (Orientation.Horizontal, s.Orientation, "Orientation");
			s.Orientation = (Orientation) Int32.MaxValue;
			Assert.AreEqual ((Orientation) Int32.MaxValue, s.Orientation, "MaxValue");
		}

		[TestMethod]
		public void ReadOnlyProperties ()
		{
			Slider s = new Slider ();
			Assert.Throws<InvalidOperationException> (delegate {
				s.SetValue (Thumb.IsFocusedProperty, true);
			}, "IsFocusedProperty");
		}

		[TestMethod]
		public void ToStringTest ()
		{
			Slider s = new Slider ();
			Assert.AreEqual ("System.Windows.Controls.Slider Minimum:0 Maximum:1 Value:0", s.ToString (), "ToString");
		}

		public class SliderPoker : Slider {

			public DependencyObject GetTemplateChild_ (string s)
			{
				return base.GetTemplateChild (s);
			}

			public double OldMaximum;
			public double NewMaximum;
			public double OldMinimum;
			public double NewMinimum;
			public double OldValue;
			public double NewValue;

			protected override void OnMaximumChanged (double oldMaximum, double newMaximum)
			{
				OldMaximum = oldMaximum;
				NewMaximum = newMaximum;
				base.OnMaximumChanged (oldMaximum, newMaximum);
			}

			protected override void OnMinimumChanged (double oldMinimum, double newMinimum)
			{
				OldMinimum = oldMinimum;
				NewMinimum = newMinimum;
				base.OnMinimumChanged (oldMinimum, newMinimum);
			}

			protected override void OnValueChanged (double oldValue, double newValue)
			{
				OldValue = oldValue;
				NewValue = newValue;
				base.OnValueChanged (oldValue, newValue);
			}
		}

		[TestMethod]
		public void OnMethods ()
		{
			// copied from RangeBaseTest to confirm identical behavior
			SliderPoker rb = new SliderPoker ();

			rb.Maximum = 0.5;
			Assert.AreEqual (1.0, rb.OldMaximum, "OldMaximum");
			Assert.AreEqual (rb.Maximum, rb.NewMaximum, "NewMaximum");

			rb.Minimum = 0.5;
			Assert.AreEqual (0.0, rb.OldMinimum, "OldMinimum");
			Assert.AreEqual (rb.Minimum, rb.NewMinimum, "NewMinimum");

			rb.Value = 0.5;
			Assert.AreEqual (0.0, rb.OldValue, "OldValue");
			Assert.AreEqual (rb.Value, rb.NewValue, "NewValue");
		}

		[TestMethod]
		public void Events ()
		{
			// copied from RangeBaseTest to confirm identical behavior
			SliderPoker rb = new SliderPoker ();
			double old = rb.Value;
			rb.ValueChanged += delegate (object sender, RoutedPropertyChangedEventArgs<double> e) {
				Assert.AreSame (rb, sender, "sender");
				Assert.IsNull (e.OriginalSource, "OriginalSource");
				Assert.AreEqual (rb.Value, e.NewValue, "NewValue");
				Assert.AreEqual (old, e.OldValue, "OldValue");
			};
			rb.Value = 1.0;
		}

		[TestMethod]
		public void DefaultTemplateChilds ()
		{
			SliderPoker s = new SliderPoker ();
			CheckEmptyTemplateChilds (s);
		}

		[TestMethod]
		public void NothingAppliedTemplateChilds ()
		{
			SliderPoker s = new SliderPoker ();
			s.ApplyTemplate ();
			CheckEmptyTemplateChilds (s);
		}

		private void CheckEmptyTemplateChilds (SliderPoker s)
		{
			Assert.IsNull (s.GetTemplateChild_ ("RootElement"), "RootElement");
			Assert.IsNull (s.GetTemplateChild_ ("HorizontalTemplateElement"), "HorizontalTemplateElement");
			Assert.IsNull (s.GetTemplateChild_ ("HorizontalTrackLargeChangeIncreaseRepeatButtonElement"), "HorizontalTrackLargeChangeIncreaseRepeatButtonElement");
			Assert.IsNull (s.GetTemplateChild_ ("HorizontalTrackLargeChangeDecreaseRepeatButtonElement"), "HorizontalTrackLargeChangeDecreaseRepeatButtonElement");
			Assert.IsNull (s.GetTemplateChild_ ("HorizontalThumbElement"), "HorizontalThumbElement");
			Assert.IsNull (s.GetTemplateChild_ ("VerticalTemplateElement"), "VerticalTemplateElement");
			Assert.IsNull (s.GetTemplateChild_ ("VerticalTrackLargeChangeIncreaseRepeatButtonElement"), "VerticalTrackLargeChangeIncreaseRepeatButtonElement");
			Assert.IsNull (s.GetTemplateChild_ ("VerticalTrackLargeChangeDecreaseRepeatButtonElement"), "VerticalTrackLargeChangeDecreaseRepeatButtonElement");
			Assert.IsNull (s.GetTemplateChild_ ("VerticalThumbElement"), "VerticalThumbElement");
			Assert.IsNull (s.GetTemplateChild_ ("FocusVisualElement"), "FocusVisualElement");
			Assert.IsNull (s.GetTemplateChild_ ("Normal State"), "Normal State");
			Assert.IsNull (s.GetTemplateChild_ ("MouseOver State"), "MouseOver State");
			Assert.IsNull (s.GetTemplateChild_ ("Disabled State"), "Disabled State");
		}

		[TestMethod]
		[MoonlightBug ("should work once templates are applied")]
		public void MinimalXaml ()
		{
			Canvas c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
       <Slider/>
</Canvas>");
			Slider s = c.Children [0] as Slider;
			Assert.IsFalse (s.IsDirectionReversed, "IsDirectionReversed");
			Assert.IsFalse (s.IsFocused, "IsFocused");
			Assert.AreEqual (Orientation.Horizontal, s.Orientation, "Orientation");

			// default properties on Control
			Assert.IsNull (s.Background, "Background");
			Assert.IsNotNull (s.BorderBrush, "BorderBrush");
			Assert.IsTrue (s.BorderBrush is LinearGradientBrush, "BorderBrush/LinearGradientBrush");
			Assert.AreEqual (new Thickness (1, 1, 1, 1), s.BorderThickness, "BorderThickness");
			Assert.IsNotNull (s.FontFamily, "FontFamily");
			Assert.AreEqual (FontStretches.Normal, s.FontStretch, "FontStretch");
			Assert.AreEqual (FontStyles.Normal, s.FontStyle, "FontStyle");
			Assert.AreEqual (FontWeights.Normal, s.FontWeight, "FontWeight");
			Assert.IsNotNull (s.Foreground, "Foreground");
			Assert.IsTrue (s.Foreground is SolidColorBrush, "Foreground/SolidColorBrush");
			Assert.AreEqual (Colors.Black, (s.Foreground as SolidColorBrush).Color, "Foreground.Color");
			Assert.AreEqual (HorizontalAlignment.Center, s.HorizontalContentAlignment, "HorizontalContentAlignment");
			Assert.IsTrue (s.IsEnabled, "IsEnabled");
			Assert.IsFalse (s.IsTabStop, "IsTabStop");
			Assert.AreEqual (new Thickness (0, 0, 0, 0), s.Padding, "Padding");
			Assert.AreEqual (Int32.MaxValue, s.TabIndex, "AreEqual");
			Assert.AreEqual (KeyboardNavigationMode.Local, s.TabNavigation, "TabNavigation");
			Assert.IsNotNull (s.Template, "Template");
			Assert.AreEqual (typeof (Slider), (s.Template as ControlTemplate).TargetType, "Template/TargetType");
			Assert.AreEqual (VerticalAlignment.Center, s.VerticalContentAlignment, "VerticalContentAlignment");

			FrameworkElementTest.CheckDefaultProperties (s, c);
		}
	}
}
