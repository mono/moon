using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using System.Collections.Generic;
using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;


namespace MoonTest.System.Windows.Controls
{
	[TestClass]
	public class ControlTest
	{
		class ControlPoker : UserControl {
			public void SetContent (UIElement ui)
			{
				Content = ui;
			}
		}

		class DefaultStyleKey_TypeClass : UserControl {
			public DefaultStyleKey_TypeClass ()
			{
				DefaultStyleKey = typeof (DefaultStyleKey_TypeClass);
			}
		}

		class DefaultStyleKey_TypeClass2 : UserControl {
			public DefaultStyleKey_TypeClass2 ()
			{
				DefaultStyleKey = typeof (UserControl);
			}
		}

		class DefaultStyleKey_NullClass : UserControl {
			public DefaultStyleKey_NullClass ()
			{
				DefaultStyleKey = null;
			}
		}

		class DefaultStyleKey_DifferentTypeClass : UserControl {
			public DefaultStyleKey_DifferentTypeClass ()
			{
				DefaultStyleKey = typeof (Button);
			}
		}

		class DefaultStyleKey_NonDOTypeClass : UserControl {
			public DefaultStyleKey_NonDOTypeClass ()
			{
				DefaultStyleKey = typeof (string);
			}
		}

		class DefaultStyleKey_NonTypeClass : UserControl {
			public DefaultStyleKey_NonTypeClass ()
			{
				DefaultStyleKey = "hi";
			}
		}

		class ConcreteControl : Control {

			public object DefaultStyleKey_ {
				get { return base.DefaultStyleKey; }
				set { base.DefaultStyleKey = value; }
			}


			public DependencyObject GetTemplateChild_ (string s)
			{
				return base.GetTemplateChild (s);
			}

			public bool GotFocusCalled = false;
			public bool LostFocusCalled = false;

			protected override void OnGotFocus (RoutedEventArgs e)
			{
				GotFocusCalled = true;
				base.OnGotFocus (e);
			}

			protected override void OnLostFocus (RoutedEventArgs e)
			{
				LostFocusCalled = true;
				base.OnLostFocus (e);
			}

			static public DependencyProperty DefaultStyleKeyProperty_
			{
				get { return Control.DefaultStyleKeyProperty; }
			}

			public void OnGotFocus_ (RoutedEventArgs e)
			{
				base.OnGotFocus (e);
			}

			public void OnLostFocus_ (RoutedEventArgs e)
			{
				base.OnLostFocus (e);
			}

			public void OnKeyDown_ (KeyEventArgs e)
			{
				base.OnKeyDown (e);
			}

			public void OnKeyUp_ (KeyEventArgs e)
			{
				base.OnKeyUp (e);
			}

			public void OnMouseEnter_ (MouseEventArgs e)
			{
				base.OnMouseEnter (e);
			}

			public void OnMouseLeave_ (MouseEventArgs e)
			{
				base.OnMouseLeave (e);
			}

			public void OnMouseMove_ (MouseEventArgs e)
			{
				base.OnMouseMove (e);
			}

			public void OnMouseLeftButtonDown_ (MouseButtonEventArgs e)
			{
				base.OnMouseLeftButtonDown (e);
			}

			public void OnMouseLeftButtonUp_ (MouseButtonEventArgs e)
			{
				base.OnMouseLeftButtonUp (e);
			}
		}

		class MoreConcreteControl : ConcreteControl {
		}

		class SiblingControl : Control {
		}

		[TestMethod]
		[MoonlightBug]
		public void DefaultStyleKeyTest_NotWorking ()
		{
			Assert.Throws (delegate { DefaultStyleKey_TypeClass tc = new DefaultStyleKey_TypeClass (); },
				       typeof (ArgumentException), "1");
			Assert.Throws (delegate { DefaultStyleKey_TypeClass2 tc = new DefaultStyleKey_TypeClass2 (); },
				       typeof (ArgumentException), "2");
			Assert.Throws (delegate { DefaultStyleKey_DifferentTypeClass dtc = new DefaultStyleKey_DifferentTypeClass (); },
				       typeof (InvalidOperationException), "4");
		}

		public void DefaultStyleKeyTest_Working ()
		{
			Assert.Throws (delegate { DefaultStyleKey_NonDOTypeClass ndotc = new DefaultStyleKey_NonDOTypeClass (); },
				       typeof (ArgumentException), "5");
			Assert.Throws (delegate { DefaultStyleKey_NonTypeClass ntc = new DefaultStyleKey_NonTypeClass (); },
				       typeof (ArgumentException), "6");
		}

