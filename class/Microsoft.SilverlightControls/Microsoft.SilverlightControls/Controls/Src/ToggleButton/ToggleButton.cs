// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.ComponentModel; 
using System.Diagnostics; 
using System.Globalization;
using System.Windows.Media.Animation;
using System.Windows.Controls;

namespace System.Windows.Controls.Primitives
{ 
    /// <summary>
    /// Base class for controls that can switch states, such as CheckBox.
    /// </summary> 
    [TemplatePart(Name = ToggleButton.ElementRootName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = ToggleButton.ElementFocusVisualName, Type = typeof(UIElement))]
    [TemplatePart(Name = ToggleButton.ElementContentFocusVisualName, Type = typeof(UIElement))] 
    [TemplatePart(Name = ToggleButton.StateNormalName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ToggleButton.StateCheckedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ToggleButton.StateIndeterminateName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = ToggleButton.StateMouseOverCheckedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ToggleButton.StateMouseOverIndeterminateName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ToggleButton.StateMouseOverUncheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = ToggleButton.StatePressedCheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = ToggleButton.StatePressedIndeterminateName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ToggleButton.StatePressedUncheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = ToggleButton.StateDisabledCheckedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ToggleButton.StateDisabledIndeterminateName, Type = typeof(Storyboard))]
    [TemplatePart(Name = ToggleButton.StateDisabledUncheckedName, Type = typeof(Storyboard))] 
    public partial class ToggleButton : ButtonBase
    {
        /// <summary> 
        /// True to ignore indeterminate states when changing the visuals, false 
        /// to include them.
        /// </summary> 
        /// <remarks>
        /// This will be set true by RadioButton so it can make use of
        /// ToggleButton's ChangeVisualState method. 
        /// </remarks>
        internal bool _ignoreIndeterminateStates;
 
