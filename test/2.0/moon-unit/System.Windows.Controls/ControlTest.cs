//
// Control Unit Tests
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
using System.Collections.Generic;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using Mono.Moonlight.UnitTesting;
using System.Windows.Markup;
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows.Controls {

	public class ConcreteControl : Control
	{
		public List<string> Methods = new List<string> ();
		public bool CallBaseArrangeOverride {
			get; set;
		}

		public bool CallBaseMeasureOverride {
			get; set;
		}
		
		public bool TemplateAppled {
			get; private set;
		}
		
		public ConcreteControl ()
		{
			CallBaseArrangeOverride = true;
			CallBaseMeasureOverride = true;
		}

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

		public override void OnApplyTemplate ()
		{
			Methods.Add ("Template");
			TemplateAppled = true;
			base.OnApplyTemplate ();
		}

		protected override Size ArrangeOverride (Size finalSize)
		{
			Methods.Add ("Arrange");
			if (CallBaseArrangeOverride)
				return base.ArrangeOverride (finalSize);
			return finalSize;
		}
		
		protected override Size MeasureOverride (Size availableSize)
		{
			Methods.Add ("Measure");
			if (CallBaseMeasureOverride)
				return base.MeasureOverride (availableSize);
			return availableSize;
		}
	}

	[TestClass]
	public class ControlTest : SilverlightTest {
		class MoreConcreteControl : ConcreteControl {
		}

		class SiblingControl : Control {
		}

		[TestMethod]
		public void ApplyTemplate ()
		{
			ConcreteControl poker = new ConcreteControl ();
			Assert.IsNull (poker.Template, "#1");
			Assert.IsNull (poker.Style);
			Assert.IsFalse (poker.ApplyTemplate (), "#2");
			Assert.IsNull (poker.Template, "#3");
			Assert.IsNull (poker.Style, "#4");
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
		[Asynchronous]
		public void FocusTest ()
		{
			// Show that Controls can be focused before they are loaded
			bool gotfocus = false;
			Button b = new Button ();
			b.GotFocus += delegate { gotfocus = true; };

			Assert.IsFalse (b.Focus (), "#1");
			TestPanel.Children.Add (b);
			Assert.IsTrue (b.Focus (), "#2");
			Enqueue (() => { });
			Enqueue (() => Assert.IsFalse (gotfocus, "#3"));
			EnqueueTestComplete ();
		}

		[TestMethod]
		public void FocusTest2 ()
		{
			// Show that focus is lost when the control is removed from the
			// visual tree
			Button b = new Button ();
			TestPanel.Children.Add (b);
			b.Focus ();
			Assert.AreEqual (b, FocusManager.GetFocusedElement (), "#1");
			TestPanel.Children.Clear ();
			Assert.IsNull (FocusManager.GetFocusedElement (), "#2");
		}

		[TestMethod]
		[Asynchronous]
		public void FocusTest3 ()
		{
			// Show that focus is preserved when an element is added/removed from
			// the visual tree
			bool gotfocus = false;
			bool lostfocus = false;
			Button b = new Button ();

			Enqueue (() => {
				TestPanel.Children.Add (b);
				b.Focus ();
			});
			Enqueue (() => {
				Assert.IsFalse (gotfocus, "#1");
				Assert.IsFalse (lostfocus, "#2");

				TestPanel.Children.Clear ();
				Assert.IsNull (FocusManager.GetFocusedElement (), "#3");
			});
			Enqueue (() => {
				Assert.IsFalse (gotfocus, "#4");
				Assert.IsFalse (lostfocus, "#5");

				TestPanel.Children.Add (b);
				Assert.IsNull (FocusManager.GetFocusedElement (), "#6");
			});
			Enqueue (() => {
				Assert.IsFalse (gotfocus, "#7");
				Assert.IsFalse (lostfocus, "#8");
			});
			EnqueueTestComplete ();
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
		public void InvalidValues()
		{
			ConcreteControl c = new ConcreteControl();
			c.FontSize = -1;
			c.FontSize = 0;
			c.FontSize = 1000000;

			c.Foreground = null;
			c.FontFamily = null;
		}

		[TestMethod]
		[MoonlightBug ("SetValue should not call DO.ClearValue when value is null")]
		public void NullifyFontFamily()
		{
			ConcreteControl c = new ConcreteControl();
			c.FontFamily = null;
			Assert.Throws<NullReferenceException>(delegate {
				object o = c.FontFamily;
			}, "#1");
			Assert.Throws<NullReferenceException>(delegate {
				c.SetValue (ConcreteControl.FontFamilyProperty, null);
			}, "#2");
			Assert.Throws<NullReferenceException>(delegate {
				Assert.IsNull (c.GetValue (ConcreteControl.FontFamilyProperty), "#3");
			}, "#3");
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
		[MoonlightBug]
		public void Events ()
		{
			bool changed = false;
			ConcreteControl c = new ConcreteControl ();
			c.IsEnabledChanged += delegate (object sender, DependencyPropertyChangedEventArgs e) {
				Assert.AreSame (c, sender, "sender");
				Assert.AreEqual (Control.IsEnabledProperty, e.Property, "IsEnabledProperty");
				Assert.IsFalse ((bool) e.NewValue, "NewValue");
				Assert.IsTrue ((bool) e.OldValue, "OldValue");
				changed = true;
			};
			c.IsEnabled = false;
			Assert.IsFalse (c.IsEnabled, "IsEnabled");
			Assert.IsFalse (changed, "Should be async");
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
		
		[TestMethod]
		public void MeasureAppliesTemplate ()
		{
			ConcreteControl c = new ConcreteControl { CallBaseArrangeOverride = false, CallBaseMeasureOverride = false };
			Assert.IsFalse (c.TemplateAppled, "#1");
			c.Measure (new Size (100, 100));
			Assert.IsFalse (c.TemplateAppled, "#2");
			c.ApplyTemplate ();
			Assert.IsFalse (c.TemplateAppled, "#3");
		}

		[TestMethod]
		public void ArrangeAppliesTemplate ()
		{
			ConcreteControl c = (ConcreteControl)XamlReader.Load (@"
<x:ConcreteControl	xmlns=""http://schemas.microsoft.com/client/2007""
					xmlns:x=""clr-namespace:MoonTest.System.Windows.Controls;assembly=moon-unit"">
	<x:ConcreteControl.Template>
		<ControlTemplate>
			<Grid />
		</ControlTemplate>
	</x:ConcreteControl.Template>
</x:ConcreteControl>");
			c.CallBaseArrangeOverride = false;
			c.CallBaseMeasureOverride = false;
			
			Assert.IsFalse (c.TemplateAppled, "#1");
			c.Methods.Clear ();
			c.Arrange (new Rect (0, 0, 1000, 1000));
			Assert.IsTrue (c.TemplateAppled, "#2");

			Assert.AreEqual (3, c.Methods.Count, "#3");
			Assert.AreEqual (0, c.Methods.IndexOf ("Template"), "No template");
			Assert.AreEqual (1, c.Methods.IndexOf ("Measure"), "No measure");
			Assert.AreEqual (2, c.Methods.IndexOf ("Arrange"), "No arrange");
		}

		[TestMethod]
		public void MeasureAppliesTemplate3 ()
		{
			ConcreteControl c = (ConcreteControl) XamlReader.Load (@"
<x:ConcreteControl	xmlns=""http://schemas.microsoft.com/client/2007""
					xmlns:x=""clr-namespace:MoonTest.System.Windows.Controls;assembly=moon-unit"">
	<x:ConcreteControl.Template>
		<ControlTemplate>
			<Grid />
		</ControlTemplate>
	</x:ConcreteControl.Template>
</x:ConcreteControl>");
			c.CallBaseArrangeOverride = false;
			c.CallBaseMeasureOverride = false;
			
			Assert.IsFalse (c.TemplateAppled, "#1");
			c.Measure (new Size (100, 100));
			Assert.IsTrue (c.TemplateAppled, "#3");
		}

		[TestMethod]
		public void SetName ()
		{
			FrameworkElementTest.SetName (new ConcreteControl ());
		}
	}
}