		[TestMethod]
		public void DefaultStyleKeyTest_Null ()
		{
			ConcreteControl c = new ConcreteControl ();
			Assert.IsNull (c.DefaultStyleKey_, "null");

			// issue here is that we can't assign the current (null) value without an exception
			// but the PropertyChange logic is "smart" enough not to allow this...
			Assert.Throws<ArgumentException> (delegate {
				c.DefaultStyleKey_ = null;
			}, "null");

			// ... and guess what it's not part of the PropertyChange validation!
			c.SetValue (ConcreteControl.DefaultStyleKeyProperty_, null);
		}

		[TestMethod]
		public void DefaultStyleKeyTest_More ()
		{
			ConcreteControl c = new ConcreteControl ();
			Assert.IsNull (c.DefaultStyleKey_, "null");

			// and some working tests
			c.DefaultStyleKey_ = typeof (ConcreteControl);
			Assert.AreEqual (typeof (ConcreteControl), c.DefaultStyleKey_, "DefaultStyleKey");

			MoreConcreteControl mc = new MoreConcreteControl ();
			mc.DefaultStyleKey_ = typeof (ConcreteControl);
			Assert.AreEqual (typeof (ConcreteControl), mc.DefaultStyleKey_, "DefaultStyleKey-Base");

			c = new ConcreteControl ();
			c.DefaultStyleKey_ = typeof (MoreConcreteControl);
			Assert.AreEqual (typeof (MoreConcreteControl), c.DefaultStyleKey_, "DefaultStyleKey-Inherited");

			mc = new MoreConcreteControl ();
			mc.DefaultStyleKey_ = typeof (SiblingControl);
			Assert.AreEqual (typeof (SiblingControl), mc.DefaultStyleKey_, "DefaultStyleKey-Sibling");

			mc = new MoreConcreteControl ();
			Assert.Throws<ArgumentException> (delegate {
				mc.DefaultStyleKey_ = typeof (Control);
			}, "Control");
		}

		[TestMethod]
		public void DefaultDesiredSizeTest ()
		{
 			ControlPoker p = new ControlPoker ();
 			Assert.AreEqual (new Size (0,0), p.DesiredSize);
		}

		[TestMethod]
		public void ChildlessMeasureTest ()
		{
 			ControlPoker p = new ControlPoker ();

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (new Size (0,0), p.DesiredSize, "DesiredSize");
		}

