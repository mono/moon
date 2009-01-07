// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.ComponentModel; 
using System.Diagnostics; 
using System.Windows.Documents;
using System.Windows.Markup; 
using System.Windows.Media;
using System.Windows.Controls;

namespace System.Windows.Controls
{
    /// <summary>
    /// Represents a control with a single piece of content. 
    /// </summary> 
    /// <remarks>
    /// A ContentControl has a limited default style.  If you want to enhance 
    /// the appearance of the control, you can create a new control template
    /// that uses &lt;ContentPresenter Content="{TemplateBinding Content}" /&gt;
    /// to display the value of the Content property. 
    /// </remarks>
    [ContentProperty("Content")]
    public partial class ContentControl : Control 
    { 
        #region Content
        /// <summary> 
        /// Gets or sets the content of a ContentControl.
        /// </summary>
        public object Content 
        {
            get { return GetValue(ContentProperty); }
            set { SetValue(ContentProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the Content dependency property.
        /// </summary>
        public static readonly DependencyProperty ContentProperty = 
            DependencyProperty.Register(
                "Content",
                typeof(object), 
                typeof(ContentControl), 
                new PropertyMetadata(OnContentPropertyChanged));
 
        /// <summary>
        /// ContentProperty property changed handler.
        /// </summary> 
        /// <param name="d">ContentControl that changed its Content.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnContentPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            ContentControl source = d as ContentControl;
            Debug.Assert(source != null, 
                "The source is not an instance of ContentControl!");

            // Notify derived classes of the change 
            source.OnContentChanged(e.OldValue, e.NewValue);
        }
        #endregion Content 
 
        #region ContentTemplate
        /// <summary> 
        /// Gets or sets the data template used to display the content of the
        /// ContentControl.
        /// </summary> 
        public DataTemplate ContentTemplate
        {
            get { return GetValue(ContentTemplateProperty) as DataTemplate; } 
            set { SetValue(ContentTemplateProperty, value); } 
        }
 
        /// <summary>
        /// Identifies the ContentTemplate dependency property.
        /// </summary> 
        public static readonly DependencyProperty ContentTemplateProperty =
            DependencyProperty.Register(
                "ContentTemplate", 
                typeof(DataTemplate), 
                typeof(ContentControl),
                new PropertyMetadata(OnContentTemplatePropertyChanged)); 

        /// <summary>
        /// ContentTemplateProperty property changed handler. 
        /// </summary>
        /// <param name="d">ContentControl that changed its ContentTemplate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnContentTemplatePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            ContentControl source = d as ContentControl; 
            Debug.Assert(source != null,
                "The source is not an instance of ContentControl!");
 
            Debug.Assert(typeof(DataTemplate).IsInstanceOfType(e.NewValue) || (e.NewValue == null),
                "The value is not an instance of DataTemplate!");
            DataTemplate value = (DataTemplate) e.NewValue; 
 
            Debug.Assert(typeof(DataTemplate).IsInstanceOfType(e.OldValue) || (e.OldValue == null),
                "The old value is not an instance of DataTemplate!"); 
            DataTemplate oldValue = (DataTemplate) e.OldValue;

            // Notify derived classes of the change 
//            source.OnContentTemplateChanged(oldValue, value);
        }
        #endregion ContentTemplate 
#if false
        #region IsEnabled 
        /// <summary>
        /// Gets or sets a value that indicates whether this element is enabled 
        /// in the user interface (UI).
        /// </summary>
        public bool IsEnabled 
        {
            get { return (bool) GetValue(IsEnabledProperty); }
            set { SetValue(IsEnabledProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the IsEnabled dependency property.
        /// </summary>
        public static readonly DependencyProperty IsEnabledProperty = 
            DependencyProperty.Register(
                "IsEnabled",
                typeof(bool), 
                typeof(ContentControl), 
                new PropertyMetadata(OnIsEnabledPropertyChanged));
 
        /// <summary>
        /// IsEnabledProperty property changed handler.
        /// </summary> 
        /// <param name="d">ContentControl that changed IsEnabled.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsEnabledPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            ContentControl source = d as ContentControl;
            Debug.Assert(source != null, 
                "The source is not an instance of ContentControl!");

            Debug.Assert(typeof(bool).IsInstanceOfType(e.NewValue), 
                "The value is not an instance of bool!");
            bool value = (bool) e.NewValue;
 
            source.OnIsEnabledChanged(value); 
        }
        #endregion IsEnabled 

        #region TextAlignment
        /// <summary> 
        /// Gets or sets a value that indicates the horizontal alignment of text
        /// content.
        /// </summary> 
        public TextAlignment TextAlignment 
        {
            get { return (TextAlignment) GetValue(TextAlignmentProperty); } 
            set { SetValue(TextAlignmentProperty, value); }
        }
 
        /// <summary>
        /// Identifies the TextAlignment dependency property.
        /// </summary> 
        public static readonly DependencyProperty TextAlignmentProperty = 
            DependencyProperty.Register(
                "TextAlignment", 
                typeof(TextAlignment),
                typeof(ContentControl),
                null); 
        #endregion TextAlignment

        #region TextDecorations 
        /// <summary> 
        /// Gets or sets a TextDecorationCollection that contains the effects to
        /// apply to the Content text. 
        /// </summary>
        public TextDecorationCollection TextDecorations 
        {
            get { return GetValue(TextDecorationsProperty) as TextDecorationCollection; }
            set { SetValue(TextDecorationsProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the TextDecorations dependency property.
        /// </summary>
        public static readonly DependencyProperty TextDecorationsProperty = 
            DependencyProperty.Register(
                "TextDecorations",
                typeof(TextDecorationCollection), 
                typeof(ContentControl), 
                null);
        #endregion TextDecorations 

        #region TextWrapping
        /// <summary> 
        /// Gets or sets a value that indicates how any Content text should be
        /// wrapped.
        /// </summary> 
        public TextWrapping TextWrapping 
        {
            get { return (TextWrapping) GetValue(TextWrappingProperty); } 
            set { SetValue(TextWrappingProperty, value); }
        }
 
        /// <summary>
        /// Identifies the TextWrapping dependency property.
        /// </summary> 
        public static readonly DependencyProperty TextWrappingProperty = 
            DependencyProperty.Register(
                "TextWrapping", 
                typeof(TextWrapping),
                typeof(ContentControl),
                null); 
        #endregion TextWrapping


        #region ToolTip 
        /// <summary> 
        /// Gets or sets the tool-tip object that is displayed for this element
        /// in the user interface (UI). 
        /// </summary>
        public object ToolTip
        { 
            get { return ToolTipService.GetToolTip(this); }
            set { ToolTipService.SetToolTip(this, value); }
        } 
 
        #endregion ToolTip
#endif
        /// <summary>
        /// Initializes a new instance of the ContentControl class.
        /// </summary> 
        public ContentControl()
        {
		DefaultStyleKey = typeof (ContentControl);
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
        protected virtual void OnContentChanged(object oldContent, object newContent)
        { 
        }

#if false
        /// <summary> 
        /// Called when the ContentTemplate property changes.
        /// </summary>
        /// <param name="oldContentTemplate"> 
        /// The old value of the ContentTemplate property. 
        /// </param>
        /// <param name="newContentTemplate"> 
        /// The new value of the ContentTemplate property.
        /// </param>
        internal virtual void OnContentTemplateChanged(DataTemplate oldContentTemplate, DataTemplate newContentTemplate) 
        {
        }

        /// <summary> 
        /// Called when the IsEnabled property changes.
        /// </summary> 
        /// <param name="isEnabled">New value of the IsEnabled property.</param>
        protected virtual void OnIsEnabledChanged(bool isEnabled)
        { 
        }
#endif
    }
} 
