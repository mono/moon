//
// XamlReader Unit Tests
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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
using System.Windows.Data;
using System.Windows.Markup;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System.Windows.Media.Animation;
using System.Windows.Media;

namespace MoonTest.System.Windows.Markup {

	[TestClass]
	public class XamlReaderTest {

		[TestMethod]
		public void Load_Null ()
		{
			Assert.Throws<NullReferenceException> (delegate {
				XamlReader.Load (null);
			}, "null");
		}

		[TestMethod]
		public void Load_Empty ()
		{
			Assert.IsNull (XamlReader.Load (String.Empty), "Empty");
		}

		[TestMethod]
		public void CanvasWithSingleControl ()
		{
			Canvas c = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
       <RepeatButton x:Name=""oops""/>
</Canvas>");
			Assert.IsNotNull (c, "Canvas");
			Assert.AreEqual (1, c.Children.Count, "Count");
			RepeatButton rb = (c.Children [0] as RepeatButton);
			Assert.IsNotNull (rb, "RepeatButton");
			Assert.AreEqual ("oops", rb.Name, "Name");
		}

		[TestMethod]
		[MoonlightBug ("we return an int, not a uint")]
		public void EnumAsContent ()
		{
			DiscreteObjectKeyFrame kf = (DiscreteObjectKeyFrame) XamlReader.Load (@"
<DiscreteObjectKeyFrame xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" >
	<DiscreteObjectKeyFrame.Value>
		<Visibility>Visible</Visibility>
	</DiscreteObjectKeyFrame.Value>
</DiscreteObjectKeyFrame>
");
			Assert.IsNotNull (kf.Value, "#1");
			Assert.AreEqual (0, Convert.ToInt32 (kf.Value), "#2");
			Assert.IsInstanceOfType<uint> (kf.Value, "#3");
		}

		[TestMethod]
		public void DiscreteObjectValuesWithContent ()
		{

			//
			// SolidColorBrush
			// 
			
			// You can't set a the Color of a solid color brush using raw text
            Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<SolidColorBrush xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">#C0C0C0</SolidColorBrush>"), "#1");