        #region IsChecked 
        /// <summary>
        /// Gets or sets whether the ToggleButton is checked. 
        /// </summary>
        [TypeConverter(typeof(NullableBoolConverter))]
        public bool? IsChecked 
        {
            get { return GetValue(IsCheckedProperty) as bool?; }
            set { SetValue(IsCheckedProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the IsChecked dependency property.
        /// </summary>
        public static readonly DependencyProperty IsCheckedProperty = 
            DependencyProperty.Register(
                "IsChecked",
                typeof(bool?), 
                typeof(ToggleButton), 
                new PropertyMetadata(OnIsCheckedPropertyChanged));
 
        /// <summary>
        /// IsCheckedProperty property changed handler.
        /// </summary> 
        /// <param name="d">ToggleButton that changed its IsChecked.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsCheckedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            ToggleButton source = d as ToggleButton;
            System.Diagnostics.Debug.Assert(source != null, 
                "The source is not an instance of ToggleButton!");

            System.Diagnostics.Debug.Assert(typeof(bool?).IsInstanceOfType(e.NewValue) || (e.NewValue == null), 
                "The value is not an instance of bool?!");
            bool? value = (bool?) e.NewValue;
 
            // Raise the appropriate changed event 
            RoutedEventArgs args = new RoutedEventArgs () { OriginalSource = source};
            if (value == true)
            {
                source.OnChecked(args); 
            }
            else if (value == false)
            { 
                source.OnUnchecked(args); 
            }
            else 
            {
                source.OnIndeterminate(args);
            } 
        }
        #endregion IsChecked
 
        #region IsThreeState 
        /// <summary>
        /// Determines whether the control supports two or three states. 
        /// </summary>
        public bool IsThreeState
        { 
            get { return (bool) GetValue(IsThreeStateProperty); }
            set { SetValue(IsThreeStateProperty, value); }
        } 
 
        /// <summary>
        /// Identifies the IsThreeState dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsThreeStateProperty =
            DependencyProperty.Register( 
                "IsThreeState",
                typeof(bool),
                typeof(ToggleButton), 
                null); 
        #endregion IsThreeState
 
        #region Template Parts
        /// <summary>
        /// Root template part of the ToggleButton. 
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal FrameworkElement _elementRoot; 
        internal const string ElementRootName = "RootElement"; 

        /// <summary> 
        /// Focus visual template part of the ToggleButton.
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal UIElement _elementFocusVisual;
        internal const string ElementFocusVisualName = "FocusVisualElement";
 
        /// <summary> 
        /// Content focus visual template part of the ToggleButton.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal UIElement _elementContentFocusVisual;
        internal const string ElementContentFocusVisualName = "ContentFocusVisualElement"; 

        /// <summary>
        /// Normal checked state of the ToggleButton. 
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _stateChecked; 
        internal const string StateCheckedName = "Checked State";

        /// <summary> 
        /// Normal indeterminate state of the ToggleButton.
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal Storyboard _stateIndeterminate; 
        internal const string StateIndeterminateName = "Indeterminate State";
 
        /// <summary>
        /// Normal unchecked state of the ToggleButton.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _stateNormal;
        internal const string StateNormalName = "Normal State"; 
 
        /// <summary>
        /// MouseOver and checked state of the ToggleButton. 
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _stateMouseOverChecked; 
        internal const string StateMouseOverCheckedName = "MouseOver Checked State";

        /// <summary> 
        /// MouseOver and indeterminate state of the ToggleButton. 
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal Storyboard _stateMouseOverIndeterminate;
        internal const string StateMouseOverIndeterminateName = "MouseOver Indeterminate State";
 
        /// <summary>
        /// MouseOver and unchecked state of the ToggleButton.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal Storyboard _stateMouseOverUnchecked;
        internal const string StateMouseOverUncheckedName = "MouseOver Unchecked State"; 

        /// <summary>
        /// Pressed and checked state of the ToggleButton. 
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _statePressedChecked; 
        internal const string StatePressedCheckedName = "Pressed Checked State"; 

        /// <summary> 
        /// Pressed and indeterminate state of the ToggleButton.
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal Storyboard _statePressedIndeterminate;
        internal const string StatePressedIndeterminateName = "Pressed Indeterminate State";
 
        /// <summary> 
        /// Pressed and unchecked state of the ToggleButton.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _statePressedUnchecked;
        internal const string StatePressedUncheckedName = "Pressed Unchecked State"; 

        /// <summary>
        /// Disabled and checked state of the ToggleButton. 
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _stateDisabledChecked; 
        internal const string StateDisabledCheckedName = "Disabled Checked State";

        /// <summary> 
        /// Disabled and indeterminate state of the ToggleButton.
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal Storyboard _stateDisabledIndeterminate; 
        internal const string StateDisabledIndeterminateName = "Disabled Indeterminate State";
 
        /// <summary>
        /// Disabled and unchecked state of the ToggleButton.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _stateDisabledUnchecked;
        internal const string StateDisabledUncheckedName = "Disabled Unchecked State"; 
        #endregion Template Parts 

        /// <summary> 
        /// Occurs when a ToggleButton is checked.
        /// </summary>
        public event RoutedEventHandler Checked; 

        /// <summary>
        /// Occurs when the state of a ToggleButton is neither on nor off. 
        /// </summary> 
        public event RoutedEventHandler Indeterminate;
 
        /// <summary>
        /// Occurs when a ToggleButton is unchecked.
        /// </summary> 
        public event RoutedEventHandler Unchecked;

        /// <summary> 
        /// Initializes a new instance of the ToggleButton class. 
        /// </summary>
        public ToggleButton() 
        {
            // IsTabStop should always match IsEnabled (which is false by
            // default) to prevent tabbing into disabled buttons. 
            IsTabStop = false;

// 
IsChecked = false; 
        }
 
        /// <summary>
        /// Apply a template to the ToggleButton.
        /// </summary> 
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate(); 
 
            object root = GetTemplateChild(ElementRootName);
            Debug.Assert(typeof(FrameworkElement).IsInstanceOfType(root) || (root == null), 
                "The template part RootElement is not an instance of FrameworkElement!");
            _elementRoot = root as FrameworkElement;
 
            object focusVisual = GetTemplateChild(ElementFocusVisualName);
            Debug.Assert(typeof(UIElement).IsInstanceOfType(focusVisual) || (focusVisual == null),
                "The template part FocusVisualElement is not an instance of UIElement!"); 
            _elementFocusVisual = focusVisual as UIElement; 

            object contentFocusVisual = GetTemplateChild(ElementContentFocusVisualName); 
            Debug.Assert(typeof(UIElement).IsInstanceOfType(contentFocusVisual) || (contentFocusVisual == null),
                "The template part ContentFocusVisualElement is not an instance of UIElement!");
            _elementContentFocusVisual = contentFocusVisual as UIElement; 

            // Get the states
            if (_elementRoot != null) 
            { 
                object @checked = _elementRoot.Resources[StateCheckedName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(@checked) || (@checked == null), 
                    "The template part Checked State is not an instance of Storyboard!");
                _stateChecked = @checked as Storyboard;
 
                object indeterminate = _elementRoot.Resources[StateIndeterminateName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(indeterminate) || (indeterminate == null),
                    "The template part Indeterminate State is not an instance of Storyboard!"); 
                _stateIndeterminate = indeterminate as Storyboard; 

                object @unchecked = _elementRoot.Resources[StateNormalName]; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(@unchecked) || (@unchecked == null),
                    "The template part Normal State is not an instance of Storyboard!");
                _stateNormal = @unchecked as Storyboard; 

                object mouseOverChecked = _elementRoot.Resources[StateMouseOverCheckedName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(mouseOverChecked) || (mouseOverChecked == null), 
                    "The template part MouseOver Checked State is not an instance of Storyboard!"); 
                _stateMouseOverChecked = mouseOverChecked as Storyboard;
 
                object mouseOverIndeterminate = _elementRoot.Resources[StateMouseOverIndeterminateName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(mouseOverIndeterminate) || (mouseOverIndeterminate == null),
                    "The template part MouseOver Indeterminate State is not an instance of Storyboard!"); 
                _stateMouseOverIndeterminate = mouseOverIndeterminate as Storyboard;

                object mouseOverUnchecked = _elementRoot.Resources[StateMouseOverUncheckedName]; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(mouseOverUnchecked) || (mouseOverUnchecked == null), 
                    "The template part MouseOver Unchecked State is not an instance of Storyboard!");
                _stateMouseOverUnchecked = mouseOverUnchecked as Storyboard; 

                object pressedChecked = _elementRoot.Resources[StatePressedCheckedName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(pressedChecked) || (pressedChecked == null), 
                    "The template part Pressed Checked State is not an instance of Storyboard!");
                _statePressedChecked = pressedChecked as Storyboard;
 
                object pressedIndeterminate = _elementRoot.Resources[StatePressedIndeterminateName]; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(pressedIndeterminate) || (pressedIndeterminate == null),
                    "The template part Pressed Indeterminate State is not an instance of Storyboard!"); 
                _statePressedIndeterminate = pressedIndeterminate as Storyboard;

