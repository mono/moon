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
using System.Reflection;
using System.Linq;
using System.Text;

namespace MoonTest.System.Windows
{
	[TestClass]
	public partial class ResourceDictionaryTest
	{
		[TestMethod]
		public void AddDouble ()
		{
			Button b = new Button();
			ResourceDictionary rd = b.Resources;

			rd.Add ("hi", 5.0);
		}

		[TestMethod]
		public void AddObject ()
		{
			Button b = new Button();
			ResourceDictionary rd = b.Resources;

			rd.Add ("hi", new object());
		}

		[TestMethod]
		public void AddDuplicate ()
		{
			Button b = new Button();
			ResourceDictionary rd = b.Resources;

			rd.Add ("hi", new object());
			Assert.Throws<ArgumentException> (delegate { rd.Add ("hi", new object()); } );
		}

		[TestMethod]
		public void CanAddToSameRDTwiceTest ()
		{
			Assembly assembly = typeof (DependencyObject).Assembly;
			ResourceDictionary rd = new Canvas ().Resources;
			ResourceDictionary rd2 = new Canvas ().Resources;

			foreach (string s in CanAddToSameRDTwice ()) {
				object o = Activator.CreateInstance (assembly.GetType (s));
				rd.Clear ();
				rd.Add ("1", o);
				try {
					rd.Add ("2", o);
				} catch {
					Assert.Fail ("Type '{0}' should allow adding to the same RD twice", s);
				}
			}
		}

		[TestMethod]
		public void CannotAddToSameRDTwiceTest ()
		{
			Assembly assembly = typeof (DependencyObject).Assembly;
			ResourceDictionary rd = new Canvas ().Resources;

			foreach (string s in CannotAddToSameRDTwice ()) {
				object o = Activator.CreateInstance (assembly.GetType (s));
				rd.Clear ();
				rd.Add ("1", o);
				try {
					rd.Add ("2", o);
					Assert.Fail ("Type '{0}' should not allow adding to the same RD twice", s);
				} catch {
				}
			}
		}

		[TestMethod]
		public void DoesNotSupportMultipleParentsTest ()
		{
			Assembly assembly = typeof (DependencyObject).Assembly;
			ResourceDictionary rd = new Canvas ().Resources;
			ResourceDictionary rd2 = new Canvas ().Resources;

			foreach (string s in CannotAddToSameRDTwice ()) {
				object o = Activator.CreateInstance (assembly.GetType (s));
				rd.Clear ();
				rd2.Clear ();

				rd.Add ("1", o);
				try {
					rd2.Add ("2", o);
					Assert.Fail ("Type '{0}' should not permit multiple parents", s);
				} catch {
				}
			}
		}

		[TestMethod]
		public void IsReadOnly ()
		{
			ResourceDictionary rd = new ResourceDictionary ();

			Assert.IsFalse (rd.IsReadOnly);
		}

		class ConcreteDependencyObject : DependencyObject { }
		[TestMethod]
		public void ParentTest ()
		{
			ConcreteDependencyObject o = new ConcreteDependencyObject ();
			Canvas c = new Canvas ();
			c.Resources.Add ("key", o);
			Assert.Throws<InvalidOperationException> (() => c.Resources.Add ("key2", o), "Can't add to same collection twice");

			Canvas c2 = new Canvas ();
			Assert.Throws<InvalidOperationException> (() => c2.Resources.Add ("key", o), "Can't add to two different collections");
		}

		[TestMethod]
		public void Count ()
		{
			Button b = new Button();
			ResourceDictionary rd = b.Resources;

			rd.Add ("mono", new object ());
			rd.Add ("monkey", new object ());
			rd.Add ("singe", new object ());

			Assert.AreEqual (3, rd.Count);
		}

		[TestMethod]
		public void AddNull ()
		{
			ResourceDictionary rd = new ResourceDictionary ();
			Assert.Throws<ArgumentNullException>(delegate { rd.Add (null, new object ()); }, "Add(null,object)");
			Assert.Throws<NotSupportedException>(delegate { rd.Add ("s", null); }, "Add(string,null)");
			Assert.AreEqual (0, rd.Count, "Count");
		}

		[TestMethod]
		public void TestParseColor ()
		{
			Canvas b = (Canvas)
				XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");

			Assert.IsNotNull (b.Resources["color"], "1");

			Color c = (Color)b.Resources["color"];

			Assert.AreEqual (Color.FromArgb (0xff, 0xff, 0xff, 0xff), b.Resources["color"], "2");
		}

		[TestMethod]
		public void TestParseDouble ()
		{
			Assert.Throws<XamlParseException> (delegate {
					XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Double x:Key=""double"">5.0</Double></Canvas.Resources></Canvas>");
				} );
		}

		[TestMethod]
		public void Parse_MissingxKey ()
		{
			Assert.Throws<XamlParseException>(delegate { 
					XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color>#ffffffff</Color></Canvas.Resources></Canvas>");
				});
		}

		[TestMethod]
		public void Parse_MissingxKey_WithxName ()
		{
			Canvas b = (Canvas)
				XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Name=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");


			Color c = (Color)b.Resources["color"];

			Assert.AreEqual (Color.FromArgb (0xff, 0xff, 0xff, 0xff), b.Resources["color"]);
		}

