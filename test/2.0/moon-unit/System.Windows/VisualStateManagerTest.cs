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
using Microsoft.Silverlight.Testing;
using VSMTest;

namespace MoonTest.System.Windows {

	[TestClass]
	public partial class VisualStateManagerTest : SilverlightTest {

		const string RootName = "Grid";

		[TestMethod]
		[Asynchronous]
		public void AddStateAtRuntime ()
		{
			Rectangle rect = new Rectangle { Name = RootName };
			VSMControl c = new VSMControl ();
			c.ApplyTemplate ();

			// Create a visual state in code which we will try to use to animate the template element called 'Grid'
			foreach (VisualStateGroup g in VisualStateManager.GetVisualStateGroups (c.TemplateGrid)) {
				VisualState s = new VisualState ();
				s.SetValue (FrameworkElement.NameProperty, "C");
				s.Storyboard = CreateWidthStoryboard (RootName, 600, 700);
				g.States.Add (s);
			}

			// The template element "Grid" can't be found by the new storyboard
			CreateAsyncTest (c,
				() => {
					Assert.IsTrue (VisualStateManager.GoToState (c, "A", false), "#1");
					Assert.Throws<InvalidOperationException> (() => VisualStateManager.GoToState (c, "C", false), "#2");

					// Adding a new element called 'Grid' to the TestPanel does not work
					TestPanel.Children.Add (rect);
					Assert.Throws<InvalidOperationException> (() => VisualStateManager.GoToState (c, "C", false), "#3");

					// The new element is not findable from the control
					Assert.IsNull (c.FindName (RootName), "#4");
					Assert.AreSame (rect, TestPanel.FindName (RootName), "#5");

					// Adding it to the template grid instead
					TestPanel.Children.Remove (rect);
					c.TemplateGrid.Children.Add (rect);

					// It's not findable from the panel, but it is from the Control
					Assert.AreSame (rect, c.FindName (RootName), "#6");
					Assert.IsNull (TestPanel.FindName (RootName), "#7");

					// Once it's findable from the control, the storyboard will resolve to the new element and succeed
					Assert.IsTrue (VisualStateManager.GoToState (c, "C", false), "#8");
			},
			() => {
					Assert.AreEqual (700, rect.Width, "#9");
					// The template element 'Grid' is not changed
					Assert.IsTrue (Double.IsNaN (c.TemplateGrid.Width), "#10");
				}
			);
		}

		[TestMethod]
		public void SetCustomVSMNullDO ()
		{
			Assert.Throws (() => {
				VisualStateManager.SetCustomVisualStateManager (null, new VisualStateManager ());
			}, typeof (ArgumentNullException));
		}

		[TestMethod]
		public void SetCustomVSMNonControl ()
		{
			VisualStateManager.SetCustomVisualStateManager (new Rectangle (), new VisualStateManager ());
		}

		[TestMethod]
		public void GetCustomVSMNullDO ()
		{
			Assert.Throws (() => VisualStateManager.GetCustomVisualStateManager (null), typeof (ArgumentNullException));
		}
		
		[TestMethod]
		[Asynchronous]
		public void ManagedVisualStates ()
		{
			Rectangle r = new Rectangle { Name = "rect", Width = 0 };
			VisualStateGroup group = new VisualStateGroup ();
			group.SetValue (FrameworkElement.NameProperty, "group");

			VisualState vstate = new VisualState ();
			vstate.SetValue (FrameworkElement.NameProperty, "state");

			DoubleAnimation anim = new DoubleAnimation { From = 100, To = 200, Duration = TimeSpan.FromSeconds (1) };
			anim.SetValue (Control.NameProperty, "ANIMATION");
			Storyboard.SetTargetName (anim, "rect");
			Storyboard.SetTargetProperty (anim, new PropertyPath ("Width"));

			Storyboard sb = new Storyboard ();
			sb.Children.Add (anim);
			vstate.Storyboard = sb;
			group.States.Add (vstate);

			Enqueue (() => TestPanel.Children.Add (r));
			Enqueue (() => VisualStateManager.GetVisualStateGroups ((FrameworkElement)TestPanel.Parent).Add (group));
			Enqueue (() => Assert.AreEqual(0, r.Width, "#1"));
			Enqueue (() => Assert.IsTrue (VisualStateManager.GoToState ((Control) TestPage, "state", false), "#2"));
			Enqueue (() => Assert.IsGreater (99, r.Width, "#3"));
			Enqueue (() => TestPanel.Children.Clear ());
			Enqueue (() => VisualStateManager.GetVisualStateGroups (TestPage).Clear ());
			EnqueueTestComplete();
		}
		
