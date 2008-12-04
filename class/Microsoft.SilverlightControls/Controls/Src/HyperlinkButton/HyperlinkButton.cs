// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.ComponentModel; 
using System.Diagnostics; 
using System.Globalization;
//using System.Windows.Browser; 
using System.Windows.Controls.Primitives;
using System.Windows.Media.Animation;
 
namespace System.Windows.Controls
{
    /// <summary> 
    /// Represents a control that displays a link to another website. 
    /// </summary>
    [TemplatePart(Name = HyperlinkButton.ElementRootName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = HyperlinkButton.ElementFocusVisualName, Type = typeof(UIElement))]
    [TemplatePart(Name = HyperlinkButton.StateNormalName, Type = typeof(Storyboard))]
    [TemplatePart(Name = HyperlinkButton.StateMouseOverName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = HyperlinkButton.StatePressedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = HyperlinkButton.StateDisabledName, Type = typeof(Storyboard))]
    public partial class HyperlinkButton : ButtonBase 
    { 
        #region NavigateUri
        /// <summary> 
        /// Gets or sets a URI to navigate to when the link is clicked.
        /// </summary>
        [TypeConverter(typeof(UriTypeConverter))] 
        public Uri NavigateUri
        {
            get { return GetValue(NavigateUriProperty) as Uri; } 
            set { SetValue(NavigateUriProperty, value); } 
        }
 
        /// <summary>
        /// Identifies the NavigateUri dependency property.
        /// </summary> 
        public static readonly DependencyProperty NavigateUriProperty =
            DependencyProperty.Register(
                "NavigateUri", 
                typeof(Uri), 
                typeof(HyperlinkButton),
                null); 
        #endregion NavigateUri

        #region TargetName 
        /// <summary>
        /// Gets or sets the name of a target window or frame.
        /// </summary> 
        public string TargetName 
        {
            get { return GetValue(TargetNameProperty) as string; } 
            set { SetValue(TargetNameProperty, value); }
        }
 
        /// <summary>
        /// Identifies the TargetName dependency property.
        /// </summary> 
        public static readonly DependencyProperty TargetNameProperty = 
            DependencyProperty.Register(
                "TargetName", 
                typeof(string),
                typeof(HyperlinkButton),
                null); 
        #endregion TargetName

        #region Template Parts 
        /// <summary> 
        /// Root template part of the HyperlinkButton.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal FrameworkElement _elementRoot;
        internal const string ElementRootName = "RootElement"; 

        /// <summary>
        /// Focus visual template part of the HyperlinkButton. 
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal UIElement _elementFocusVisual; 
        internal const string ElementFocusVisualName = "FocusVisualElement";

        /// <summary> 
        /// Normal state of the HyperlinkButton.
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal Storyboard _stateNormal; 
        internal const string StateNormalName = "Normal State";
 
        /// <summary>
        /// MouseOver state of the HyperlinkButton.
        /// </summary> 
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _stateMouseOver;
        internal const string StateMouseOverName = "MouseOver State"; 
 
        /// <summary>
        /// Pressed state of the HyperlinkButton. 
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks>
        internal Storyboard _statePressed; 
        internal const string StatePressedName = "Pressed State";

        /// <summary> 
        /// Disabled state of the HyperlinkButton. 
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal Storyboard _stateDisabled;
        internal const string StateDisabledName = "Disabled State";
        #endregion Template Parts 

        /// <summary>
        /// Initializes a new instance of the HyperlinkButton class. 
        /// </summary> 
        public HyperlinkButton()
        { 
            // IsTabStop should always match IsEnabled (which is false by
            // default) to prevent tabbing into disabled buttons.
            IsTabStop = false; 
        }