		[TestMethod]
		public void Parse_xKeyOutsideResourceDictionary ()
		{
			// no exception
			XamlReader.Load (@"<Storyboard xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Key=""keysb"" x:Name=""sb"" />");
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void Parse_BothxKeyAndxName_sl3 ()
		{
			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""keycolor"" x:Name=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");
				}, "1");

			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Storyboard x:Key=""keysb"" x:Name=""sb""/></Canvas.Resources></Canvas>");
				}, "2");

			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><LineGeometry x:Key=""keygeom"" x:Name=""keygeom""/></Canvas.Resources></Canvas>");
				}, "3");

			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Storyboard  x:Key=""sb"" x:Name=""sb"" /></Canvas.Resources></Canvas>");
				}, "4");

			// no exception if the key/name are specified together but not on a direct child of a RD.
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Storyboard x:Key=""keysb""><DoubleAnimation x:Key=""keyanim"" x:Name=""anim""/></Storyboard></Canvas.Resources></Canvas>");
			
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><LineGeometry x:Name=""sb"" x:Key=""keysb"" /></Canvas.Resources></Canvas>");
		}


		[TestMethod]
		[MinRuntimeVersion(4)]
		public void Parse_BothxKeyAndxName_sl4 ()
		{
			// no longer throws an exception in sl4
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""keycolor"" x:Name=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");

			// no longer throws an exception in sl4
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Storyboard x:Key=""keysb"" x:Name=""sb""/></Canvas.Resources></Canvas>");

			// no longer throws an exception in sl4
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><LineGeometry x:Key=""keygeom"" x:Name=""keygeom""/></Canvas.Resources></Canvas>");

			// no longer throws an exception in sl4
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Storyboard  x:Key=""sb"" x:Name=""sb"" /></Canvas.Resources></Canvas>");

			// no exception if the key/name are specified together but not on a direct child of a RD.
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Storyboard x:Key=""keysb""><DoubleAnimation x:Key=""keyanim"" x:Name=""anim""/></Storyboard></Canvas.Resources></Canvas>");
			
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><LineGeometry x:Name=""sb"" x:Key=""keysb"" /></Canvas.Resources></Canvas>");
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void Parse_StyleTargetTypeOnly_sl3 ()
		{
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""> <Style TargetType=""Button""> </Style> </ResourceDictionary>");

			Assert.IsNotNull (rd["System.Windows.Controls.Button"], "1");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void Parse_StyleTargetTypeOnly_sl4 ()
		{
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""> <Style TargetType=""Button""> </Style> </ResourceDictionary>");

			// it's null in SL4
			Assert.IsNull (rd["System.Windows.Controls.Button"], "1");
		}

		[TestMethod]
		public void Parse_StyleTargetTypeAndKey1 ()
		{
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""> <Style TargetType=""Button"" x:Key=""buttonstyle""> </Style> </ResourceDictionary>");

			Assert.IsNull (rd["System.Windows.Controls.Button"], "1");
			Assert.IsNotNull (rd["buttonstyle"], "2");
		}

		[TestMethod]
		public void Parse_StyleTargetTypeAndKey2 ()
		{
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""> <Style x:Key=""buttonstyle"" TargetType=""Button""> </Style> </ResourceDictionary>");

			Assert.IsNull (rd["System.Windows.Controls.Button"], "1");
			Assert.IsNotNull (rd["buttonstyle"], "2");
		}

		[TestMethod]
		public void Parse_StyleTargetTypeAndName1 ()
		{
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""> <Style TargetType=""Button"" x:Name=""buttonstyle""> </Style> </ResourceDictionary>");

			Assert.IsNull (rd["System.Windows.Controls.Button"], "1");
			Assert.IsNotNull (rd["buttonstyle"], "2");
		}

		[TestMethod]
		public void Parse_StyleTargetTypeAndName2 ()
		{
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""> <Style x:Name=""buttonstyle"" TargetType=""Button""> </Style> </ResourceDictionary>");

			Assert.IsNull (rd["System.Windows.Controls.Button"], "1");
			Assert.IsNotNull (rd["buttonstyle"], "2");
		}

		[TestMethod]
		public void Parse_InternalStaticResourceReference1 ()
		{
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""> <Color x:Key=""colorTest"">#FFA52A2A</Color> <SolidColorBrush x:Key=""brush"" Color=""{StaticResource colorTest}"" /> </ResourceDictionary>");

			Assert.IsNotNull (rd["colorTest"], "1");
			Assert.IsNotNull (rd["brush"], "2");

			SolidColorBrush scb = (SolidColorBrush)rd["brush"];

			Assert.AreEqual (Colors.Brown, scb.Color, "3");
		}

		[TestMethod]
		public void Parse_InternalStaticResourceReference2 ()
		{
			Canvas canvas = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""colorTest"">#FFA52A2A</Color> <Style TargetType=""Button"" x:Key=""ButtonStyle""> <Setter Property=""Template""><Setter.Value> <ControlTemplate TargetType=""Button""> <Canvas><Canvas.Background><SolidColorBrush Color=""{StaticResource colorTest}"" /></Canvas.Background></Canvas> </ControlTemplate></Setter.Value></Setter> </Style> </Canvas.Resources> <Button Style=""{StaticResource ButtonStyle}"" /> </Canvas>");

			
			Button b = (Button)canvas.Children[0];

			b.ApplyTemplate ();

			Canvas c = (Canvas)VisualTreeHelper.GetChild (b, 0);

			Assert.IsTrue (c.Background is SolidColorBrush, "1");

			SolidColorBrush scb = (SolidColorBrush)c.Background;

			Assert.AreEqual (Colors.Brown, scb.Color, "2");
		}

		[TestMethod]
		public void Add_TypeAsKey_Throws ()
		{
			ResourceDictionary rd = new ResourceDictionary ();

			Assert.Throws<ArgumentException> (() => rd.Add (typeof (TextBlock), "foobar"));
		}

		[TestMethod]
		public void Add_TypeAsKeyToStyle_Throws ()
		{
			ResourceDictionary rd = new ResourceDictionary ();

			Assert.Throws<ArgumentException> (() => rd.Add (typeof (TextBlock), new Style ()));
		}

		[TestMethod]
		public void Contains_TypeAsKey_DoesNotThrow ()
		{
			ResourceDictionary rd = new ResourceDictionary ();

			rd.Contains (typeof (TextBlock));
		}

		[TestMethod]
		public void Contains_IntAsKey_Throws ()
		{
			ResourceDictionary rd = new ResourceDictionary ();

			Assert.Throws<ArgumentException> (() => rd.Contains (35));
		}

		[TestMethod]
		public void Contains_ObejctAsKey_Throws ()
		{
			ResourceDictionary rd = new ResourceDictionary ();

			Assert.Throws<ArgumentException> (() => rd.Contains (new object ()));
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void Contains_TypeNameAsKeyItemRegisteredInXaml_ReturnsTrue_sl3 ()
		{
			ResourceDictionary rd = (ResourceDictionary) XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
											    <Style TargetType=""Button""> </Style>
											</ResourceDictionary>");

			bool contains = rd.Contains ("System.Windows.Controls.Button");
			Assert.IsTrue (contains);
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void Contains_TypeNameAsKeyItemRegisteredInXaml_ReturnsFalse_sl4 ()
		{
			ResourceDictionary rd = (ResourceDictionary) XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
											    <Style TargetType=""Button""> </Style>
											</ResourceDictionary>");

			bool contains = rd.Contains ("System.Windows.Controls.Button");
			Assert.IsFalse (contains);
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void Contains_TypeAsKeyItemRegisteredInXaml_ReturnsFalse_sl3 ()
		{
			ResourceDictionary rd = (ResourceDictionary) XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
											    <Style TargetType=""Button""> </Style>
											</ResourceDictionary>");

			bool contains = rd.Contains (typeof (Button));
			Assert.IsFalse (contains);
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void Contains_TypeAsKeyItemRegisteredInXaml_ReturnsTrue_sl4 ()
		{
			ResourceDictionary rd = (ResourceDictionary) XamlReader.Load (@"<ResourceDictionary xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
											    <Style TargetType=""Button""> </Style>
											</ResourceDictionary>");

			bool contains = rd.Contains (typeof (Button));
			Assert.IsTrue (contains);
		}

		[TestMethod]
		public void Contains_TypeAsKeyStyleWithTargetTypeSet_ReturnsTrue ()
		{
			ResourceDictionary rd = new ResourceDictionary ();

			Style s = new Style () {
				TargetType = typeof (Button),
			};

			rd.Add (typeof (Button), s);

			bool contains = rd.Contains (typeof (Button));
			Assert.IsTrue (contains);
		}

		[TestMethod]
		public void Contains_TypeNameAsKeyStyleWithTargetTypeSet_ReturnsFalse ()
		{
			ResourceDictionary rd = new ResourceDictionary ();

			Style s = new Style () {
				TargetType = typeof (Button),
			};

			rd.Add (typeof (Button), s);

			bool contains = rd.Contains ("System.Windows.Controls.Button");
			Assert.IsFalse (contains);
		}

		[TestMethod]
		public void TestIntegerIndex ()
		{
			Canvas b = (Canvas)
				XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");

			Assert.Throws<ArgumentException>(delegate { Color c = (Color)b.Resources[0]; });
		}

		[TestMethod]
		public void RemoveTest ()
		{
			Canvas b;

			b = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");

			IDictionary<object, object> oo = (IDictionary<object, object>)b.Resources;
			Assert.IsTrue (oo.Remove ("color"), "1");
			Assert.IsFalse (oo.Remove ("color"), "2");

			b = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");

			ICollection<KeyValuePair<object, object>> koo = (ICollection<KeyValuePair<object, object>>)b.Resources;
			Assert.IsFalse (oo.Remove ("color"), "3");
			Assert.IsFalse (oo.Remove ("color"), "4");

			b = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");

			b.Resources.Remove ("color");
			b.Resources.Remove ("color");


			Assert.Throws<ArgumentNullException>( delegate { b.Resources.Remove (null); } );
		}

		[TestMethod]
		public void ContainsTest ()
		{
			var canvas = CreateCanvas ();

			Assert.IsTrue (canvas.Resources.Contains ("color"));
		}

		static Canvas CreateCanvas ()
		{
			return (Canvas) XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");
		}

		[TestMethod]
		public void ContainsObjectKey ()
		{
			var canvas = CreateCanvas ();

			Assert.Throws<ArgumentException> (() => canvas.Resources.Contains (new object ()));
		}

		[TestMethod]
		public void ContainsNullKey ()
		{
			var canvas = CreateCanvas ();

			Assert.Throws<ArgumentNullException> (() => canvas.Resources.Contains (null));
		}

		[TestMethod]
		public void InterfaceTest ()
		{
			IDictionary<object, object> d = new ResourceDictionary ();
			Assert.Throws<ArgumentException> (delegate {
				d.Add (new KeyValuePair<object, object> (new object (), new object ()));
			}, "#1");
			d.Add (new KeyValuePair<object, object> ("bob", new object ()));
			Assert.Throws<NotImplementedException> (delegate {
				int a = d.Count;
			}, "#2");
			Assert.AreEqual (1, ((ResourceDictionary) d).Count, "#3");
			Assert.Throws<ArgumentException> (delegate {
				d.Add (new object (), new object ());
			}, "#4");
			d.Add ("str", new object ());
			Assert.AreEqual (2, ((ResourceDictionary) d).Count, "#5");
			Assert.Throws<ArgumentException> (delegate {
				d.ContainsKey (new object ());
			}, "#6");
			Assert.IsTrue (d.Contains (new KeyValuePair<object, object> ("str", new object ())), "#7");
			d.Clear ();
			Assert.AreEqual (0, ((ResourceDictionary) d).Count, "#8");

			// this doesn't throw in SL4 for any runtime version
			d.GetEnumerator ();

			Assert.Throws<NotImplementedException> (delegate {
				var v = d.Keys;
			}, "#10");
			Assert.Throws<NotImplementedException> (delegate {
				var v = d.Values;
			}, "#11");
			Assert.IsFalse (d.IsReadOnly, "#12");
			Assert.Throws<NotImplementedException> (delegate {
				d.CopyTo (new KeyValuePair<object, object> [10], 0);
			}, "#13");
		}
		
		[TestMethod]
		public void TestFindName ()
		{
			Canvas b = (Canvas)
				XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color><Storyboard x:Key=""sb""/><Storyboard x:Name=""anothersb""/></Canvas.Resources></Canvas>");

			Assert.IsNull (b.FindName ("color"));
			Assert.IsNull (b.FindName ("sb"));
			Assert.IsNotNull (b.FindName ("anothersb"));
			Assert.IsNotNull (b.Resources ["anothersb"]);

			Assert.AreSame (b.FindName ("anothersb"), b.Resources["anothersb"]);
		}

		[TestMethod]
		public void TestNameAndKey ()
		{
			Assert.Throws<XamlParseException>(delegate {
					XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Storyboard x:Name=""sb""/><Storyboard x:Key=""sb""/></Canvas.Resources></Canvas>");
				});
		}

		[TestMethod]
		public void TestCount ()
		{
			Canvas b = (Canvas)
				XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");

			Assert.AreEqual (1, b.Resources.Count);

			b = (Canvas)
				XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Name=""color"">#ffffffff</Color></Canvas.Resources></Canvas>");

			Assert.AreEqual (1, b.Resources.Count);
		}

		[TestMethod]
		public void TestLoadResources ()
		{
			ListBox box = (ListBox) XamlReader.Load (@"
<ListBox xmlns=""http://schemas.microsoft.com/client/2007""
		 xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
	<ListBox.Resources>
		<Storyboard x:Name=""Show"" />
	</ListBox.Resources>
</ListBox>");
			Assert.AreEqual (1, box.Resources.Count, "#1");
			Assert.IsInstanceOfType<Storyboard> (box.Resources ["Show"], "#2");
		}
		
		[TestMethod]
		public void TestxKeyOutsideDictionary ()
		{
			Canvas b = (Canvas)
				XamlReader.Load (@"<Canvas x:Key=""foo"" xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />");
		}

		[TestMethod]
		public void TestStaticResourceSameElement ()
		{
			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" Background=""{StaticResource BackgroundBrush}""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color><SolidColorBrush x:Key=""BackgroundBrush"" Color=""Black""/></Canvas.Resources></Canvas>");
				} );
		}

		[TestMethod]
		public void TestStaticResourceParentElement_Element ()
		{
			Canvas top = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child"" Fill=""{StaticResource FillBrush}""/></Canvas>");

			Rectangle child = (Rectangle)top.FindName ("child");
			Assert.AreEqual (Color.FromArgb (0xff, 0x00, 0x00, 0x00), ((SolidColorBrush)child.Fill).Color);
		}

		[TestMethod]
		public void TestStaticResourceParentElement_Property ()
		{
			Canvas top = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Color x:Key=""color"">#ffffffff</Color></Canvas.Resources><Rectangle x:Name=""child""><Rectangle.Stroke><SolidColorBrush Color=""{StaticResource color}""/></Rectangle.Stroke></Rectangle></Canvas>");

			Rectangle child = (Rectangle)top.FindName ("child");
			Assert.AreEqual (Color.FromArgb (0xff, 0xff, 0xff, 0xff), ((SolidColorBrush)child.Stroke).Color);
		}

		[TestMethod]
		public void TestStaticResourceMissingResource ()
		{
			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child"" Fill=""{StaticResource FillBrush}""><Rectangle.Stroke><SolidColorBrush Color=""{StaticResource color}""/></Rectangle.Stroke></Rectangle></Canvas>");
				});
		}

		// This was breaking airlines
		[TestMethod]
		public void TestUnnamespacedName ()
		{
			Canvas b = (Canvas)
				XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><Storyboard Name=""sb""/></Canvas.Resources></Canvas>");

			Assert.IsNotNull (b.Resources["sb"]);
		}

		[TestMethod]
		public void TestStaticResourceSyntax ()
		{
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child"" Fill=""{ StaticResource    FillBrush }""/></Canvas>");

			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child"" Fill="" {StaticResource FillBrush}""/></Canvas>"); }, "3");
			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child"" Fill=""{StaticResource FillBrush} ""/></Canvas>"); }, "4");
		}

		[TestMethod]
		[Ignore]
		public void CorruptsMemoryInSilverlight4 ()
		{
			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child"" Fill=""{StaticResource}""/></Canvas>"); }, "1");
			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child"" Fill=""{StaticResource }""/></Canvas>"); }, "2");
		}

		[TestMethod]
		public void TestStaticResourceElementSyntax ()
		{
			XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child""><Rectangle.Fill><StaticResource ResourceKey=""FillBrush"" /></Rectangle.Fill></Rectangle></Canvas>");

		}

		[TestMethod]
		public void TestStaticResourceMissingElementSyntax ()
		{
			Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child""><Rectangle.Fill><StaticResource ResourceKey=""nope"" /></Rectangle.Fill></Rectangle></Canvas>"); }, "1");

			
			///  Don't do this it crashes Silverlight
			/// Assert.Throws<XamlParseException>(delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child""><Rectangle.Fill><StaticResource /></Rectangle.Fill></Rectangle></Canvas>"); }, "1");
			///
		}
		
		[TestMethod]
		public void TestResourceFrozenState ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""><Canvas.Resources><SolidColorBrush x:Key=""FillBrush"" Color=""Black""/></Canvas.Resources><Rectangle x:Name=""child"" Fill=""{StaticResource FillBrush}""/></Canvas>");
			Rectangle r = (Rectangle)c.FindName ("child");

			SolidColorBrush scb = (SolidColorBrush)c.Resources["FillBrush"];

			Assert.AreEqual (scb, r.Fill, "1");

			scb.Color = Colors.Blue;

			Assert.AreEqual (Colors.Blue, ((SolidColorBrush)r.Fill).Color, "2");
		}

		[TestMethod]
		public void Clear ()
		{
			ResourceDictionary rd = new ResourceDictionary ();
			rd.Add ("key", new object ());
			Assert.AreEqual (1, rd.Count, "Count-1");
			rd.Clear ();
			Assert.AreEqual (0, rd.Count, "Count-2");
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void TypeConversionOfStaticResources_sl3 ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:sys=""clr-namespace:System;assembly=mscorlib"">
<Canvas.Resources><sys:Int32 x:Key=""an_int"">300</sys:Int32><sys:String x:Key=""a_str"">Arial</sys:String></Canvas.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource an_int}""/><TextBlock x:Name=""block"" FontFamily=""{StaticResource a_str}""/></Canvas>");
			Rectangle r = (Rectangle)c.FindName ("rect");
			TextBlock tb = (TextBlock) c.FindName ("block");

			Assert.AreEqual (r.Width, 300, "1");
			Assert.AreEqual (tb.FontFamily, new FontFamily ("Arial"), "2");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void TypeConversionOfStaticResources_sl4 ()
		{
			Assert.Throws<XamlParseException> ( delegate { XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:sys=""clr-namespace:System;assembly=mscorlib"">
<Canvas.Resources><sys:Int32 x:Key=""an_int"">300</sys:Int32></Canvas.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource an_int}""/></Canvas>"); });

			Canvas c = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:sys=""clr-namespace:System;assembly=mscorlib"">
<Canvas.Resources><sys:Double x:Key=""a_double"">300.0</sys:Double><sys:String x:Key=""a_str"">Arial</sys:String></Canvas.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource a_double}""/><TextBlock x:Name=""block"" FontFamily=""{StaticResource a_str}""/></Canvas>");
			Rectangle r = (Rectangle)c.FindName ("rect");
			TextBlock tb = (TextBlock) c.FindName ("block");

			Assert.AreEqual (r.Width, 300, "1");
			Assert.AreEqual (tb.FontFamily, new FontFamily ("Arial"), "2");
		}

		[TestMethod]
		[MaxRuntimeVersion(3)]
		public void SpacesInKeys_sl3 ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:sys=""clr-namespace:System;assembly=mscorlib"">
<Canvas.Resources><sys:Int32 x:Key=""an int"">300</sys:Int32></Canvas.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource an int}""/></Canvas>");
			Rectangle r = (Rectangle)c.FindName ("rect");

			Assert.AreEqual (r.Width, 300, "1");
		}

		[TestMethod]
		[MinRuntimeVersion(4)]
		public void SpacesInKeys_sl4 ()
		{
			Assert.Throws<XamlParseException> ( delegate {XamlReader.Load (@"<Canvas xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:sys=""clr-namespace:System;assembly=mscorlib"">
<Canvas.Resources><sys:Int32 x:Key=""an int"">300</sys:Int32></Canvas.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource an int}""/></Canvas>"); });
		}

		[TestMethod]
		public void SourceProperty_Null ()
		{
			// This just verifies we can set a null to the source property.
			var rd = new ResourceDictionary ();
			rd.Source = null;
		}

		[TestMethod]
		public void SourceProperty_InvalidUrl ()
		{
			// Invalid uris cause exceptions
			var rd = new ResourceDictionary ();
			Assert.Throws<Exception> (() => {
				rd.Source = new Uri ("/whatever/yeah/thing", UriKind.Relative);
			});
		}

		[TestMethod]
		public void SourceProperty_ValidUrl ()
		{
			// Invalid uris cause exceptions
			var rd = new ResourceDictionary ();
			rd.Source = new Uri ("/moon-unit;component/System.Windows/resourcedictionarysourcepropertytest.xaml", UriKind.Relative);
			Assert.AreEqual (1, rd.Count, "#1");
		}

		[TestMethod]
		public void SourceProperty_InvalidUrl2 ()
		{
			// Invalid uris cause exceptions
			var rd = new ResourceDictionary ();
			Assert.Throws<Exception> (() => {
				rd.Source = new Uri ("/moon-unit;component/System.Windows/fake.xaml", UriKind.Relative);
			});
		}

		[TestMethod]
		public void SourcePropertyInResources ()
		{
			Grid c = (Grid)XamlReader.Load (@"<Grid xmlns=""http://schemas.microsoft.com/client/2007""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Grid.Resources>
  <ResourceDictionary Source=""/moon-unit;component/System.Windows/resourcedictionarysourcepropertytest.xaml"" />
</Grid.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource the_width}"" />
</Grid>");

			Rectangle r = (Rectangle)c.FindName ("rect");

			Assert.AreEqual (r.Width, 300, "1");
		}

		[TestMethod]
		public void SourcePropertyInXap ()
		{
			Grid c = (Grid)XamlReader.Load (@"<Grid xmlns=""http://schemas.microsoft.com/client/2007""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Grid.Resources>
  <ResourceDictionary Source=""/System.Windows/ResourceDictionarySourcePropertyTest.xaml"" />
</Grid.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource the_width}"" />
</Grid>");

			Rectangle r = (Rectangle)c.FindName ("rect");

			Assert.AreEqual (r.Width, 300, "1");
		}

		[TestMethod]
		public void MergedDictionariesTest ()
		{
			Grid c = (Grid)XamlReader.Load (@"<Grid xmlns=""http://schemas.microsoft.com/client/2007""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Grid.Resources>
  <ResourceDictionary>
    <ResourceDictionary.MergedDictionaries>
       <ResourceDictionary Source=""/System.Windows/ResourceDictionarySourcePropertyTest.xaml"" />
    </ResourceDictionary.MergedDictionaries>
  </ResourceDictionary>
</Grid.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource the_width}"" />
</Grid>");

			Rectangle r = (Rectangle)c.FindName ("rect");

			Assert.AreEqual (r.Width, 300, "1");
		}

		[TestMethod]
		public void MergedDictionariesTest_SourceLackingDefaultXmlns ()
		{
			Grid c = (Grid)XamlReader.Load (@"<Grid xmlns=""http://schemas.microsoft.com/client/2007""
xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
<Grid.Resources>
  <ResourceDictionary>
    <ResourceDictionary.MergedDictionaries>
       <ResourceDictionary Source=""/System.Windows/ResourceDictionarySourcePropertyTest_LackingDefaultXmlns.xaml"" />
    </ResourceDictionary.MergedDictionaries>
  </ResourceDictionary>
</Grid.Resources>
<Rectangle x:Name=""rect"" Width=""{StaticResource the_width}"" />
</Grid>");

			Rectangle r = (Rectangle)c.FindName ("rect");

			Assert.AreEqual (r.Width, 300, "1");
		}

		[TestMethod]
		public void MergedDictionaries_KeyClashes ()
		{
			var key = "Key";
			var value = new object ();
			var value1 = new object ();
			var value2 = new object ();

			var merge1 = new ResourceDictionary ();
			var merge2 = new ResourceDictionary ();
			ResourceDictionary main = new ResourceDictionary ();
			main.MergedDictionaries.Add (merge1);
			main.MergedDictionaries.Add (merge2);

			main.Add (key, value);
			merge1.Add (key, value1);
			merge2.Add (key, value2);
			
			// We ignore the values in the merged dictionaries if the main dictionary
			// has the key
			Assert.AreEqual (value, main [key], "#1");

			main.Remove (key);

			// We should look up the merged dictionaries in reverse order
			Assert.AreEqual (value2,main [key],  "#2");

			merge2.Remove (key);

			// Now we should find it in the first merged dictionary
			Assert.AreEqual (value1, main [key], "#3");
		}

		[TestMethod]
		public void MergedDictionary_Contains ()
		{
			// 'Contains' looks up the merged dictionaries too
			var d1 = new ResourceDictionary ();
			var d2 = new ResourceDictionary ();
			d2.Add ("string", "string2");
			d1.MergedDictionaries.Add (d2);
			Assert.IsTrue(d1.Contains ("string"), "#1");
		}

		[TestMethod]
		public void SupportsMultipleParentsTest ()
		{
			Assembly assembly = typeof (DependencyObject).Assembly;
			ResourceDictionary rd = new Canvas ().Resources;
			ResourceDictionary rd2 = new Canvas ().Resources;

			foreach (string s in SupportsMultipleParents ()) {
				object o = Activator.CreateInstance (assembly.GetType (s));
				rd.Clear ();
				rd2.Clear ();

				rd.Add ("1", o);
				try {
					rd2.Add ("2", o);
				} catch {
					Assert.Fail ("Type '{0}' should permit multiple parents", s);
				}
			}
		}

		IEnumerable<string> CanAddToSameRDTwice ()
		{
			return new string [] { 
				"System.Windows.DataTemplate", 
				"System.Windows.Style", 
				"System.Windows.Ink.StrokeCollection", 
				"System.Windows.Ink.DrawingAttributes", 
				"System.Windows.Media.RotateTransform", 
				"System.Windows.Media.ScaleTransform", 
				"System.Windows.Media.SkewTransform", 
				"System.Windows.Media.TranslateTransform", 
				"System.Windows.Media.TransformGroup", 
				"System.Windows.Media.MatrixTransform", 
				"System.Windows.Media.SolidColorBrush", 
				"System.Windows.Media.LinearGradientBrush", 
				"System.Windows.Media.RadialGradientBrush", 
				"System.Windows.Media.ImageBrush", 
				"System.Windows.Media.VideoBrush", 
				"System.Windows.Input.StylusPointCollection", 
				"System.Windows.Controls.ControlTemplate", 
				"System.Windows.Controls.ItemsPanelTemplate", 
				"System.Windows.Media.Imaging.BitmapImage", 
				"System.Windows.Ink.Stroke"
			};
		}

		IEnumerable<string> CannotAddToSameRDTwice ()
		{
			return new string [] { 
				"System.Windows.EventTrigger", 
				"System.Windows.TriggerActionCollection", 
				"System.Windows.AssemblyPart", 
				"System.Windows.AssemblyPartCollection", 
				"System.Windows.Setter", 
				"System.Windows.SetterBaseCollection", 
				"System.Windows.VisualState", 
				"System.Windows.Media.DoubleCollection", 
				"System.Windows.Media.PointCollection", 
				"System.Windows.Media.TransformCollection", 
				"System.Windows.Media.LineSegment", 
				"System.Windows.Media.BezierSegment", 
				"System.Windows.Media.QuadraticBezierSegment", 
				"System.Windows.Media.ArcSegment", 
				"System.Windows.Media.PolyLineSegment", 
				"System.Windows.Media.PolyBezierSegment", 
				"System.Windows.Media.PolyQuadraticBezierSegment", 
				"System.Windows.Media.PathSegmentCollection", 
				"System.Windows.Media.PathFigure", 
				"System.Windows.Media.PathFigureCollection", 
				"System.Windows.Media.PathGeometry", 
				"System.Windows.Media.EllipseGeometry", 
				"System.Windows.Media.RectangleGeometry", 
				"System.Windows.Media.LineGeometry", 
				"System.Windows.Media.GeometryCollection", 
				"System.Windows.Media.GeometryGroup", 
				"System.Windows.Media.GradientStop", 
				"System.Windows.Media.GradientStopCollection", 
				"System.Windows.Media.TimelineMarker", 
				"System.Windows.Media.TimelineMarkerCollection", 
				"System.Windows.Media.Animation.BeginStoryboard", 
				"System.Windows.Media.Animation.Storyboard", 
				"System.Windows.Media.Animation.DoubleAnimation", 
				"System.Windows.Media.Animation.ColorAnimation", 
				"System.Windows.Media.Animation.PointAnimation", 
				"System.Windows.Media.Animation.KeySpline", 
				"System.Windows.Media.Animation.ColorKeyFrameCollection", 
				"System.Windows.Media.Animation.DoubleKeyFrameCollection", 
				"System.Windows.Media.Animation.PointKeyFrameCollection", 
				"System.Windows.Media.Animation.ObjectKeyFrameCollection", 
				"System.Windows.Media.Animation.DoubleAnimationUsingKeyFrames", 
				"System.Windows.Media.Animation.DiscreteDoubleKeyFrame", 
				"System.Windows.Media.Animation.LinearDoubleKeyFrame", 
				"System.Windows.Media.Animation.SplineDoubleKeyFrame", 
				"System.Windows.Media.Animation.ColorAnimationUsingKeyFrames", 
				"System.Windows.Media.Animation.DiscreteColorKeyFrame", 
				"System.Windows.Media.Animation.LinearColorKeyFrame", 
				"System.Windows.Media.Animation.SplineColorKeyFrame", 
				"System.Windows.Media.Animation.PointAnimationUsingKeyFrames", 
				"System.Windows.Media.Animation.DiscretePointKeyFrame", 
				"System.Windows.Media.Animation.LinearPointKeyFrame", 
				"System.Windows.Media.Animation.SplinePointKeyFrame", 
				"System.Windows.Media.Animation.ObjectAnimationUsingKeyFrames", 
				"System.Windows.Media.Animation.DiscreteObjectKeyFrame", 
				"System.Windows.Media.Animation.TimelineCollection", 
				"System.Windows.Documents.Run", 
				"System.Windows.Documents.LineBreak", 
				"System.Windows.Controls.MultiScaleSubImage", 
				"System.Windows.Controls.RowDefinition", 
				"System.Windows.Controls.ColumnDefinition", 
				"System.Windows.VisualStateGroup", 
				"System.Windows.VisualStateManager", 
				"System.Windows.Media.DeepZoomImageTileSource", 
				"System.Windows.ResourceDictionary"
			};
		}

		IEnumerable<string> SupportsMultipleParents ()
		{
			return new string [] {
				"System.Windows.DataTemplate", 
				"System.Windows.Style", 
				"System.Windows.Ink.StrokeCollection", 
				"System.Windows.Ink.DrawingAttributes", 
				"System.Windows.Media.RotateTransform", 
				"System.Windows.Media.ScaleTransform", 
				"System.Windows.Media.SkewTransform", 
				"System.Windows.Media.TranslateTransform", 
				"System.Windows.Media.TransformGroup", 
				"System.Windows.Media.MatrixTransform", 
				"System.Windows.Media.SolidColorBrush", 
				"System.Windows.Media.LinearGradientBrush", 
				"System.Windows.Media.RadialGradientBrush", 
				"System.Windows.Media.ImageBrush", 
				"System.Windows.Media.VideoBrush", 
				"System.Windows.Input.StylusPointCollection", 
				"System.Windows.Controls.ControlTemplate", 
				"System.Windows.Controls.ItemsPanelTemplate", 
				"System.Windows.Media.Imaging.BitmapImage", 
				"System.Windows.Ink.Stroke"
			};
		}

		IEnumerable<string> DoesNotSupportMultipleParents ()
		{
			return new string [] {
				"System.Windows.EventTrigger", 
				"System.Windows.TriggerActionCollection", 
				"System.Windows.AssemblyPart", 
				"System.Windows.AssemblyPartCollection", 
				"System.Windows.Setter", 
				"System.Windows.SetterBaseCollection", 
				"System.Windows.VisualState", 
				"System.Windows.Media.DoubleCollection", 
				"System.Windows.Media.PointCollection", 
				"System.Windows.Media.TransformCollection", 
				"System.Windows.Media.LineSegment", 
				"System.Windows.Media.BezierSegment", 
				"System.Windows.Media.QuadraticBezierSegment", 
				"System.Windows.Media.ArcSegment", 
				"System.Windows.Media.PolyLineSegment", 
				"System.Windows.Media.PolyBezierSegment", 
				"System.Windows.Media.PolyQuadraticBezierSegment", 
				"System.Windows.Media.PathSegmentCollection", 
				"System.Windows.Media.PathFigure", 
				"System.Windows.Media.PathFigureCollection", 
				"System.Windows.Media.PathGeometry", 
				"System.Windows.Media.EllipseGeometry", 
				"System.Windows.Media.RectangleGeometry", 
				"System.Windows.Media.LineGeometry", 
				"System.Windows.Media.GeometryCollection", 
				"System.Windows.Media.GeometryGroup", 
				"System.Windows.Media.GradientStop", 
				"System.Windows.Media.GradientStopCollection", 
				"System.Windows.Media.TimelineMarker", 
				"System.Windows.Media.TimelineMarkerCollection", 
				"System.Windows.Media.Animation.BeginStoryboard", 
				"System.Windows.Media.Animation.Storyboard", 
				"System.Windows.Media.Animation.DoubleAnimation", 
				"System.Windows.Media.Animation.ColorAnimation", 
				"System.Windows.Media.Animation.PointAnimation", 
				"System.Windows.Media.Animation.KeySpline", 
				"System.Windows.Media.Animation.ColorKeyFrameCollection", 
				"System.Windows.Media.Animation.DoubleKeyFrameCollection", 
				"System.Windows.Media.Animation.PointKeyFrameCollection", 
				"System.Windows.Media.Animation.ObjectKeyFrameCollection", 
				"System.Windows.Media.Animation.DoubleAnimationUsingKeyFrames", 
				"System.Windows.Media.Animation.DiscreteDoubleKeyFrame", 
				"System.Windows.Media.Animation.LinearDoubleKeyFrame", 
				"System.Windows.Media.Animation.SplineDoubleKeyFrame", 
				"System.Windows.Media.Animation.ColorAnimationUsingKeyFrames", 
				"System.Windows.Media.Animation.DiscreteColorKeyFrame", 
				"System.Windows.Media.Animation.LinearColorKeyFrame", 
				"System.Windows.Media.Animation.SplineColorKeyFrame", 
				"System.Windows.Media.Animation.PointAnimationUsingKeyFrames", 
				"System.Windows.Media.Animation.DiscretePointKeyFrame", 
				"System.Windows.Media.Animation.LinearPointKeyFrame", 
				"System.Windows.Media.Animation.SplinePointKeyFrame", 
				"System.Windows.Media.Animation.ObjectAnimationUsingKeyFrames", 
				"System.Windows.Media.Animation.DiscreteObjectKeyFrame", 
				"System.Windows.Media.Animation.TimelineCollection", 
				"System.Windows.Documents.Run", 
				"System.Windows.Documents.LineBreak", 
				"System.Windows.Controls.MultiScaleSubImage", 
				"System.Windows.Controls.RowDefinition", 
				"System.Windows.Controls.ColumnDefinition", 
				"System.Windows.VisualStateGroup", 
				"System.Windows.VisualStateManager", 
				"System.Windows.Media.DeepZoomImageTileSource", 
				"System.Windows.ResourceDictionary"
			};
		}
	}
}
