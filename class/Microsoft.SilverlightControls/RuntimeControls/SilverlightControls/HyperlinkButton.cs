// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 

using System;
using System.ComponentModel; 
using System.Diagnostics; 
using System.Globalization;
using System.Windows; 
using System.Windows.Controls.Primitives;
using System.Windows.Media.Animation;
using System.Windows.Automation.Peers; 
using System.Security;
#if !BOOTSTRAP
using System.Windows.Browser;
#endif 

namespace System.Windows.Controls
{
    /// <summary> 
    /// Represents a control that displays a link to another website.
    /// </summary>
    [TemplateVisualState(Name = HyperlinkButton.StateNormal, GroupName = HyperlinkButton.GroupCommon)] 
    [TemplateVisualState(Name = HyperlinkButton.StateMouseOver, GroupName = HyperlinkButton.GroupCommon)]
    [TemplateVisualState(Name = HyperlinkButton.StatePressed, GroupName = HyperlinkButton.GroupCommon)]
    [TemplateVisualState(Name = HyperlinkButton.StateDisabled, GroupName = HyperlinkButton.GroupCommon)] 
    [TemplateVisualState(Name = HyperlinkButton.StateUnfocused, GroupName = HyperlinkButton.GroupFocus)] 
    [TemplateVisualState(Name = HyperlinkButton.StateFocused, GroupName = HyperlinkButton.GroupFocus)]
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
            DependencyProperty.RegisterCore(
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
            DependencyProperty.RegisterCore(
                "TargetName", 
                typeof(string),
                typeof(HyperlinkButton),
                null); 
        #endregion TargetName 

        /// <summary> 
        /// Initializes a new instance of the HyperlinkButton class.
        /// </summary>
        public HyperlinkButton() 
        {
            DefaultStyleKey = typeof(HyperlinkButton);
        } 
 
        /// <summary>
        /// Apply a template to the HyperlinkButton. 
        /// </summary>
        public override void OnApplyTemplate()
        { 
            base.OnApplyTemplate();

            // Sync the logical and visual states of the control 
            UpdateVisualState(false); 
        }
 
        /// <summary>
        /// Change to the correct visual state for the HyperlinkButton.
        /// </summary> 
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state. 
        /// </param> 
        internal override void ChangeVisualState(bool useTransitions)
        { 
            if (!IsEnabled)
            {
                GoToState(useTransitions, StateDisabled); 
            }
            else if (IsPressed)
            { 
                GoToState(useTransitions, StatePressed); 
            }
            else if (IsMouseOver) 
            {
                GoToState(useTransitions, StateMouseOver);
            } 
            else
            {
                GoToState(useTransitions, StateNormal); 
            } 

            if (IsFocused && IsEnabled) 
            {
                GoToState(useTransitions, StateFocused);
            } 
            else
            {
                GoToState(useTransitions, StateUnfocused); 
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
                    throw new NotSupportedException(); 
                } 
                else if (Application.Current == null)
                { 
                    throw new NotSupportedException();
                }
 
                // Make the Uri relative to the application root
                destination = new Uri(Application.Current.Host.Source, destination);
            } 
 
            return destination;
        } 

        /// <summary>
        /// Navigate to the Uri. 
        /// </summary>
        /// <SecurityNote>
        /// Safe because we own the caller and the code this method pinvokes to. 
        /// </SecurityNote> 
        private void Navigate()
        { 
            Uri destination = GetAbsoluteUri();
            string target = TargetName;
 
            if (!destination.IsAbsoluteUri || !IsSafeScheme(destination.Scheme))
            {
                throw new InvalidOperationException(); 
            } 

            try 
            {
                if (!string.IsNullOrEmpty(target))
                { 
#if !BOOTSTRAP
                    HtmlPage.Window.Navigate(destination, target);
#endif
                }
                else 
                { 
#if !BOOTSTRAP
                    HtmlPage.Window.Navigate(destination, "_self");
#endif
                } 
            }
            catch (InvalidOperationException)
            { 
                throw new InvalidOperationException();
            }
        } 
 
        private bool IsSafeScheme(string scheme)
        { 
            if (scheme.Equals("http", StringComparison.OrdinalIgnoreCase) ||
                scheme.Equals("https", StringComparison.OrdinalIgnoreCase) ||
                scheme.Equals("file", StringComparison.OrdinalIgnoreCase) || 
                scheme.Equals("mailto", StringComparison.OrdinalIgnoreCase) ||
                scheme.Equals("mms", StringComparison.OrdinalIgnoreCase))
            { 
                return true; 
            }
            else 
            {
                return false;
            } 
        }

        protected override AutomationPeer OnCreateAutomationPeer ()
        {
            return new HyperlinkButtonAutomationPeer (this);
        }
    }
} 