		[TestMethod]
		[Asynchronous]
		public void ManagedVisualStates2 ()
		{
			Rectangle r = new Rectangle { Width = 0 };
			VisualStateGroup group = new VisualStateGroup ();
			group.SetValue (FrameworkElement.NameProperty, "group2");

			// Create the first visual state
			VisualState vstate = new VisualState ();
			vstate.SetValue (FrameworkElement.NameProperty, "state2");

			DoubleAnimation anim = new DoubleAnimation { From = 100, To = 200, Duration = TimeSpan.FromSeconds (1) };
			Storyboard.SetTarget (anim, r);
			Storyboard.SetTargetProperty (anim, new PropertyPath ("Width"));

			Storyboard sb = new Storyboard ();
			sb.Children.Add (anim);
			vstate.Storyboard = sb;
			group.States.Add (vstate);

			// Create the second visual state
			VisualState vstate2 = new VisualState ();
			vstate2.SetValue (FrameworkElement.NameProperty, "state3");

			Storyboard sb2 = new Storyboard ();
			Storyboard.SetTarget (sb2, r);
			Storyboard.SetTargetProperty (sb2, new PropertyPath ("Width"));

			vstate2.Storyboard = sb2;
			group.States.Add (vstate2);

			// Ensure that the values reset correct
			Enqueue (() => TestPanel.Children.Add (r));
			Enqueue (() => VisualStateManager.GetVisualStateGroups ((FrameworkElement)TestPanel.Parent).Add (group));
			Enqueue (() => Assert.AreEqual(0, r.Width, "#1"));
			Enqueue (() => Assert.IsTrue (VisualStateManager.GoToState (TestPage, "state2", false), "#2"));
			Enqueue (() => Assert.IsGreater (99, r.Width, "#3"));
			Enqueue (() => Assert.IsTrue (VisualStateManager.GoToState (TestPage, "state3", false), "#3"));
			Enqueue (() => Assert.AreEqual (0, r.Width, "#4"));
			Enqueue (() => Assert.AreEqual (ClockState.Filling, sb2.GetCurrentState (), "#5"));
			Enqueue (() => TestPanel.Children.Clear ());
			Enqueue (() => VisualStateManager.GetVisualStateGroups (TestPage).Clear ());
			EnqueueTestComplete();
		}

		[TestMethod]
		[Asynchronous]
		public void TargetControlInTemplate ()
		{
			Control c = new VSMTest.VSMControl ();
			Grid g = null;
			c.ApplyTemplate ();
			CreateAsyncTest (c,
				() => g = (Grid) VisualTreeHelper.GetChild (c, 0),
				() => Assert.IsTrue (VisualStateManager.GoToState (c, "A", false), "#1"),
				() => Assert.AreEqual (100, g.Width, "#2"),
				() => Assert.IsTrue (VisualStateManager.GoToState (c, "B", false), "#3"),
				() => Assert.AreEqual (200, g.Width, "#4"),
				() => Assert.IsTrue (VisualStateManager.GoToState (c, "A", true), "#5"),
				() => Assert.AreEqual (100, g.Width, "#6"),
				() => Assert.IsTrue (VisualStateManager.GoToState (c, "B", true), "#7"),
				() => Assert.AreEqual (200, g.Width, "#8")
			);
		}


		Storyboard CreateWidthStoryboard (string targetName, double from, double to)
		{
			DoubleAnimation anim = new DoubleAnimation { To = to, From = from, Duration = TimeSpan.Zero };
			Storyboard.SetTargetName (anim, targetName);
			Storyboard.SetTargetProperty (anim, new PropertyPath ("Width"));
			Storyboard sb = new Storyboard { };
			sb.Children.Add (anim);
			return sb;
		}

		Control CreateVSMControl (string targetName)
		{
			return (Control) XamlReader.Load (string.Format (@"
<ContentControl
	xmlns=""http://schemas.microsoft.com/winfx/2006/xaml/presentation"" 
	xmlns:x=""http://schemas.microsoft.com/winfx/2006/xaml""
	xmlns:vsm=""clr-namespace:System.Windows;assembly=System.Windows""	
	>
    <ContentControl.Template>
        <ControlTemplate>
            <Grid x:Name=""{1}"">
                <vsm:VisualStateManager.VisualStateGroups>
                    <vsm:VisualStateGroup x:Name=""CommonStates"">
                        <VisualState x:Name=""A"">
                            <Storyboard>
                                <DoubleAnimation Storyboard.TargetName=""{0}"" Storyboard.TargetProperty=""Width"" From=""1"" To=""100"" Duration=""0"" />
                            </Storyboard>
                        </VisualState>
                        <VisualState x:Name=""B"">
                            <Storyboard>
                                <DoubleAnimation Storyboard.TargetName=""{0}"" Storyboard.TargetProperty=""Width"" From=""101"" To=""200"" Duration=""0"" />
                            </Storyboard>
                        </VisualState>
                    </vsm:VisualStateGroup>
                </vsm:VisualStateManager.VisualStateGroups>
            </Grid>
        </ControlTemplate>
    </ContentControl.Template>
	<ContentControl.Content>
		<Rectangle />
	</ContentControl.Content>
</ContentControl>", targetName, RootName));
		}
	}
}
