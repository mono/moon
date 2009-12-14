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
    /// Displays the content of a ContentControl. 
    /// </summary> 
    /// <remarks>
    /// Typically, you the ContentPresenter directly within the ControlTemplate 
    /// of a ContentControl to mark where the content should be displayed.
    /// Every type derived from ContentControl should have a ContentPrenter in
    /// its ControlTemplate (although it may not necessarily be a TemplatePart). 
    /// The ContentPresenter in the ControlTemplate should use a TemplateBinding
    /// to associate ContentControl.Content with ContentPresenter.Content (and
    /// an other relevant properties like FontSize, VeriticalAlignment, etc.). 
    /// </remarks> 
    public class ContentPresenter : FrameworkElement
    { 
        /// <summary>
        /// XAML markup used to define the write-once ContentPresenter template.
        /// </summary> 
        private const string ContentPresenterDefaultTemplate =
            "<ControlTemplate " +
              "xmlns=\"http://schemas.microsoft.com/client/2007\" " + 
              "xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\" " + 
//              "xmlns:controls=\"clr-namespace:System.Windows.Controls;assembly=System.Windows.Controls\" " +
              "xmlns:controls=\"clr-namespace:System.Windows.Controls;assembly=System.Windows\" " +
              "TargetType=\"controls:ContentPresenter\">" + 
                "<Grid x:Name=\"RootElement\" " +
//                  "Background=\"{TemplateBinding Background}\" " +
                  "Cursor=\"{TemplateBinding Cursor}\">" + 
                    "<TextBlock x:Name=\"TextElement\" " +
//                      "FontFamily=\"{TemplateBinding FontFamily}\" " +
//                      "FontSize=\"{TemplateBinding FontSize}\" " + 
//                      "FontStretch=\"{TemplateBinding FontStretch}\" " + 
//                      "FontStyle=\"{TemplateBinding FontStyle}\" " +
//                      "FontWeight=\"{TemplateBinding FontWeight}\" " + 
//                      "Foreground=\"{TemplateBinding Foreground}\" " +
                      "HorizontalAlignment=\"{TemplateBinding HorizontalContentAlignment}\" " +
//                      "Padding=\"{TemplateBinding Padding}\" " + 
//                      "TextAlignment=\"{TemplateBinding TextAlignment}\" " +
//                      "TextDecorations=\"{TemplateBinding TextDecorations}\" " +
//                      "TextWrapping=\"{TemplateBinding TextWrapping}\" " + 
                      "VerticalAlignment=\"{TemplateBinding VerticalContentAlignment}\" " + 
                      "Visibility=\"Collapsed\" />" +
                "</Grid>" + 
            "</ControlTemplate>";

        /// <summary> 
        /// Root element of the panel that will contain the visual content.
        /// </summary>
        /// <remarks> 
        /// Since a Control cannot change its content (i.e. it may only call 
        /// InitializeFromXaml once to set its root), every ContentPresetner
        /// will create a container as its root element regardless of the 
        /// content that will be stuffed into it.  It's also worth noting that
        /// there is no TemplatePartAttribute because the Template is write-once
        /// and cannot be changed by users.  This field is marked internal for 
        /// unit testing.
        /// </remarks>
        internal Grid _elementRoot; 
        internal const string RootElementName = "RootElement"; 

        /// <summary> 
        /// Visual representation of the Content currently being displayed.
        /// </summary>
        /// <remarks>This field is marked internal for unit testing.</remarks> 
        internal UIElement _elementContent;

        /// <summary> 
        /// Visual representation of any text Content currently being displayed. 
        /// </summary>
        /// <remarks> 
        /// The _elementText property is necessary in addition to the
        /// _elementContent property because of all of all the font related
        /// properties whose values need to be propogated into it by the 
        /// ContentPresenter.  This field is marked internal for unit testing.
        /// </remarks>
        internal TextBlock _elementText; 
        internal const string ElementTextContentName = "TextElement"; 

        /// <summary> 
        /// True if the template has already been applied, false otherwise.
        /// </summary>
        internal bool _alreadyAppliedTemplate; 

        #region Content
        /// <summary> 
        /// Gets or sets the data used to generate the contentPresenter elements of a 
        /// ContentPresenter.
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
                typeof(ContentPresenter),
                new PropertyMetadata(OnContentPropertyChanged)); 
 
        /// <summary>
        /// ContentProperty property changed handler. 
        /// </summary>
        /// <param name="d">ContentPresenter that changed its Content.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnContentPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ContentPresenter source = d as ContentPresenter; 
            Debug.Assert(source != null, 
                "The source is not an instance of ContentPresenter!");
 
            // Use the Content as the DataContext to enable bindings in
            // ContentTemplate
            if (source.ContentTemplate != null) 
            {
                source.DataContext = e.NewValue;
            } 
 
            // Display the Content
            source.PrepareContentPresenter(); 
        }
        #endregion Content
 
        #region ContentTemplate
        /// <summary>
        /// Gets or sets the data template used to display the content of the 
        /// ContentPresenter. 
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
                typeof(ContentPresenter),
                new PropertyMetadata(OnContentTemplatePropertyChanged));
 
        /// <summary> 
        /// ContentTemplateProperty property changed handler.
        /// </summary> 
        /// <param name="d">ContentPresenter that changed its ContentTemplate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnContentTemplatePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            ContentPresenter source = d as ContentPresenter;
            Debug.Assert(source != null, 
                "The source is not an instance of ContentPresenter!"); 

            // Use the Content as the DataContext to enable bindings in 
            // ContentTemplate or clear it if we removed our template (NOTE:
            // this should use ClearValue instead when it's available).
            source.DataContext = e.NewValue != null ? source.Content : null; 

            // Display the Content
            source.PrepareContentPresenter(); 
        } 
        #endregion ContentTemplate