		[TestMethod]
		public void ChildlessMarginMeasureTest ()
		{
 			ControlPoker p = new ControlPoker ();

			p.Margin = new Thickness (10);

			p.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), p.DesiredSize, "DesiredSize");
		}
		
		[TestMethod]
		public void ChildlessMinWidthMeasureTest1 ()
		{
 			ControlPoker p = new ControlPoker ();

			p.MinWidth = 50;

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (10, p.DesiredSize.Width);
		}

		[TestMethod]
		public void ChildlessMinWidthMeasureTest2 ()
		{
 			ControlPoker p = new ControlPoker ();

			p.MinWidth = 5;

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (5, p.DesiredSize.Width);
		}

		[TestMethod]
		public void ChildlessMinHeightMeasureTest1 ()
		{
 			ControlPoker p = new ControlPoker ();

			p.MinHeight = 50;

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (10, p.DesiredSize.Height);
		}

		[TestMethod]
		public void ChildlessMinHeightMeasureTest2 ()
		{
 			ControlPoker p = new ControlPoker ();

			p.MinHeight = 5;

			Size s = new Size (10,10);

			p.Measure (s);

			Assert.AreEqual (5, p.DesiredSize.Height);
		}

		[TestMethod]
		public void ChildMeasureTest1 ()
		{
			ControlPoker p = new ControlPoker ();
			Rectangle r = new Rectangle();

			p.SetContent (r);

			r.Width = 50;
			r.Height = 50;

			p.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), p.DesiredSize);
		}

		[TestMethod]
		public void ChildMeasureTest2 ()
		{
			ControlPoker p = new ControlPoker ();
			Rectangle r = new Rectangle();

			p.SetContent (r);

			r.Width = 50;
			r.Height = 50;

			p.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (50,50), p.DesiredSize);
		}

		[TestMethod]
		public void ChildThicknessMeasureTest1 ()
		{
			ControlPoker p = new ControlPoker ();
			Rectangle r = new Rectangle();

			p.Margin = new Thickness (5);
			p.SetContent (r);

			r.Width = 50;
			r.Height = 50;

			p.Measure (new Size (10, 10));

			Assert.AreEqual (new Size (10,10), p.DesiredSize);
		}

		[TestMethod]
		public void ChildThicknessMeasureTest2 ()
		{
			ControlPoker p = new ControlPoker ();
			Rectangle r = new Rectangle();

			p.Margin = new Thickness (5);
			p.SetContent (r);

			r.Width = 50;
			r.Height = 50;

			p.Measure (new Size (100, 100));

			Assert.AreEqual (new Size (60,60), p.DesiredSize);
		}

		[TestMethod]
		public void GetTemplateChildTest ()
		{
			ConcreteControl c = new ConcreteControl ();
			Assert.Throws<ArgumentException> (delegate {
				c.GetTemplateChild_ (null);
			}, "null");
			Assert.IsNull (c.GetTemplateChild_ (String.Empty), "Empty");
		}

		[TestMethod]
		public void ChildNameScope ()
		{
			ControlPoker b = new ControlPoker ();
		        Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Border>
    <Path x:Name=""foo"" Data=""F1 M 10,10 20,20 10,20"" Stroke=""Red""/>
  </Border>
</Canvas>");
			Assert.IsNotNull (c.FindName ("foo"),"c before");
			
			b.SetContent (c);
			
			Assert.IsNull (b.FindName ("foo"),"b after");
			Assert.IsNotNull (c.FindName ("foo"),"c after");
		}

		[TestMethod]
		public void DefaultProperties ()
		{
			ConcreteControl c = new ConcreteControl ();
			Assert.IsNull (c.DefaultStyleKey_, "DefaultStyleKey");
			CheckDefaultProperties (c);
		}

		static public void CheckDefaultProperties (Control c)
		{
			// default properties on Control
			Assert.IsNull (c.Background, "Background");
			Assert.IsNull (c.BorderBrush, "BorderBrush");
			Assert.AreEqual (new Thickness (0, 0, 0, 0), c.BorderThickness, "BorderThickness");
			Assert.IsNotNull (c.FontFamily, "FontFamily");
			Assert.AreEqual (FontStretches.Normal, c.FontStretch, "FontStretch");
			Assert.AreEqual (FontStyles.Normal, c.FontStyle, "FontStyle");
			Assert.AreEqual (FontWeights.Normal, c.FontWeight, "FontWeight");
// FIXME: default is null right now
//			Assert.IsNotNull (c.Foreground, "Foreground");
//			Assert.IsTrue (c.Foreground is SolidColorBrush, "Foreground/SolidColorBrush");
//			Assert.AreEqual (Colors.Black, (c.Foreground as SolidColorBrush).Color, "Foreground.Color");
			Assert.AreEqual (HorizontalAlignment.Center, c.HorizontalContentAlignment, "HorizontalContentAlignment");
			Assert.IsTrue (c.IsEnabled, "IsEnabled");
			Assert.IsTrue (c.IsTabStop, "IsTabStop");
			Assert.AreEqual (new Thickness (0, 0, 0, 0), c.Padding, "Padding");
			Assert.AreEqual (Int32.MaxValue, c.TabIndex, "AreEqual");
			Assert.AreEqual (KeyboardNavigationMode.Local, c.TabNavigation, "TabNavigation");
			Assert.IsNull (c.Template, "Template");
			Assert.AreEqual (VerticalAlignment.Center, c.VerticalContentAlignment, "VerticalContentAlignment");

			FrameworkElementTest.CheckDefaultProperties (c);
		}

		[TestMethod]
		public void DefaultMethods ()
		{
			ConcreteControl c = new ConcreteControl ();
			CheckDefaultMethods (c);
			// Focus returns false and does not trigger [Get|Lost]Focus
			Assert.IsFalse (c.GotFocusCalled, "GotFocusCalled");
			Assert.IsFalse (c.LostFocusCalled, "LostFocusCalled");
		}

		static public void CheckDefaultMethods (Control c)
		{
			Assert.IsFalse (c.ApplyTemplate (), "ApplyTemplate");
			Assert.IsFalse (c.Focus (), "Focus");
		}

		[TestMethod]
		public void Events ()
		{
			ConcreteControl c = new ConcreteControl ();
			c.IsEnabledChanged += delegate (object sender, DependencyPropertyChangedEventArgs e) {
				Assert.AreSame (c, sender, "sender");
				Assert.AreEqual (Control.IsEnabledProperty, e.Property, "IsEnabledProperty");
				Assert.IsFalse ((bool) e.NewValue, "NewValue");
				Assert.IsTrue ((bool) e.OldValue, "OldValue");
			};
			c.IsEnabled = false;
			Assert.IsFalse (c.IsEnabled, "IsEnabled");
		}

		[TestMethod]
		public void OnNull ()
		{
			ConcreteControl c = new ConcreteControl ();
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnGotFocus_ (null);
			}, "OnGotFocus");
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnLostFocus_ (null);
			}, "LostFocus");
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnKeyDown_ (null);
			}, "OnKeyDown");
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnKeyUp_ (null);
			}, "OnKeyU");
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnMouseEnter_ (null);
			}, "OnMouseEnter");
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnMouseLeave_ (null);
			}, "OnMouseLeave");
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnMouseMove_ (null);
			}, "OnMouseMove");
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnMouseLeftButtonDown_ (null);
			}, "OnMouseLeftButtonDown");
			Assert.Throws<ArgumentNullException> (delegate {
				c.OnMouseLeftButtonUp_ (null);
			}, "OnMouseLeftButtonUp");
		}
	}
}
