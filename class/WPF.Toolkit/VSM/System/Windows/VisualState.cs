// -------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All Rights Reserved.
// -------------------------------------------------------------------

using System.Windows;
using System.Windows.Markup;
using System.Windows.Media.Animation;

namespace System.Windows
{
    /// <summary>
    ///     A visual state that can be transitioned into.
    /// </summary>
    [ContentProperty("Storyboard")]
    [RuntimeNameProperty("Name")]
    public class VisualState : DependencyObject
    {
	    internal static DependencyProperty NameProperty = DependencyProperty.Lookup (Kind.DEPENDENCY_OBJECT, "Name", typeof (string));

        /// <summary>
        ///     The name of the VisualState.
        /// </summary>
        public string Name
        {
		get { return (string) GetValue (NameProperty); }
		set { SetValue (NameProperty, value); }
        }

        private static readonly DependencyProperty StoryboardProperty =
            DependencyProperty.Register(
            "Storyboard",
            typeof(Storyboard),
            typeof(VisualState));

        /// <summary>
        ///     Storyboard defining the values of properties in this visual state.
        /// </summary>
        public Storyboard Storyboard
        {
            get { return (Storyboard)GetValue(StoryboardProperty); }
            set { SetValue(StoryboardProperty, value); }
        }
    }
}
