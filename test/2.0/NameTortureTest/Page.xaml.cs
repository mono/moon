using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Net;
using System.Reflection;
using System.Text;
using System.Windows;
using System.Windows.Browser;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace NameTortureTest
{
	[ScriptableType]
	public partial class Page : UserControl
	{
		public Page()
		{
			InitializeComponent ();

			Assert.SetLog (log);

 			HtmlPage.RegisterScriptableObject ("Harness", this);
		}

		public void RunTests ()
		{
			RunTests ("AddItemToCanvasInTemplate");
			RunTests ("AddTemplateItemToNewSubtree");
			RunTests ("BasicXamlReaderTests");
			RunTests ("ChangeNameOfTemplatedItem");
			RunTests ("CustomControlInTemplate");
			RunTests ("AddXamlReaderOutputToExistingTreeVisualType");
			RunTests ("AddXamlReaderOutputToExistingTreeNonVisualType");
			RunTests ("UserControlNamescope1");
			RunTests ("UserControlNamescope4");
			RunTests ("CanvasSubclassNamescope");
 			RunTests ("NonVisualTypes");
			RunTests ("VisualTypes");
			RunTests ("StaticResources");
			RunTests ("StaticResourceNameLookup");
			RunTests ("StaticResourceNamesConflictingOnSibling");
			RunTests ("StaticResourceNamesConflictingBetweenResourceAndElement");
			RunTests ("StoryboardTemplateItem");
			RunTests ("StaticResourceUserControls");
			RunTests ("TemplatedControl");
			RunTests ("UserControlEmbeddedInXaml");

 			HtmlPage.RegisterScriptableObject ("Assert", new JSAsserter());

 			RunJSTests ("CreateFromXamlWithNamescope");
			RunJSTests ("ContentFindNameFallback");
			RunJSTests ("FindNameEnclosingNamescopes");
		}

		private void RunTests (string testName)
		{
			Assert.Reset ();

			log.Text += string.Format ("{0}\n", testName);

			try {
				MethodInfo mi = typeof (Page).GetMethod (testName);
				mi.Invoke (this, new object[] { });
			}
			catch (Exception e) {
				log.Text += e.ToString() + "\n";
				Assert.Failures ++;
				Assert.TotalFailures ++;
			}

			log.Text += string.Format ("    Assertions: {0}, failures: {1}\n\n", Assert.Count, Assert.Failures);
			if (Assert.Failures > 0)
				log.Background = new SolidColorBrush(Colors.Red);

			testArea.Children.Clear ();
			status.Text = string.Format ("Assertions: {0}, Failures: {1}", Assert.TotalCount, Assert.TotalFailures);
		}

		private void RunJSTests (string testName)
		{
			Assert.Reset ();

			log.Text += string.Format ("(JS) {0}\n", testName);

			try {
				ScriptObject obj = HtmlPage.Window.CreateInstance (testName);
				obj.Invoke ("RunTest");
			}
			catch (Exception e) {
				log.Text += e.ToString() + "\n";
				Assert.Failures ++;
				Assert.TotalFailures ++;
			}

			log.Text += string.Format ("    Assertions: {0}, failures: {1}\n\n", Assert.Count, Assert.Failures);
			if (Assert.Failures > 0)
				log.Background = new SolidColorBrush(Colors.Red);

			testArea.Children.Clear ();
			status.Text = string.Format ("Assertions: {0}, Failures: {1}", Assert.TotalCount, Assert.TotalFailures);
		}
		
		public void AddTemplateItemToNewSubtree ()
		{
            		MyControl c = (MyControl)System.Windows.Markup.XamlReader.Load(@"
<clr:MyControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
               xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
               xmlns:clr=""clr-namespace:NameTortureTest;assembly=NameTortureTest""> 
    <clr:MyControl.Template>
        <ControlTemplate>
            <Canvas x:Name=""Canvas"">
                <Rectangle x:Name=""Rect"" />
                <ContentPresenter />
            </Canvas>
        </ControlTemplate>
    </clr:MyControl.Template>
</clr:MyControl>
");
			c.Content = new object ();
			c.ApplyTemplate();
			Canvas canvas = (Canvas)c.TemplateCanvas;
			Rectangle r = (Rectangle) canvas.Children [0];
			canvas.Children.Remove (r);
			testArea.Children.Add (r);
			
			Assert.IsTrue (r == c.GetTemplateChild ("Rect"), "Rect is still in template namescope");
			Assert.IsNull (testArea.FindName ("Rect"), "Rect is not added to the new namescope");
		}

		public void AddItemToCanvasInTemplate ()
		{
			Rectangle rect = new Rectangle { Name = "Rect" };
            		MyControl c = (MyControl)System.Windows.Markup.XamlReader.Load(@"
<clr:MyControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
               xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
               xmlns:clr=""clr-namespace:NameTortureTest;assembly=NameTortureTest""> 
    <clr:MyControl.Template>
        <ControlTemplate>
            <Canvas x:Name=""Canvas"">
                <ContentPresenter />
            </Canvas>
        </ControlTemplate>
    </clr:MyControl.Template>
</clr:MyControl>
");
		    c.Content = new object ();
		    c.ApplyTemplate();
		    Canvas canvas = (Canvas)c.TemplateCanvas;
		    canvas.Children.Add(rect);
		    Assert.IsTrue(rect == c.FindName("Rect"), "Element should be findable from the control");
		    Assert.IsNull (c.GetTemplateChild("Rect"), "Rect is not added to the template namescope");
		}

		public void BasicXamlReaderTests ()
		{
			// root of loaded tree is a FWE
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" 
    x:Name=""root"" >
  <Canvas.Background>
    <SolidColorBrush x:Name=""brush"" />
  </Canvas.Background>
  <Border x:Name=""border"" />
  <TextBlock Name=""text"">
    <Run Name=""run1"" />
    <Run x:Name=""run2"" />
  </TextBlock>
</Canvas>
");
			// we can use FindName on the toplevel FWE to find all objects created with x:Names in the xaml
			Assert.IsNotNull (c.FindName ("brush"), "1");
			Assert.IsNotNull (c.FindName ("border"), "2");
			Assert.IsNotNull (c.FindName ("run2"), "3");

			// and also those created with Name where appropriate
			Assert.IsNotNull (c.FindName ("text"), "4");
			Assert.IsNotNull (c.FindName ("run1"), "5");

			// Adding this canvas to the root and calling testArea.FindName does not find the elements
			testArea.Children.Add (c);
			Assert.IsNull (testArea.FindName ("brush"), "6");
			Assert.IsNull (testArea.FindName ("root"), "7");

			c.Children.Add (new Rectangle { Name = "MyName" });
			Assert.IsNotNull (c.FindName ("MyName"), "8");

			// root of loaded tree is not a FWE
			ResourceDictionary rd = (ResourceDictionary)XamlReader.Load (@"
<ResourceDictionary
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Border x:Name=""border"" />
  <Storyboard x:Name=""hi"" />
</ResourceDictionary>
");

			Assert.IsNotNull (rd["border"], "6");
			Assert.IsNotNull (((Border)rd["border"]).FindName("hi"), "7");
		}

		public void ChangeNameOfTemplatedItem ()
		{
            MyControl c = (MyControl)System.Windows.Markup.XamlReader.Load(@"
<clr:MyControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
               xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
               xmlns:clr=""clr-namespace:NameTortureTest;assembly=NameTortureTest""> 
    <clr:MyControl.Template>
        <ControlTemplate>
			<Canvas x:Name=""Parent"">
				<Canvas x:Name=""Canvas"">
					<ContentPresenter />
				</Canvas>
			</Canvas>
        </ControlTemplate>
    </clr:MyControl.Template>
</clr:MyControl>
");
		    c.Content = new object ();
		    c.ApplyTemplate();
			Canvas parent = (Canvas) c.GetTemplateChild ("Parent");
		    Canvas canvas = (Canvas) c.GetTemplateChild("Canvas");
			canvas.Name = "OtherCanvas";
			Assert.IsTrue (canvas == c.GetTemplateChild ("Canvas"), "Namescope is not updated");

			parent.Children.Remove (canvas);
			testArea.Children.Add (canvas);
			Assert.IsNull (testArea.FindName ("OtherCanvas"), "Name is not registered");

			canvas.Name = "FinalName";
			Assert.IsNull (testArea.FindName ("FinalName"), "Name changes are not registered");
		}

		public void AddXamlReaderOutputToExistingTreeVisualType ()
		{
			// first create an empty canvas
			Canvas c1 = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    x:Name=""parentCanvas"">
</Canvas>
");

			// now we create a named canvas and add it to the first canvas's children.
			Canvas c2 = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    x:Name=""nestedCanvas"">
</Canvas>
");

			c1.Children.Add (c2);

			// the names are visible in the originating canvas' namescopes
			Assert.IsNotNull (c1.FindName("parentCanvas"), "1");
			Assert.IsNotNull (c2.FindName("nestedCanvas"), "2");

			// but neither is visible in the other scopes
			Assert.IsNull (c1.FindName ("nestedCanvas"), "3");
			Assert.IsNull (c2.FindName ("parentCanvas"), "4");
		}

		public void AddXamlReaderOutputToExistingTreeNonVisualType ()
		{
			// first create an empty canvas
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    x:Name=""parentCanvas"">
</Canvas>
");
			// now we create a named brush and set it as the canvas's background
			SolidColorBrush b = (SolidColorBrush)XamlReader.Load (@"
<SolidColorBrush
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
    x:Name=""brush"" Color=""Green"" />
");

			c.Background = b;

			// the brush's name is not visible in the canvas' namescope.
			Assert.IsNull (c.FindName ("brush"), "1");
		}

		public void UserControlNamescope1 ()
		{
			UserControl1Container ucc = new UserControl1Container();

			Assert.IsNotNull (ucc.FindName("userControl1"), "1");

			/* now with the reference to userControl1,
			   look up the name inside the UserControl1's
			   definition namescope (i.e. the one used to
			   parse it) */
			UserControl1 uc1 = (UserControl1)ucc.FindName("userControl1");

			Assert.IsNotNull (uc1.FindName ("userControl1"), "2");
			

			/* now see if the two controls are sharing the
			   same namescope (i.e. it's been merged) by
			   checking to see if we can look up the
			   container's name in the embedded control's
			   namescope */

			Assert.IsNull (uc1.FindName ("userControl1Container"), "3");

			/* and an element defined in the UserControl's
			   xaml is not locatable from outside that
			   scope */

			Assert.IsNull (ucc.FindName ("border"), "4");
			Assert.IsNotNull (uc1.FindName ("border"), "5");
		}

		public void CanvasSubclassNamescope ()
		{
			CanvasSubclassContainer cc = new CanvasSubclassContainer();

			// the name defined in the definition namescope for the usercontrol is visible
			Assert.IsNotNull (cc.FindName("localNameForCanvas"), "1");

			// the overridden name takes priority here
			Assert.IsNotNull (cc.FindName("containerLocalName"), "2");

			// see what happens when we add two
			// UserControl4's with their default names
			// into the same canvas.
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />
");

			c.Children.Add (new CanvasSubclass());
			Assert.Throws<ArgumentException> ( () => c.Children.Add (new CanvasSubclass()), "3");

			c = new Canvas ();
			c.Children.Add (new CanvasSubclass());
			c.Children.Add (new CanvasSubclass());
		}

		public void CustomControlInTemplate ()
		{
			MyControl c = (MyControl)System.Windows.Markup.XamlReader.Load(@"
<clr:MyControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
               xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
               xmlns:clr=""clr-namespace:NameTortureTest;assembly=NameTortureTest""> 
    <clr:MyControl.Template>
        <ControlTemplate>
            <clr:CustomControl x:Name=""CustomControl"" />
        </ControlTemplate>
    </clr:MyControl.Template>
</clr:MyControl>
");
			c.ApplyTemplate();
			CustomControl custom = (CustomControl) c.GetTemplateChild("CustomControl");
			custom.ApplyTemplate();
			Assert.IsNotNull (custom, "Custom control should be findable");
			Assert.IsNull (custom.FindName("LayoutRoot"), "Cannot FindName 'LayoutRoot' in CustomControl");
			Assert.IsNull(c.GetTemplateChild("LayoutRoot"), "Cannot GetTemplateChild 'LayoutRoot' in MyControl");
			Assert.IsNull(c.FindName("LayoutRoot"), "Cannot FindName 'LayoutRoot' in MyControl");
		}

		public void UserControlNamescope4 ()
		{
			UserControl4Container ucc = new UserControl4Container();

			// the name defined in the definition namescope for the usercontrol is visible
			Assert.IsNotNull (ucc.FindName("localNameForUserControl4"), "1");

			// the overridden name takes priority here
			Assert.IsNotNull (ucc.FindName("containerLocalName"), "2");

			// see what happens when we add two
			// UserControl4's with their default names
			// into the same canvas.
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />
");

			c.Children.Add (new UserControl4());
			Assert.Throws<ArgumentException> ( () => c.Children.Add (new UserControl4()), "3");

			// add them to a non-xamlreader created canvas
			Canvas c2 = new Canvas ();
			c2.Children.Add (new UserControl4());
			c2.Children.Add (new UserControl4());

			// and then add that canvas to a xamlreader created canvas.
			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />
");

			Assert.Throws<ArgumentException> ( () => c.Children.Add (c2), "4");

			// add two of them but set the name on one to see if it overrides the definition scope
			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />
");
			c.Children.Add (new UserControl4());
			UserControl4 uc = new UserControl4 ();

			Assert.IsNotNull (uc.FindName ("localNameForUserControl4"), "5");

			uc.Name = "thisOtherName";

			Assert.IsNull (uc.FindName ("localNameForUserControl4"), "6");
			Assert.IsNotNull (uc.FindName ("thisOtherName"), "7");

			// all that, but we still throw?
			Assert.Throws<ArgumentException> ( () => c.Children.Add (new UserControl4()), "8");
		}

		public void NonVisualTypes ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Canvas.Background>
    <SolidColorBrush x:Name=""brush"" Color=""Green""/>
  </Canvas.Background>
</Canvas>
");

			// just for sanity's sake
			Assert.IsNotNull (c.FindName("brush"), "0");

			Brush b = c.Background;

			// Clear the property and check if that clears the name
			c.ClearValue (Panel.BackgroundProperty);
			Assert.IsNull (c.FindName("brush"), "1");

			// see if the name is reregistered when we add it back
			c.Background = b;

			Assert.IsNotNull (c.FindName("brush"), "2");

			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Canvas.Background>
    <SolidColorBrush x:Name=""brush"" Color=""Green""/>
  </Canvas.Background>
</Canvas>
");
			// setting to null?
			c.Background = null;
			Assert.IsNull (c.FindName("brush"), "3");


			// what happens when we "reparent" a nonvisual type?  does it's name move?
			c = (Canvas)XamlReader.Load (@"<Canvas
						           xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
						           xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
						         <Canvas.Background>
						           <SolidColorBrush x:Name=""brush"" Color=""Green""/>
						         </Canvas.Background>
						       </Canvas>");

			Canvas c2 = new Canvas();

			c2.Background = (Brush)c.FindName("brush");

			// it's still visible in the original canvas
			Assert.IsNotNull (c.FindName ("brush"), "4");

			// but not in the newly created one.
			Assert.IsNull (c2.FindName ("brush"), "5");

			// let's try a XamlReader.Load'ed canvas
			c2 = (Canvas)XamlReader.Load (@"<Canvas
							  xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							  xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />");

			c2.Background = (Brush)c.FindName ("brush");

			// it's still visible in the original canvas
			Assert.IsNotNull (c.FindName ("brush"), "6");

			// and it's still not visible in the newly created one.
			Assert.IsNull (c2.FindName ("brush"), "7");


			// let's try removing it from the first tree before adding it to the second
			c = (Canvas)XamlReader.Load (@"<Canvas
						           xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
						           xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
						         <Canvas.Background>
						           <SolidColorBrush x:Name=""brush"" Color=""Green""/>
						         </Canvas.Background>
						       </Canvas>");                                                    

			c2 = (Canvas)XamlReader.Load (@"<Canvas
							  xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							  xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />");

			b = c.Background;

			c.ClearValue (Panel.BackgroundProperty);
			c2.Background = b;

			// it's not visible in the original canvas
			Assert.IsNull (c.FindName ("brush"), "8");

			// and now it's visible in the new scope
			Assert.IsNotNull (c2.FindName ("brush"), "9");


			// try removing it from the first tree before adding it to the second when there's a conflict
			c = (Canvas)XamlReader.Load (@"<Canvas
						           xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
						           xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
						         <Canvas.Background>
						           <SolidColorBrush x:Name=""brush"" Color=""Green""/>
						         </Canvas.Background>
						       </Canvas>");                                                    

			Rectangle r;
			r = (Rectangle)XamlReader.Load (@"<Rectangle
							  xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
							  xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
						         <Rectangle.Fill>
						           <SolidColorBrush x:Name=""brush"" Color=""Green""/>
						         </Rectangle.Fill>
						       </Rectangle>");

			
			b = c.Background;

			c.ClearValue (Panel.BackgroundProperty);
			
			Assert.Throws<ArgumentException> ( () => r.Stroke = b, "10" );

			// let's try loading some xaml with name
			// conflicts between 2 non-visual items that
			// are properties of sibling elements.
			Assert.Throws<XamlParseException> (() => XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Canvas>
    <Canvas.Background>
      <SolidColorBrush x:Name=""brush"" Color=""Green""/>
    </Canvas.Background>
  </Canvas>

  <Canvas>
    <Canvas.Background>
      <SolidColorBrush x:Name=""brush"" Color=""Blue""/>
    </Canvas.Background>
  </Canvas>
</Canvas>
"), "11");

			// and also try loading some xaml with name
			// conflicts between 2 non-visual items that
			// are properties of the same element.
			Assert.Throws<XamlParseException> (() => XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >
  <Rectangle>
    <Rectangle.Fill>
      <SolidColorBrush x:Name=""brush"" Color=""Green""/>
    </Rectangle.Fill>

    <Rectangle.Stroke>
      <SolidColorBrush x:Name=""brush"" Color=""Blue""/>
    </Rectangle.Stroke>
  </Rectangle>
</Canvas>
"), "12");
		}

		public void VisualTypes ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");
			// just for sanity's sake
			Assert.IsNotNull (c.FindName("border"), "0");

			// Clear the children list and check if that clears the name
			c.Children.Clear();
			Assert.IsNull (c.FindName("border"), "1");


			// try again, this time remove the border and then add it back again
			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");

			Border b = (Border)c.FindName("border");
			c.Children.Remove (b);

			c.Children.Add (b);

			Assert.IsNotNull (c.FindName("border"), "2");

			// try moving it to a newly created canvas

			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");

			Canvas c2 = new Canvas ();

			b = (Border)c.FindName("border");
			c.Children.Remove (b);

			c2.Children.Add (b);

			Assert.IsNull (c.FindName("border"), "3");

			Assert.IsNull (c2.FindName("border"), "4");

			// try moving it to a XamlReader.Load'ed canvas

			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");

			c2 = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" />
");

			b = (Border)c.FindName("border");
			c.Children.Remove (b);

			c2.Children.Add (b);

			Assert.IsNull (c.FindName("border"), "5");

			Assert.IsNotNull (c2.FindName("border"), "6");


			// try adding it in a situation where it'll cause a name conflict

			c = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");

			c2 = (Canvas)XamlReader.Load (@"
<Canvas
    xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
    xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"" >

    <Border x:Name=""border"" />
</Canvas>
");

			b = (Border)c.FindName("border");
			c.Children.Remove (b);

			Assert.Throws <ArgumentException> ( () => c2.Children.Add (b), "7");
		}

		public void StaticResources ()
		{
			UserControl3Container ucc = new UserControl3Container ();

			/* make sure the brush is in the resources */
			Assert.IsNotNull (ucc.Resources["backgroundBrush"], "1");

			/* it should also be available through FindName */
			Assert.IsNotNull (ucc.FindName ("backgroundBrush"), "2");

			/* make sure the second brush (registered with x:Key) is in the resources */
			Assert.IsNotNull (ucc.Resources["backgroundBrush2"], "3");

			/* but it is not available through FindName */
			Assert.IsNull (ucc.FindName ("backgroundBrush2"), "4");

			UserControl3 uc3 = (UserControl3)ucc.FindName ("userControl3");

			// the brush's name is not available through the control's namescope.
			Assert.IsNull (uc3.FindName ("backgroundBrush"), "5");
		}

		public void StaticResourceNameLookup ()
		{
			Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
  <Canvas.Resources>
      <SolidColorBrush x:Name=""foo"" Color=""Orange""/>
  </Canvas.Resources>
  <Canvas>
    <Canvas.Resources>
      <SolidColorBrush x:Name=""bar"" Color=""Orange""/>
    </Canvas.Resources>
  </Canvas>
</Canvas>");

			Assert.IsNotNull (c.FindName ("foo"), "1");
			Assert.IsNotNull (c.FindName ("bar"), "2");
		}

		public void StaticResourceNamesConflictingOnSibling ()
		{
			Canvas c = null;

			Assert.DoesNotThrow (() => {
					c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <Grid>
        <Grid.Resources>
            <SolidColorBrush x:Name=""foo"" Color=""Orange""/>
        </Grid.Resources>
        <Rectangle Grid.Column=""0"" Grid.Row=""1"" x:Name=""rect3"" Fill=""{StaticResource foo}""/>
    </Grid>
    <Grid>
        <Grid.Resources>
            <SolidColorBrush x:Name=""foo"" Color=""Green""/>
        </Grid.Resources>
        <Rectangle Grid.Column=""0"" Grid.Row=""1"" x:Name=""rect5"" Fill=""{StaticResource foo}""/>
    </Grid>
</Canvas>
");
					Assert.IsNull (c.FindName ("foo"), "1");
				}, "2");

			Assert.Throws<XamlParseException> ( () => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <Grid>
        <Grid.Resources>
            <SolidColorBrush x:Name=""foo"" Color=""Orange""/>
        </Grid.Resources>
    </Grid>
    <Grid>
        <Grid.Resources>
            <SolidColorBrush x:Name=""foo"" Color=""Green""/>
        </Grid.Resources>
    </Grid>
    <Grid>
        <Grid.Resources>
            <SolidColorBrush x:Name=""foo"" Color=""Orange""/>
        </Grid.Resources>
    </Grid>
</Canvas>
"), "3");

			Assert.Throws<XamlParseException> ( () => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <Grid>
        <Grid.Resources>
            <Button x:Name=""foo""/>
        </Grid.Resources>
    </Grid>
    <Grid>
        <Grid.Resources>
            <Button x:Name=""foo""/>
        </Grid.Resources>
    </Grid>
</Canvas>
"), "4");

			Assert.Throws<XamlParseException> ( () => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <Grid>
        <Grid.Resources>
            <Button x:Name=""foo""/>
        </Grid.Resources>
    </Grid>
    <Grid>
        <Grid.Resources>
            <Button x:Name=""foo""/>
        </Grid.Resources>
    </Grid>
    <Grid>
        <Grid.Resources>
            <Button x:Name=""foo""/>
        </Grid.Resources>
    </Grid>
</Canvas>
"), "5");
		}

		public void StaticResourceNamesConflictingBetweenResourceAndElement ()
		{
			Assert.DoesNotThrow ( () => {
					Canvas c = (Canvas)XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <Grid>
        <Grid.Resources>
            <SolidColorBrush x:Name=""foo"" Color=""Orange""/>
        </Grid.Resources>
        <Rectangle Grid.Column=""0"" Grid.Row=""1"" x:Name=""foo"" Fill=""{StaticResource foo}""/>
    </Grid>
</Canvas>
");
					Assert.IsNotNull (c.FindName ("foo"), "1");
					Assert.IsTrue (c.FindName ("foo") is Rectangle, "2");
				}, "3");

			Assert.Throws<XamlParseException> ( () => XamlReader.Load (@"
<Canvas xmlns=""http://schemas.microsoft.com/client/2007""
        xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml"">
    <Grid>
        <Rectangle Grid.Column=""0"" Grid.Row=""1"" x:Name=""foo"" Fill=""{StaticResource foo}""/>
        <Grid.Resources>
            <SolidColorBrush x:Name=""foo"" Color=""Orange""/>
        </Grid.Resources>
    </Grid>
</Canvas>
"), "2");
		}

		public void StaticResourceUserControls ()
		{
			StaticResourceUserControls1 s1 = new StaticResourceUserControls1();

			Assert.IsNull (s1.Resources["localNameForUserControl4"], "1");
			Assert.IsNotNull (s1.Resources["foo"], "2");

			StaticResourceUserControls2 s2 = new StaticResourceUserControls2();

			Assert.IsNotNull (s2.Resources["localNameForUserControl4"], "3");
		}

		public void StoryboardTemplateItem ()
		{
            MyControl c = (MyControl)System.Windows.Markup.XamlReader.Load(@"
<clr:MyControl xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation""
               xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
               xmlns:clr=""clr-namespace:NameTortureTest;assembly=NameTortureTest""> 
    <clr:MyControl.Template>
        <ControlTemplate>
            <Canvas x:Name=""Canvas"">
                
            </Canvas>
        </ControlTemplate>
    </clr:MyControl.Template>
</clr:MyControl>
");
		    c.Content = new object ();
		    c.ApplyTemplate();
			Canvas canvas = (Canvas) c.GetTemplateChild ("Canvas");


			Storyboard sb = new Storyboard ();
			DoubleAnimation anim = new DoubleAnimation ();
			Storyboard.SetTargetName (anim, "Canvas");
			Storyboard.SetTargetProperty (anim, new PropertyPath ("Width"));
			sb.Children.Add (anim);
			Assert.Throws<InvalidOperationException> (() => sb.Begin (), "Not in tree, cannot find 'Canvas'");

			canvas.Resources.Add ("Key", sb);
			Assert.Throws<InvalidOperationException> (() => sb.Begin (), "In template resources, cannot find 'Canvas'");

			canvas.Resources.Clear ();
			c.Resources.Add ("Key", sb);
			Assert.Throws<InvalidOperationException> (() => sb.Begin (), "In control resources, cannot find 'Canvas'");
		}

		public void TemplatedControl ()
		{
			TemplatedControl c = new TemplatedControl { Name = "Custom" };
			c.ApplyTemplate ();

			Rectangle Bob = new Rectangle { Name = "Bob" };
			Storyboard resourceSB = c.Resources ["ResourceSB"] as Storyboard;
			Storyboard templateSb = (Storyboard) c.GetTemplateChild ("TemplateSB");
			Grid grid = c.TemplateGrid;
			Rectangle rect = (Rectangle) grid.FindName ("Rect");
			Rectangle fakeGrid = new Rectangle { Name = "Grid" };

			c.TemplateGrid.Children.Add (Bob);
			testArea.Children.Add (c);

			Assert.IsNull (testArea.FindName ("LayoutRoot"), "LayoutRoot.FindName (\"LayoutRoot\")");
			Assert.IsTrue (testArea.FindName ("Custom") == c, "LayoutRoot.FindName (\"Custom\")");
			Assert.IsNull (testArea.FindName ("Grid"), "LayoutRoot.FindName (\"Grid\")");
			Assert.IsNull (testArea.FindName ("Bob"), "LayoutRoot.FindName (\"Bob\")");
			Assert.IsNull (testArea.FindName ("Rect"), "LayoutRoot.FindName (\"Rect\")");

			Assert.IsNull (c.FindName ("LayoutRoot"), "c.FindName (\"LayoutRoot\")");
			Assert.IsTrue (c.FindName ("Custom") == c, "c.FindName (\"Custom\")");
			Assert.IsNull (c.FindName ("Grid"), "c.FindName (\"Grid\")");
			Assert.IsTrue (c.FindName ("Bob") == Bob, "c.FindName (\"Bob\")");
			Assert.IsNull (c.FindName ("Rect"), "c.FindName (\"Rect\")");

			Assert.IsNull (grid.FindName ("LayoutRoot"), "grid.FindName (\"LayoutRoot\")");
			Assert.IsNull (grid.FindName ("Custom"), "grid.FindName (\"Custom\")");
			Assert.IsTrue (grid.FindName ("Grid") == grid, "grid.FindName (\"Grid\")");
			Assert.IsNull (grid.FindName ("Bob"), "grid.FindName (\"Bob\")");
			Assert.IsTrue (grid.FindName ("Rect") == rect, "grid.FindName (\"Rect\")");

			Assert.IsNull (Bob.FindName ("LayoutRoot"), "Bob.FindName (\"LayoutRoot\")");
			Assert.IsTrue (Bob.FindName ("Custom") == c, "Bob.FindName (\"Custom\")");
			Assert.IsNull (Bob.FindName ("Grid"), "Bob.FindName (\"Grid\")");
			Assert.IsTrue (Bob.FindName ("Bob") == Bob, "Bob.FindName (\"Bob\")");
			Assert.IsNull (Bob.FindName ("Rect"), "Bob.FindName (\"Rect\")");

			Assert.IsNull (rect.FindName ("LayoutRoot"), "rect.FindName (\"LayoutRoot\")");
			Assert.IsNull (rect.FindName ("Custom"), "rect.FindName (\"Custom\")");
			Assert.IsTrue (rect.FindName ("Grid") == c.TemplateGrid, "rect.FindName (\"Grid\")");
			Assert.IsNull (rect.FindName ("Bob"), "rect.FindName (\"Bob\")");
			Assert.IsTrue (rect.FindName ("Rect") == rect, "rect.FindName (\"Rect\")");
		}

		public void UserControlEmbeddedInXaml ()
		{
			Assert.Throws <XamlParseException> (() => new UserControl2Container(), "1");
		}
	}
}
