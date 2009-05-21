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

namespace MoonTest.System.Windows {

	[TestClass]
	public partial class VisualStateManagerTest : SilverlightTest {

		[TestMethod]
		public void SetCustomVSMNullDO ()
		{
			Assert.Throws (() => {
				VisualStateManager.SetCustomVisualStateManager (null, new VisualStateManager ());
			}, typeof (ArgumentNullException));
		}

		[TestMethod]
		public void GetCustomVSMNullDO ()
		{
			Assert.Throws (() => VisualStateManager.GetCustomVisualStateManager (null), typeof (ArgumentNullException));
		}
		
		[TestMethod]
		[Asynchronous]
		[MoonlightBug ("Namescope issues - when VSM calls 'start' on the storyboard, the rectangle can't be found")]
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
	}
}