                object pressedUnchecked = _elementRoot.Resources[StatePressedUncheckedName]; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(pressedUnchecked) || (pressedUnchecked == null),
                    "The template part Pressed Unchecked State is not an instance of Storyboard!");
                _statePressedUnchecked = pressedUnchecked as Storyboard; 
 
                object disabledChecked = _elementRoot.Resources[StateDisabledCheckedName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(disabledChecked) || (disabledChecked == null), 
                    "The template part Disabled Checked State is not an instance of Storyboard!");
                _stateDisabledChecked = disabledChecked as Storyboard;
 
                object disabledIndeterminate = _elementRoot.Resources[StateDisabledIndeterminateName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(disabledIndeterminate) || (disabledIndeterminate == null),
                    "The template part Disabled Indeterminate State is not an instance of Storyboard!"); 
                _stateDisabledIndeterminate = disabledIndeterminate as Storyboard; 

                object disabledUnchecked = _elementRoot.Resources[StateDisabledUncheckedName]; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(disabledUnchecked) || (disabledUnchecked == null),
                    "The template part Disabled Unchecked State is not an instance of Storyboard!");
                _stateDisabledUnchecked = disabledUnchecked as Storyboard; 
            }

            // Sync the logical and visual states of the control 
            UpdateVisualState(); 
        }
 
        /// <summary>
        /// Change to the correct visual state for the ToggleButton.
        /// </summary> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity", Justification = "Required to support default states.")]
        internal override void ChangeVisualState()
        { 
            // Cache dependency properties we'll check more than once 
            bool? isChecked = IsChecked;
            bool isEnabled = IsEnabled; 

            // Potentially exclude the indeterminate states
            if (_ignoreIndeterminateStates) 
            {
                isChecked = isChecked ?? (bool?) false;
            } 
 
            // Synchronize the visual state with the logical state
            if (!isEnabled) 
            {
                if (isChecked == true)
                { 
                    ChangeVisualState(_stateDisabledChecked ??
                        _stateChecked);
                } 
                else if (isChecked == false) 
                {
                    ChangeVisualState(_stateDisabledUnchecked ?? 
                        _stateNormal);
                }
                else 
                {
                    ChangeVisualState(_stateDisabledIndeterminate ??
                        _stateIndeterminate ?? 
                        _stateDisabledUnchecked ?? 
                        _stateNormal);
                } 
            }
            else if (IsPressed)
            { 
                if (isChecked == true)
                {
                    ChangeVisualState(_statePressedChecked ?? 
                        _stateMouseOverChecked ?? 
                        _stateChecked);
                } 
                else if (isChecked == false)
                {
                    ChangeVisualState(_statePressedUnchecked ?? 
                        _stateMouseOverUnchecked ??
                        _stateNormal);
                } 
                else 
                {
                    ChangeVisualState(_statePressedIndeterminate ?? 
                        _stateMouseOverIndeterminate ??
                        _stateIndeterminate ??
                        _statePressedUnchecked ?? 
                        _stateMouseOverUnchecked ??
                        _stateNormal);
                } 
            } 
            else if (IsMouseOver)
            { 
                if (isChecked == true)
                {
                    ChangeVisualState(_stateMouseOverChecked ?? 
                        _stateChecked);
                }
                else if (isChecked == false) 
                { 
                    ChangeVisualState(_stateMouseOverUnchecked ??
                        _stateNormal); 
                }
                else
                { 
                    ChangeVisualState(_stateMouseOverIndeterminate ??
                        _stateIndeterminate ??
                        _stateMouseOverUnchecked ?? 
                        _stateNormal); 
                }
            } 
            else
            {
                if (isChecked == true) 
                {
                    ChangeVisualState(_stateChecked);
                } 
                else if (isChecked == false) 
                {
                    ChangeVisualState(_stateNormal); 
                }
                else
                { 
                    ChangeVisualState(_stateIndeterminate ??
                        _stateNormal);
                } 
            } 

            // Toggle the visibility of the focus visuals 
            object content = Content;
            Visibility focusVisibility = (IsFocused && isEnabled) ?
                Visibility.Visible : 
                Visibility.Collapsed;
            if ((content != null) && (_elementContentFocusVisual != null))
            { 
                // Display the ContentFocusVisual when we have Content 
                _elementContentFocusVisual.Visibility = focusVisibility;
                if (_elementFocusVisual != null) 
                {
                    _elementFocusVisual.Visibility = Visibility.Collapsed;
                } 
            }
            else if (_elementFocusVisual != null)
            { 
                // Otherwise display the FocusVisual 
                _elementFocusVisual.Visibility = focusVisibility;
                if (_elementContentFocusVisual != null) 
                {
                    _elementContentFocusVisual.Visibility = Visibility.Collapsed;
                } 
            }
            else if (_elementContentFocusVisual != null)
            { 
                // Turn off the ContentFocusVisual if it shouldn't be displayed 
                _elementContentFocusVisual.Visibility = Visibility.Collapsed;
            } 
        }

        /// <summary> 
        /// Called when a ToggleButton raises a Checked event.
        /// </summary>
        /// <param name="e">The event data for the Checked event.</param> 
        protected virtual void OnChecked(RoutedEventArgs e) 
        {
            UpdateVisualState(); 

            RoutedEventHandler handler = Checked;
            if (handler != null) 
            {
                handler(this, e);
            } 
        } 

        /// <summary> 
        /// Called when a ToggleButton raises an Indeterminate event.
        /// </summary>
        /// <param name="e">The event data for the Indeterminate event.</param> 
        protected virtual void OnIndeterminate(RoutedEventArgs e)
        {
            UpdateVisualState(); 
 
            RoutedEventHandler handler = Indeterminate;
            if (handler != null) 
            {
                handler(this, e);
            } 
        }

        /// <summary> 
        /// Called when a ToggleButton raises an Unchecked event. 
        /// </summary>
        /// <param name="e">The event data for the Unchecked event.</param> 
        protected virtual void OnUnchecked(RoutedEventArgs e)
        {
            UpdateVisualState(); 

            RoutedEventHandler handler = Unchecked;
            if (handler != null) 
            { 
                handler(this, e);
            } 
        }

        /// <summary> 
        /// Move the button to its next IsChecked value.
        /// </summary>
        protected virtual void OnToggle() 
        { 
            bool? isChecked = IsChecked;
            if (isChecked == true) 
            {
                IsChecked = IsThreeState ? null : ((bool?) false);
            } 
            else
            {
                IsChecked = (bool?) isChecked.HasValue; 
            } 
        }
 
        /// <summary>
        /// Called when the Content property changes.
        /// </summary> 
        /// <param name="oldContent">
        /// The old value of the Content property.
        /// </param> 
        /// <param name="newContent"> 
        /// The new value of the Content property.
        /// </param> 
        protected override void OnContentChanged(object oldContent, object newContent)
        {
            base.OnContentChanged(oldContent, newContent); 
            UpdateVisualState();
        }
 
        /// <summary> 
        /// Raises the Click routed event.
        /// </summary> 
        protected override void OnClick()
        {
            OnToggle(); 
            base.OnClick();
        }
 
        /// <summary> 
        /// Called when the IsEnabled property changes.
        /// </summary> 
        /// <param name="isEnabled">New value of the IsEnabled property.</param>
        internal override void OnIsEnabledChanged(bool isEnabled)
        { 
            base.OnIsEnabledChanged(isEnabled);

            // Sync IsTabStop with IsEnabled so that disabled buttons won't 
            // receive focus.  Unfortunately because we can't receive change 
            // notifications on IsTabStop, we can't preserve a user supplied
            // value when IsEnabled = true and have to clobber it. 
            IsTabStop = isEnabled;
        }
 
        /// <summary>
        /// Returns the string representation of a ToggleButton.
        /// </summary> 
        /// <returns>String representation of a ToggleButton.</returns> 
        public override string ToString()
        { 
            string original = base.ToString();
            string content = (Content ?? "").ToString();
            bool? isChecked = IsChecked; 
            return string.Format(CultureInfo.InvariantCulture,
                Resource.ToggleButton_ToString_FormatString,
                original, 
                content, 
                isChecked.HasValue ? isChecked.Value.ToString() : "null");
        } 
    }
}