			// But you can set it here... somehow!
			var v = (DiscreteObjectKeyFrame)XamlReader.Load (@"
<DiscreteObjectKeyFrame xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<DiscreteObjectKeyFrame.Value>
		<SolidColorBrush>#C0C0C0</SolidColorBrush>
	</DiscreteObjectKeyFrame.Value>
</DiscreteObjectKeyFrame>
");
			Assert.IsInstanceOfType<SolidColorBrush> (v.Value, "#2");
			Assert.AreEqual ("#FFC0C0C0", ((SolidColorBrush) v.Value).Color.ToString (), "#3");


			
			// 
			//  DoubleCollection
			//

			Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<DoubleCollection xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">10,20</DoubleCollection>"), "#4");

			
			v = (DiscreteObjectKeyFrame)XamlReader.Load (@"
<DiscreteObjectKeyFrame xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<DiscreteObjectKeyFrame.Value>
		<DoubleCollection>10,20</DoubleCollection>
	</DiscreteObjectKeyFrame.Value>
</DiscreteObjectKeyFrame>
");
			Assert.IsInstanceOfType<DoubleCollection> (v.Value, "#5");
			Assert.AreEqual (2, ((DoubleCollection) v.Value).Count, "#6");
			Assert.AreEqual (10, ((DoubleCollection) v.Value) [0], "#7");
			Assert.AreEqual (20, ((DoubleCollection) v.Value) [1], "#8");



			//
			// PointCollection
			//

			Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<PointCollection xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">25,35 39,49</PointCollection>"), "#9");

			v = (DiscreteObjectKeyFrame)XamlReader.Load (@"
<DiscreteObjectKeyFrame xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<DiscreteObjectKeyFrame.Value>
		<PointCollection>25,35 39,49</PointCollection>
	</DiscreteObjectKeyFrame.Value>
</DiscreteObjectKeyFrame>
");
			Assert.IsInstanceOfType<PointCollection> (v.Value, "#10");
			Assert.AreEqual (2, ((PointCollection) v.Value).Count, "#11");
			Assert.AreEqual (new Point (25, 35), ((PointCollection) v.Value) [0], "#12");
			Assert.AreEqual (new Point (39, 49), ((PointCollection) v.Value) [1], "#13");



			//
			// Point
			//

			Assert.Throws<XamlParseException>(() => XamlReader.Load (@"
<Point xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">25,35</Point>"), "#14");

			v = (DiscreteObjectKeyFrame)XamlReader.Load (@"
<DiscreteObjectKeyFrame xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<DiscreteObjectKeyFrame.Value>
		<Point>25,35</Point>
	</DiscreteObjectKeyFrame.Value>
</DiscreteObjectKeyFrame>
");
			Assert.IsInstanceOfType<Point> (v.Value, "#15");
			Assert.AreEqual (new Point (25, 35), v.Value, "#16");

		}

		[TestMethod]
		public void StaticResourceToNullable ()
		{
			var canvas = (Canvas) XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<Canvas.Resources>
		<Color x:Key=""res"">#FFEEDDBB</Color>
		<ColorAnimation x:Name=""Anim"" To=""{StaticResource res}"" />
	</Canvas.Resources>
</Canvas>");
			var anim = (ColorAnimation) canvas.Resources ["Anim"];
			Assert.IsTrue (anim.To.HasValue, "#1");
			Assert.AreEqual ("#FFEEDDBB", anim.To.Value.ToString (), "#2");
		}

		[TestMethod]
		public void SetStaticResourceToNonDP ()
		{
			// Using a DependencyObject subclass, apply a StaticResource to a
			// property defined on a base class which is not backed by a DP.
			// Check that the CLR wrapper is invoked
			Base b = CreateBase (@"Color=""{StaticResource Col}""");
			Assert.IsTrue (b.UsedColorSetter, "#1");
			Assert.IsNotNull (b.Color, "#2");
		}

		[TestMethod]
		public void SetBindingToACLRProperty ()
		{
			// Using a DependencyObject subclass, apply a Binding to a
			// property defined on a base class which is not backed by a DP.
			// Check that the CLR wrapper is invoked
			Base b = CreateBase (@"Binding=""{Binding}""");
			Assert.IsTrue (b.UsedBindingSetter, "#1");
			Assert.IsNotNull (b.Binding, "#2");
		}

		[TestMethod]
		public void SetBindingToADPOfTypeBinding ()
		{
			// Using a DependencyObject subclass, apply a Binding to a
			// DP of type Binding, defined on a base class.
			// Check that the CLR wrapper is *not* invoked
			Base b = CreateBase (@"DPBinding=""{Binding}""");
			Assert.IsFalse (b.UsedBindingDPSetter, "#1");
			Assert.IsNull (b.Binding, "#2");
			Assert.IsInstanceOfType<BindingExpression> (b.ReadLocalValue (Base.DPBindingProperty), "#3");
		}

		Base CreateBase (string properties)
		{
			var canvas = (Canvas) XamlReader.Load(string.Format (@"
<Canvas xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
		xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
		xmlns:clr=""clr-namespace:MoonTest.System.Windows.Markup;assembly=moon-unit"">
		<Canvas.Resources>
			<Color x:Key=""Col"">#11223344</Color>
		</Canvas.Resources>
		<clr:Subclass {0} />
</Canvas>", properties));
			Console.WriteLine ("Child {0} is: {0}", canvas.Children[0]);
			return canvas.Children [0] as Base;
		}
	}

	public class Base : Control
	{
		public static readonly DependencyProperty DPBindingProperty =
			DependencyProperty.Register ("DPBinding", typeof (Binding), typeof (Base), null);

		public static readonly DependencyProperty ValueProperty =
			DependencyProperty.Register ("Color", typeof (Color), typeof (Base), null);

		Binding binding;
		public bool UsedBindingSetter {
			get; set;
		}
		public bool UsedBindingDPSetter {
			get; set;
		}
		public bool UsedColorSetter {
			get; set;
		}

		public Binding DPBinding
		{
			get {
				return (Binding) GetValue (DPBindingProperty);
			}
			set {
				UsedBindingDPSetter = true;
				SetValue (DPBindingProperty, value);
			}
		}

		public Binding Binding
		{
			get {
				return binding;
			}
			set {
				UsedBindingSetter = true;
				binding = value;
			}
		}

		public Color Color
		{
			get {
				return (Color) GetValue (ValueProperty);
			}
			set {
				UsedColorSetter = true;
				SetValue (ValueProperty, value);
			}
		}
	}

	public class Subclass : Base
	{

	}
}
