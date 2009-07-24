using System;
using System.Net;
using System.Linq;
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
using Microsoft.Silverlight.Testing;

namespace MoonTest.System.Windows
{
	[TestClass]
	public partial class VisualStateTest : SilverlightTest
	{
		[TestMethod]
		[MoonlightBug]
		public void GroupNameTest ()
		{
			UserControl c = (UserControl) XamlReader.Load (@"
<UserControl
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    xmlns:clr=""clr-namespace:SilverlightApplication14""
    x:Name=""RootControl"">
    <VisualStateManager.VisualStateGroups>
        <VisualStateGroup x:Name=""Tester"" />
    </VisualStateManager.VisualStateGroups>
    <Canvas>
    
    </Canvas>
</UserControl>");
			VisualStateGroup g = VisualStateManager.GetVisualStateGroups (c).Cast<VisualStateGroup> ().First ();
			Assert.AreEqual ("Tester", g.Name, "#1");
		}
		
		[TestMethod]
		[MoonlightBug]
		public void AddRemoveHandlerTest ()
		{
			// This test spews errors to the terminal but doesn't throw exceptions
			// Only remove the moonlight bug when this is fixed.
			// Event lookup of event 'CurrentStateChanging' in type 'object' failed.
			// Event lookup of event 'CurrentStateChanging' in (more exactly) type 'EventObject' failed.
			// Event lookup of event 'CurrentStateChanging' in (more exactly) type 'DependencyObject' failed.
			// Event lookup of event 'CurrentStateChanging' in (more exactly) type 'System.Windows.VisualStateGroup' failed.
			VisualStateGroup g = new VisualStateGroup ();
			g.CurrentStateChanged += delegate { };
			g.CurrentStateChanged -= delegate { };
			
			g.CurrentStateChanging += delegate { };
			g.CurrentStateChanging -= delegate { };
		}

		[TestMethod]
		public void TestParse ()
		{
			VisualState vs = (VisualState)XamlReader.Load (@"<vsm:VisualState xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:vsm=""clr-namespace:System.Windows;assembly=System.Windows"" x:Name=""foo""><Storyboard /></vsm:VisualState>");
			Assert.AreEqual ("foo", vs.Name);
			Assert.IsNotNull (vs.Storyboard);
		}

		[TestMethod]
		[MoonlightBug]
		public void TestParse_NoManagedNamespace ()
		{
			Assert.Throws (delegate { 
					XamlReader.Load (@"<VisualState xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" x:Name=""foo""><Storyboard /></VisualState>");
				},
				// "VisualState does not support Storyboard as content."
				typeof (XamlParseException));
		}

		[TestMethod]
		public void TestParse_NoStoryboard ()
		{
			VisualState vs = (VisualState)XamlReader.Load (@"<vsm:VisualState xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:vsm=""clr-namespace:System.Windows;assembly=System.Windows"" x:Name=""foo"" />");
			Assert.IsNull (vs.Storyboard);
		}

		[TestMethod]
		public void TestParse_NoName ()
		{
			VisualState vs = (VisualState)XamlReader.Load (@"<vsm:VisualState xmlns=""http://schemas.microsoft.com/client/2007"" xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" xmlns:vsm=""clr-namespace:System.Windows;assembly=System.Windows""><Storyboard /></vsm:VisualState>");
			Assert.AreEqual ("", vs.Name);
		}

		[TestMethod]
		public void TestDefaults ()
		{
			VisualState st = new VisualState ();
			Assert.AreEqual ("", st.Name, "1");
			Assert.IsNull (st.Storyboard, "2");
		}

		[TestMethod]
		public void UseStoryboardThrice ()
		{
			Storyboard sb = new Storyboard ();
			VisualState s1 = new VisualState { Storyboard = sb };
			VisualState s2 = new VisualState { Storyboard = sb };

			TestPanel.Resources.Add ("a", s1);
			TestPanel.Resources.Add ("b", s2);
			TestPanel.Resources.Add ("c", sb);
		}
	}
}
