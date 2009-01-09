// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Diagnostics; 
using System.Windows; 
using System.Windows.Controls;
using System.Windows.Input; 
using System.Windows.Media.Animation;

#if WPF 
using PropertyChangedCallback = System.Windows.FrameworkPropertyMetadata;
#endif
 
#if WPF 
namespace WPF
#else 
namespace System.Windows.Controls
#endif
{ 
    /// <summary>
    /// Represents a selectable item in a ListBox.
    /// </summary> 
    [TemplatePart(Name = ListBoxItem.ElementRootName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = ListBoxItem.ElementFocusVisualName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = ListBoxItem.StateNormalName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = ListBoxItem.StateSelectedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ListBoxItem.StateSelectedFocusedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ListBoxItem.StateMouseOverName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = ListBoxItem.StateMouseOverSelectedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ListBoxItem.StateMouseOverSelectedFocusedName, Type = typeof(Storyboard))]
    public class ListBoxItem : ContentControl 
    { 
        /// <summary>
        /// Gets or sets a value that indicates whether a ListBoxItem is selected. 
        /// </summary>
        public bool IsSelected
        { 
            get { return (bool)GetValue(IsSelectedProperty); }
            set { SetValue(IsSelectedProperty, value); }
        } 
 
        /// <summary>
        /// Identifies the IsSelected dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsSelectedProperty = DependencyProperty.Register(
            "IsSelected", typeof(bool), typeof(ListBoxItem), 
            new PropertyMetadata(new PropertyChangedCallback(OnIsSelectedChanged)));

        /// <summary> 
        /// Gets a value that indicates whether the mouse pointer is located over this element. 
        /// </summary>
        internal bool IsMouseOver { get; private set; } 

        /// <summary>
        /// Identifies the ItemsControl item represented by this object (null when this object IS the item) 
        /// </summary>
        internal object Item { get; set; }
 
        /// <summary> 
        /// Identifies the parent ListBox.
        /// </summary> 
        internal ListBox ParentListBox { get; set; }

        /// <summary> 
        /// Identifies the root visual element from the template.
        /// </summary>
        private FrameworkElement ElementRoot { get; set; } 
        private const string ElementRootName = "RootElement"; 

        /// <summary> 
        /// Identifies the focus visual element from the template.
        /// </summary>
        private FrameworkElement ElementFocusVisual { get; set; } 
        private const string ElementFocusVisualName = "FocusVisualElement";

        /// <summary> 
        /// Identifies the normal state. 
        /// </summary>
        private Storyboard StateNormal 
        {
            get { return _stateNormal; }
            set { _stateNormal = value; } 
        }
        private Storyboard _stateNormal;
        private const string StateNormalName = "Normal State"; 
 
        /// <summary>
        /// Identifies the selected state (with fallback). 
        /// </summary>
        private Storyboard StateSelected
        { 
            get { return _stateSelected ?? StateSelectedFocused; }
            set { _stateSelected = value; }
        } 
        private Storyboard _stateSelected; 
        private const string StateSelectedName = "Unfocused Selected State";
 
        /// <summary>
        /// Identifies the selected+focused state (with fallback).
        /// </summary> 
        private Storyboard StateSelectedFocused
        {
            get { return _stateSelectedFocused ?? StateNormal; } 
            set { _stateSelectedFocused = value; } 
        }
        private Storyboard _stateSelectedFocused; 
        private const string StateSelectedFocusedName = "Normal Selected State";

        /// <summary> 
        /// Identifies the normal+mouse state (with fallback).
        /// </summary>
        private Storyboard StateMouseOver 
        { 
            get { return _stateMouseOver ?? StateNormal; }
            set { _stateMouseOver = value; } 
        }
        private Storyboard _stateMouseOver;
        private const string StateMouseOverName = "MouseOver State"; 

        /// <summary>
        /// Identifies the selected+mouse state (with fallback). 
        /// </summary> 
        private Storyboard StateMouseOverSelected
        { 
            get { return _stateMouseOverSelected ?? StateMouseOverSelectedFocused; }
            set { _stateMouseOverSelected = value; }
        } 
        private Storyboard _stateMouseOverSelected;
        private const string StateMouseOverSelectedName = "MouseOver Unfocused Selected State";
 
        /// <summary> 
        /// Identifies the selected+mouse+focused state (with fallback).
        /// </summary> 
        private Storyboard StateMouseOverSelectedFocused
        {
            get { return _stateMouseOverSelectedFocused ?? StateSelectedFocused; } 
            set { _stateMouseOverSelectedFocused = value; }
        }
        private Storyboard _stateMouseOverSelectedFocused; 
        private const string StateMouseOverSelectedFocusedName = "MouseOver Selected State"; 

        /// <summary> 
        /// Identifies the current state.
        /// </summary>
        private Storyboard CurrentState { get; set; } 

        /// <summary>
        /// Initializes a new instance of the ListBoxItem class. 
        /// </summary> 
        public ListBoxItem()
        { 
#if WPF
            KeyboardNavigation.SetDirectionalNavigation(this, KeyboardNavigationMode.Once);
            KeyboardNavigation.SetTabNavigation(this, KeyboardNavigationMode.Local); 
#else
            // DirectionalNavigation not supported by Silverlight
            TabNavigation = KeyboardNavigationMode.Local; 
#endif 
            IsTabStop = true;
            MouseLeftButtonDown += delegate(object sender, MouseButtonEventArgs e) 
            {
                OnMouseLeftButtonDown(e);
            }; 
            MouseEnter += delegate(object sender, MouseEventArgs e)
            {
                OnMouseEnter(e); 
            }; 
            MouseLeave += delegate(object sender, MouseEventArgs e)
            { 
                OnMouseLeave(e);
            };
            GotFocus += delegate(object sender, RoutedEventArgs e) 
            {
                OnGotFocus(e);
            }; 
            LostFocus += delegate(object sender, RoutedEventArgs e) 
            {
                OnLostFocus(e); 
            };
        }
 
#if WPF
        public override void OnApplyTemplate()
#else 
        /// <summary> 
        /// Invoked whenever application code or internal processes call
        /// ApplyTemplate. 
        /// </summary>
        public override void OnApplyTemplate()
#endif 
        {
            base.OnApplyTemplate();
 
            ElementRoot = GetTemplateChild(ElementRootName) as FrameworkElement; 
            if (null != ElementRoot)
            { 
                StateNormal = ElementRoot.Resources[StateNormalName] as Storyboard;
                StateSelected = ElementRoot.Resources[StateSelectedName] as Storyboard;
                StateSelectedFocused = ElementRoot.Resources[StateSelectedFocusedName] as Storyboard; 
                StateMouseOver = ElementRoot.Resources[StateMouseOverName] as Storyboard;
                StateMouseOverSelected = ElementRoot.Resources[StateMouseOverSelectedName] as Storyboard;
                StateMouseOverSelectedFocused = ElementRoot.Resources[StateMouseOverSelectedFocusedName] as Storyboard; 
            } 
            ElementFocusVisual = GetTemplateChild(ElementFocusVisualName) as FrameworkElement;
        } 

        /// <summary>
        /// Called when the user presses the left mouse button over the ListBoxItem. 
        /// </summary>
        /// <param name="e">The event data.</param>
        protected virtual void OnMouseLeftButtonDown(MouseButtonEventArgs e) 
        { 
            if (!e.Handled)
            { 
                e.Handled = true;
                if (Focus())
                { 
                    if (null != ParentListBox)
                    {
                        ParentListBox.NotifyListItemClicked(this); 
                    } 
                }
            } 
        }

        /// <summary> 
        /// Called when the mouse pointer enters the bounds of this element.
        /// </summary>
        /// <param name="e">The event data.</param> 
        protected virtual void OnMouseEnter(MouseEventArgs e) 
        {
            IsMouseOver = true; 
            ChangeVisualState();
        }
 
        /// <summary>
        /// Called when the mouse pointer leaves the bounds of this element.
        /// </summary> 
        /// <param name="e">The event data.</param> 
        protected virtual void OnMouseLeave(MouseEventArgs e)
        { 
            IsMouseOver = false;
            ChangeVisualState();
        } 

        /// <summary>
        /// Called when the control got focus. 
        /// </summary> 
        /// <param name="e">The event data.</param>
        protected virtual void OnGotFocus(RoutedEventArgs e) 
        {
            if (null != ElementFocusVisual)
            { 
                ElementFocusVisual.Visibility = Visibility.Visible;
            }
            if (null != ParentListBox) 
            { 
                ParentListBox.NotifyListItemGotFocus(this);
            } 
        }

        /// <summary> 
        /// Called when the control lost focus.
        /// </summary>
        /// <param name="e">The event data.</param> 
        protected virtual void OnLostFocus(RoutedEventArgs e) 
        {
            if (null != ElementFocusVisual) 
            {
                ElementFocusVisual.Visibility = Visibility.Collapsed;
            } 
            if (null != ParentListBox)
            {
                ParentListBox.NotifyListItemLostFocus(this); 
            } 
        }
 
        /// <summary>
        /// Implements the IsSelectedProperty PropertyChangedCallback.
        /// </summary> 
        /// <param name="d">The DependencyObject for which the property changed.</param>
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param>
        private static void OnIsSelectedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            ListBoxItem listBoxItem = d as ListBoxItem;
            Debug.Assert(null != listBoxItem); 
            Debug.Assert(typeof(bool).IsInstanceOfType(e.OldValue));
            Debug.Assert(typeof(bool).IsInstanceOfType(e.NewValue));
            listBoxItem.ChangeVisualState(); 
        }

        /// <summary> 
        /// Changes the visual state by playing the appropriate Storyboard 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes", Justification = "An invalid Storyboard or playing a Storyboard at the wrong time throws System.Exception.")] 
        internal void ChangeVisualState()
        {
            if (null != ParentListBox) 
            {
                Storyboard newState = null;
                if (IsSelected) 
                { 
                    newState = (ListBox.GetIsSelectionActive(ParentListBox) ?
                        (IsMouseOver ? StateMouseOverSelectedFocused : StateSelectedFocused) : 
                        (IsMouseOver ? StateMouseOverSelected : StateSelected));
                }
                else 
                {
                    newState = (IsMouseOver ? StateMouseOver : StateNormal);
                } 
                if ((null != newState) && (CurrentState != newState)) 
                {
                    // Begin the new state transition 
                    try
                    {
                        newState.Begin( 
#if WPF
                            ElementRoot
#endif 
                        ); 
                        // Update the current state
                        Storyboard previousState = CurrentState; 
                        CurrentState = newState;
                        // Stop the old state transition (per Silverlight convention)
                        if (null != previousState) 
                        {
                            previousState.Stop(
#if WPF 
                                ElementRoot 
#endif
                            ); 
                        }
                    }
                    catch 
                    {
                        // Silverlight Storyboard.Begin must always be wrapped in try/catch
                    } 
                } 
            }
        } 
    }
}