#if false
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
                typeof(ContentPresenter), 
                null);
        #endregion TextAlignment
 
        #region TextDecorations
        /// <summary>
        /// Gets or sets a TextDecorationCollection that contains the effects to 
        /// apply to the Content text. 
        /// </summary>
//        [TypeConverter(typeof(TextDecorationCollectionConverter))] 
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
                typeof(ContentPresenter),
                null); 
        #endregion TextDecorations 

        #region TextWrapping 
        /// <summary>
        /// Gets or sets a value that indicates how the TextBlock should wrap
        /// text. 
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
                typeof(ContentPresenter), 
                null);
        #endregion TextWrapping
#endif
        /// <summary>
        /// Initializes a new instance of the ContentPresenter class.
        /// </summary> 
        public ContentPresenter() 
        {
            // Apply the default Template to ContentPresenter. 
//            Template = XamlReader.Load(ContentPresenterDefaultTemplate) as ControlTemplate;
//            ApplyTemplate();
            Debug.Assert(_alreadyAppliedTemplate, 
                "The write-once Template flag _alreadyAppliedTemplate was not set!");
        }
#if false
        /// <summary> 
        /// Apply the default Template to ContentPresenter.  This is only
        /// allowed to happen one time. 
        /// </summary>
        public override void OnApplyTemplate()
        { 
            // Ensure that the Template property is set/applied only once (from
            // our constructor).
            if (_alreadyAppliedTemplate) 
            { 
                throw new InvalidOperationException(Resource.ContentPresenter_OnApplyTemplate_WriteOnce);
            } 
            _alreadyAppliedTemplate = true;

            // Get the content root and the text content visual 
            _elementRoot = GetTemplateChild(RootElementName) as Grid;
            Debug.Assert(_elementRoot != null,
                "The required template part RootElement was not found!"); 
 
            _elementText = GetTemplateChild(ElementTextContentName) as TextBlock;
            Debug.Assert(_elementText != null, 
                "The required template part TextElement is not an instance of TextBlock!");

            // Ensure the template parts were structured correctly 
            Debug.Assert(_elementContent == null,
                "_elementContent should not exist when applying the template!");
            Debug.Assert(_elementRoot.Children.Contains(_elementText), 
                "TextElement should be a child of RootElement!"); 
            Debug.Assert(_elementRoot.Children.Count == 1,
                "RootElement should have TextElement as its only child!"); 
        }
#endif
        /// <summary> 
        /// Update the ContentPresenter's logical tree with the appropriate
        /// visual elements when its Content is changed.
        /// </summary> 
        private void PrepareContentPresenter() 
        {
            Debug.Assert(_elementRoot != null, "RootElement is null!"); 
            Debug.Assert(_elementText != null, "TextElement is null!");

            // Remove the old content 
            if (_elementContent != _elementText)
            {
                _elementRoot.Children.Remove(_elementContent); 
                _elementContent = null; 
            }
            else 
            {
                _elementText.Text = null;
            } 

            // Expand the ContentTemplate if it exists
            DataTemplate template = ContentTemplate; 
            object content = (template != null) ? 
                template.LoadContent() :
                Content; 

            // Add the new content
            UIElement element = content as UIElement; 
            if (element != null)
            {
                _elementContent = element; 
                _elementRoot.Children.Add(_elementContent); 
                _elementText.Visibility = Visibility.Collapsed;
            } 
            else if (content != null)
            {
                _elementText.Visibility = Visibility.Visible; 
//
                _elementText.Text = content.ToString();
            } 
            else 
            {
                _elementText.Visibility = Visibility.Collapsed; 
            }
        }
#if false
        /// <summary>
        /// Returns the specified contentPresenter element.
        /// </summary> 
        /// <param name="index">The index of the desired contentPresenter.</param> 
        /// <returns>The contentPresenter element at the specified index value.</returns>
        protected virtual DependencyObject GetVisualChild(int index) 
        {
            if (index != 0)
            { 
                throw new ArgumentOutOfRangeException("index");
            }
 
            object content = Content; 
            return (content == null) ?
                null : 
                ((content is UIElement) ?
                    _elementContent :
                    _elementText); 
        }
#endif
    }
} 