        /// <summary> 
        /// Apply a template to the HyperlinkButton. 
        /// </summary>
        public override void OnApplyTemplate() 
        {
            base.OnApplyTemplate();
 
            // Get the elements
            object root = GetTemplateChild(ElementRootName);
            Debug.Assert(typeof(FrameworkElement).IsInstanceOfType(root) || (root == null), 
                "The template part RootElement is not an instance of FrameworkElement!"); 
            _elementRoot = root as FrameworkElement;
 
            object focusVisual = GetTemplateChild(ElementFocusVisualName);
            Debug.Assert(typeof(UIElement).IsInstanceOfType(focusVisual) || (focusVisual == null),
                "The template part FocusVisualElement is not an instance of UIElement!"); 
            _elementFocusVisual = focusVisual as UIElement;

            // Get the states 
            if (_elementRoot != null) 
            {
                object normal = _elementRoot.Resources[StateNormalName]; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(normal) || (normal == null),
                    "The template part Normal State is not an instance of Storyboard!");
                _stateNormal = normal as Storyboard; 

                object mouseOver = _elementRoot.Resources[StateMouseOverName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(mouseOver) || (mouseOver == null), 
                    "The template part MouseOver State is not an instance of Storyboard!"); 
                _stateMouseOver = mouseOver as Storyboard;
 
                object pressed = _elementRoot.Resources[StatePressedName];
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(pressed) || (pressed == null),
                    "The template part Pressed State is not an instance of Storyboard!"); 
                _statePressed = pressed as Storyboard;

                object disabled = _elementRoot.Resources[StateDisabledName]; 
                Debug.Assert(typeof(Storyboard).IsInstanceOfType(disabled) || (disabled == null), 
                    "The template part Disabled State is not an instance of Storyboard!");
                _stateDisabled = disabled as Storyboard; 
            }

            // Sync the logical and visual states of the control 
            UpdateVisualState();
        }
 
        /// <summary> 
        /// Change to the correct visual state for the HyperlinkButton.
        /// </summary> 
        internal override void ChangeVisualState()
        {
            if (!IsEnabled) 
            {
                ChangeVisualState(_stateDisabled ??
                    _stateNormal); 
            } 
            else if (IsPressed)
            { 
                ChangeVisualState(_statePressed ??
                    _stateMouseOver ??
                    _stateNormal); 
            }
            else if (IsMouseOver)
            { 
                ChangeVisualState(_stateMouseOver ?? 
                    _stateNormal);
            } 
            else
            {
                ChangeVisualState(_stateNormal); 
            }

            if (_elementFocusVisual != null) 
            { 
                _elementFocusVisual.Visibility = (IsFocused && IsEnabled) ?
                    Visibility.Visible : 
                    Visibility.Collapsed;
            }
        } 

        /// <summary>
        /// Raises the Click routed event. 
        /// </summary> 
        protected override void OnClick()
        { 
            base.OnClick();

            // Navigate to the link when not in design mode 
            if ((NavigateUri != null) && !DesignerProperties.GetIsInDesignMode(this))
            {
                Navigate(); 
            } 
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
        /// Get an absolute Uri corresponding to the NavigateUri.
        /// </summary> 
        /// <returns>Absolute Uri corresponding to the NavigateUri.</returns>
        internal Uri GetAbsoluteUri()
        { 
            // Get the destination (relative to the application path)
            Uri destination = NavigateUri;
            Debug.Assert(destination != null, 
                "NavigateUri should not be null!"); 

            // Make relative Uris absolute 
            if (!destination.IsAbsoluteUri)
            {
                // Page relative Uris are invalid 
                string original = destination.OriginalString;
                if (!string.IsNullOrEmpty(original) && (original[0] != '/'))
                { 
                    throw new NotSupportedException(Resource.HyperlinkButton_GetAbsoluteUri_PageRelativeUri); 
                }
                else if (Application.Current == null) 
                {
                    throw new NotSupportedException(Resource.HyperlinkButton_GetAbsoluteUri_NoApplication);
                } 

                // Make the Uri relative to the application root
                destination = new Uri(Application.Current.Host.Source, destination); 
            } 

            return destination; 
        }

        /// <summary> 
        /// Navigate to the Uri.
        /// </summary>
        private void Navigate() 
        { 
		throw new NotImplementedException ();
//            Uri destination = GetAbsoluteUri();
// 
//            string target = TargetName;
//            try
//            { 
//                if (!string.IsNullOrEmpty(target))
//                {
//                    HtmlPage.Window.Navigate(destination, target); 
//                } 
//                else
//                { 
//                    HtmlPage.Window.Navigate(destination);
//                }
//            } 
//            catch (InvalidOperationException ex)
//            {
//                throw new InvalidOperationException( 
//                    string.Format(CultureInfo.InvariantCulture, 
//                        Resource.HyperlinkButton_Navigate_Failed,
//                        destination.ToString()), 
//                    ex);
//            }
        } 
    }
}
