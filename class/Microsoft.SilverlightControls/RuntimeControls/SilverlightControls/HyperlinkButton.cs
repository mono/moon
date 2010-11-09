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
using System.Windows.Media;
using System.Collections.Generic;
using Mono;
using System.Windows.Interop;
#endif 

namespace System.Windows.Controls
{
	// These are a series of tests which can't be made into moonunits as they require user-initiated events
	// The test the behaviour of HyperlinkButton under various permutations.

	// Navigates to /test
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("/test", UriKind.Relative), TargetName = null };
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("/test", UriKind.Relative), TargetName = "" };

	// Navigates to /test - new window
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("/test", UriKind.Relative), TargetName = "asdf" };

	// Throws invalid operation exception
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("test", UriKind.Relative), TargetName = null };
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("test", UriKind.Relative), TargetName = "" };
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("test", UriKind.Relative), TargetName = "asdf" };

	// Downloads the xap - same window
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("", UriKind.Relative), TargetName = null };
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("", UriKind.Relative), TargetName = "" };

	// Downloads the xap - new window
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("", UriKind.Relative), TargetName = "asdasf" };

	// Navigates to Ireland.com - same window
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("http://www.ireland.com", UriKind.Absolute), TargetName = null };
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("http://www.ireland.com", UriKind.Absolute), TargetName = "" };

	// Navigates to Ireland.com - new window
	//new HyperlinkButton { Content = "Click", NavigateUri = new Uri("http://www.ireland.com", UriKind.Absolute), TargetName = "sdfljh" };

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
#if !BOOTSTRAP
        // If the targetname is one of these, it counts as an external target.
        // The list is compiled from the MSDN docs.
        static readonly List<string> ExternalTargets = new List<string> {
            "_blank",
            "_media",
            "_parent",
            "_search",
            "_self",
            "_top",
        };
#endif
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
                // Page relative Uris are not invalid
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
            string target = TargetName;
            var navigateUri = NavigateUri;
            try
            {
#if !BOOTSTRAP
                if (ExternalTargets.Contains (TargetName))
                {
                    if (!Settings.EnableNavigation)
                        throw new InvalidOperationException ("Cannot navigate to an external site when Navigation is disabled");

                    Uri destination = GetAbsoluteUri();

                    if (!destination.IsAbsoluteUri || !IsSafeScheme(destination.Scheme))
                    {
                        throw new InvalidOperationException();
                    }

                    if (Helper.IsUserInitiated ())
                        HtmlPage.Window.Navigate(destination, TargetName);
                }
                else 
                {
                    if (TryNavigateInternally (NavigateUri))
                        return;

                    if (navigateUri.IsAbsoluteUri || navigateUri.OriginalString.StartsWith ("/") || string.IsNullOrEmpty (navigateUri.OriginalString)) {
                        if (!Settings.EnableNavigation)
                            throw new InvalidOperationException ("Cannot navigate to an external site when Navigation is disabled");
                        if (Helper.IsUserInitiated ())
                            HtmlPage.Window.Navigate(GetAbsoluteUri (), TargetName ?? "");
                    } else {
                        throw new NotSupportedException (string.Format ("Could not find the navigator specified by the TargetName '{0}'", TargetName));
                    }
                }
#endif
            }
            catch (InvalidOperationException)
            {
                throw new InvalidOperationException();
            }
        }
 
#if !BOOTSTRAP
        bool TryNavigateInternally (Uri destination)
        {
            UIElement e = (UIElement) VisualTreeHelper.GetParent (this);
            UIElement skip = this;
            INavigate navigator;
            while (e != null && !FindNavigatorInSubtree (e, skip, out navigator)) {
                skip = e;
                e = (UIElement)VisualTreeHelper.GetParent (e);
            }

            if (navigator == null)
                return false;
            navigator.Navigate (destination);
            return true;
        }
        
        bool FindNavigatorInSubtree (UIElement e, UIElement skip, out INavigate navigator)
        {
            // The docs say that HyperLinkButton searches the visual tree so that's what I implemented
            // Maybe it actually uses FindName.
            navigator = e as INavigate;
            if (navigator != null) {
		    if (!string.IsNullOrEmpty (TargetName)) {
			    // if we have a target name make sure it matches
			    if (navigator is FrameworkElement && ((FrameworkElement)navigator).Name == TargetName)
				    return true;
		    }
		    else {
			    // if we don't have a target name, take what we can get
			    return true;
		    }
	    }

			// Popups are treated specially as they don't have a regular visual child.
			// Needed to pass DRT HyperlinkButtonP2.
            int count = e is Popup ? 1 : VisualTreeHelper.GetChildrenCount (e);
            for (int i = 0; i < count; i++) {
		    UIElement ui = e is Popup ? ((Popup)e).Child : (UIElement) VisualTreeHelper.GetChild (e, i);
		    if (ui == skip)
			    continue;
		    if (FindNavigatorInSubtree (ui, null, out navigator))
			    return true;
	    }
            return false;
        }
#endif

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
