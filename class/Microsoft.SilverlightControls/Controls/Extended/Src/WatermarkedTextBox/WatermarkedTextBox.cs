// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

namespace System.Windows.Controls
{ 
    using System.Diagnostics; 
    using System.Globalization;
    using System.IO; 
    using System.Windows;
    using System.Windows.Input;
    using System.Windows.Markup; 
    using System.Windows.Media.Animation;

    /// <summary> 
    /// WatermarkedTextBox is a specialized form of TextBox which displays custom visuals when its contents are empty 
    /// </summary>
    [TemplatePart(Name = WatermarkedTextBox.ElementRootName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = WatermarkedTextBox.ElementContentName, Type = typeof(ContentControl))]
    [TemplatePart(Name = WatermarkedTextBox.StateNormalName, Type = typeof(Storyboard))]
    [TemplatePart(Name = WatermarkedTextBox.StateMouseOverName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = WatermarkedTextBox.StateNormalWatermarkedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = WatermarkedTextBox.StateMouseOverWatermarkedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = WatermarkedTextBox.StateDisabledName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = WatermarkedTextBox.StateDisabledWatermarkedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = WatermarkedTextBox.StateFocusedName, Type = typeof(Storyboard))]
    public partial class WatermarkedTextBox : TextBox 
    {
        #region Constants
        private const string ElementRootName = "RootElement"; 
        private const string ElementContentName = "WatermarkElement";
        private const string StateNormalName = "Normal State";
        private const string StateNormalWatermarkedName = "Normal Watermarked State"; 
        private const string StateMouseOverName = "MouseOver State"; 
        private const string StateMouseOverWatermarkedName = "MouseOver Watermarked State";
        private const string StateDisabledName = "Disabled State"; 
        private const string StateDisabledWatermarkedName = "Disabled Watermarked State";
        private const string StateFocusedName = "Focused State";
 
        private const string TemplateXamlPath = "System.Windows.Controls.WatermarkedTextBox.WatermarkedTextBox.xaml";

 
        #endregion 

        #region Constructor 
        /// <summary>
        /// Initializes a new instance of the <see cref="WatermarkedTextBox"/> class.
        /// </summary> 
        public WatermarkedTextBox()
        {
            SetStyle(); 
            SetDefaults(); 

            this.MouseEnter += OnMouseEnter; 
            this.MouseLeave += OnMouseLeave;
            this.Loaded += OnLoaded;
            this.LostFocus += OnLostFocus; 
            this.GotFocus += OnGotFocus;
            this.TextChanged += OnTextChanged;
        } 
        #endregion 

        #region Internal 

        internal FrameworkElement elementRoot;
        internal ContentControl elementContent; 
        internal Storyboard stateNormalWatermarked;
        internal Storyboard stateNormal;
        internal Storyboard stateFocused; 
        internal Storyboard stateMouseOver; 
        internal Storyboard stateMouseOverWatermarked;
        internal Storyboard stateDisabled; 
        internal Storyboard stateDisabledWatermarked;
        internal Storyboard currentState;
        internal bool isHovered; 
        internal bool hasFocus;

        //this method is made 'internal virtual' so the a TestWatermarkedTextBox with custom verification code 
        //that executes in OnLoaded could be created 
        internal virtual void OnLoaded(object sender, RoutedEventArgs e)
        { 
            ApplyTemplate();
            ToggleState();
        } 

        internal void ToggleState()
        { 
            if (IsEnabled) 
            {
                if (!hasFocus && this.Watermark != null && string.IsNullOrEmpty(Text)) 
                {
                    if (isHovered)
                    { 
                        TryChangeState(stateMouseOverWatermarked ?? stateNormalWatermarked ?? stateNormal);
                    }
                    else 
                    { 
                        TryChangeState(stateNormalWatermarked ?? stateNormal);
                    } 
                }
                else
                { 
                    if (isHovered)
                    {
                        TryChangeState(stateMouseOver ?? stateNormal); 
                    } 
                    else if (hasFocus)
                    { 
                        TryChangeState(stateFocused ?? stateNormal);
                    }
                    else 
                    {
                        TryChangeState(stateNormal);
                    } 
                } 
            }
            else 
            {
                if (this.Watermark != null && string.IsNullOrEmpty(this.Text))
                { 
                    TryChangeState(stateDisabledWatermarked ?? stateDisabled ?? stateNormalWatermarked ?? stateNormal);
                }
                else 
                { 
                    TryChangeState(stateDisabled ?? stateNormal);
                } 
            }
        }
        #endregion 

        #region Protected
 
        /// <summary> 
        /// Called when template is applied to the control.
        /// </summary> 
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate(); 

            elementRoot = ExtractTemplatePart<FrameworkElement>(ElementRootName);
            elementContent = ExtractTemplatePart<ContentControl>(ElementContentName); 
 
            OnWatermarkChanged();
 
            if (elementRoot != null)
            {
                stateNormalWatermarked = ExtractTemplatePartResource<Storyboard>(elementRoot, StateNormalWatermarkedName); 
                stateNormal = ExtractTemplatePartResource<Storyboard>(elementRoot, StateNormalName);
                stateFocused = ExtractTemplatePartResource<Storyboard>(elementRoot, StateFocusedName);
                stateMouseOver = ExtractTemplatePartResource<Storyboard>(elementRoot, StateMouseOverName); 
                stateMouseOverWatermarked = ExtractTemplatePartResource<Storyboard>(elementRoot, StateMouseOverWatermarkedName); 
                stateDisabled = ExtractTemplatePartResource<Storyboard>(elementRoot, StateDisabledName);
                stateDisabledWatermarked = ExtractTemplatePartResource<Storyboard>(elementRoot, StateDisabledWatermarkedName); 
            }

 

            ToggleState();
        } 
 
        #endregion
 
        #region Public

        #region IsEnabled 


        /// <summary> 
        /// IsEnabled dependency property 
        /// </summary>
        public static readonly DependencyProperty IsEnabledProperty = DependencyProperty.Register( 
            "IsEnabled", typeof(bool), typeof(WatermarkedTextBox), new PropertyMetadata(OnIsEnabledPropertyChanged));

 
        /// <summary>
        /// Gets or sets a value indicating whether this element is enabled in the user interface (UI).  This is a dependency property.
        /// </summary> 
        /// <value></value> 
        /// <returns>true if the element is enabled; otherwise, false. The default value is true.</returns>
        public bool IsEnabled 
        {
            get { return (bool)GetValue(IsEnabledProperty); }
            set { SetValue(IsEnabledProperty, value); } 
        }

        #endregion 
 
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
 
        #region Watermark
        /// <summary>
        /// Watermark dependency property 
        /// </summary> 
        public static readonly DependencyProperty WatermarkProperty = DependencyProperty.Register(
            "Watermark", typeof(object), typeof(WatermarkedTextBox), new PropertyMetadata(OnWatermarkPropertyChanged)); 

        /// <summary>
        /// Watermark content 
        /// </summary>
        /// <value>The watermark.</value>
        public object Watermark 
        { 
            get { return (object)GetValue(WatermarkProperty); }
            set { SetValue(WatermarkProperty, value); } 
        }

        #endregion 

        #endregion
 
        #region Private 

        private static string styleXaml; 

        private static T ExtractTemplatePartResource<T>(FrameworkElement root, string resourceName) where T : DependencyObject
        { 
            DependencyObject obj = (DependencyObject)root.Resources[resourceName];
            return ExtractTemplatePart<T>(resourceName, obj);
        } 
 

        private T ExtractTemplatePart<T>(string partName) where T : DependencyObject 
        {
            DependencyObject obj = GetTemplateChild(partName);
            return ExtractTemplatePart<T>(partName, obj); 
        }

 
        private static T ExtractTemplatePart<T>(string partName, DependencyObject obj) where T : DependencyObject 
        {
            Debug.Assert(obj == null || typeof(T).IsInstanceOfType(obj), 
             string.Format(CultureInfo.InvariantCulture, Resource.WatermarkedTextBox_TemplatePartIsOfIncorrectType, partName, typeof(T).Name));
            return obj as T;
        } 


 
        private void OnGotFocus(object sender, RoutedEventArgs e) 
        {
            if (IsEnabled) 
            {
                hasFocus = true;
 
                if (!string.IsNullOrEmpty(this.Text))
                {
                    Select(0, this.Text.Length); 
                } 

                ToggleState(); 
            }
        }
 

        /// <summary>
        /// Called when is enabled property is changed. 
        /// </summary> 
        /// <param name="sender">The sender.</param>
        /// <param name="args">The <see cref="System.Windows.DependencyPropertyChangedEventArgs"/> instance containing the event data.</param> 
        private static void OnIsEnabledPropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args)
        {
            WatermarkedTextBox watermarkedTextBox = sender as WatermarkedTextBox; 
            Debug.Assert(watermarkedTextBox != null, "The source is not an instance of a WatermarkedTextBox!");
            bool newValue = (bool)args.NewValue;
 
            //MIX-only solution, as IsEnabled is not defined on Control level 
            watermarkedTextBox.IsHitTestVisible = newValue;
            watermarkedTextBox.IsTabStop = newValue; 
            watermarkedTextBox.IsReadOnly = !newValue;

            watermarkedTextBox.ToggleState(); 
        }

 
        private void OnLostFocus(object sender, RoutedEventArgs e) 
        {
            hasFocus = false; 
            ToggleState();
        }
 

        private void OnMouseEnter(object sender, MouseEventArgs e)
        { 
            isHovered = true; 

            if (!hasFocus) 
            {
                ToggleState();
            } 
        }

        private void OnMouseLeave(object sender, MouseEventArgs e) 
        { 
            isHovered = false;
 
            if (!hasFocus)
            {
                ToggleState(); 
            }
        }
 
 
        private void OnTextChanged(object sender, TextChangedEventArgs e)
        { 
            ToggleState();
        }
 

        private void OnWatermarkChanged()
        { 
            if (elementContent != null) 
            {
                Control watermarkControl = this.Watermark as Control; 
                if (watermarkControl != null)
                {
                    watermarkControl.IsTabStop = false; 
                    watermarkControl.IsHitTestVisible = false;
                }
            } 
        } 

        /// <summary> 
        /// Called when watermark property is changed.
        /// </summary>
        /// <param name="sender">The sender.</param> 
        /// <param name="args">The <see cref="System.Windows.DependencyPropertyChangedEventArgs"/> instance containing the event data.</param>
        private static void OnWatermarkPropertyChanged(DependencyObject sender, DependencyPropertyChangedEventArgs args)
        { 
            WatermarkedTextBox watermarkTextBox = sender as WatermarkedTextBox; 
            Debug.Assert(watermarkTextBox != null, "The source is not an instance of a WatermarkedTextBox!");
            watermarkTextBox.OnWatermarkChanged(); 
            watermarkTextBox.ToggleState();

        } 

        private void SetDefaults()
        { 
            IsEnabled = true; 
            this.Watermark = Resource.WatermarkedTextBox_DefaultWatermarkText;
        } 

        private void SetStyle()
        { 
            if (styleXaml == null)
            {
                Stream stream = typeof(WatermarkedTextBox).Assembly.GetManifestResourceStream(TemplateXamlPath); 
                Debug.Assert(stream != null, string.Format(CultureInfo.InvariantCulture, "XAML resource '{0}' is not found!", TemplateXamlPath)); 
                using (StreamReader reader = new StreamReader(stream))
                { 
                    styleXaml = reader.ReadToEnd();
                }
            } 
            Style style = XamlReader.Load(styleXaml) as Style;
            this.Style = style;
        } 
 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1031:DoNotCatchGeneralExceptionTypes",
            Justification="The control with throw Exception if transition occurs while it is not in the tree." )] 
        private void TryChangeState(Storyboard newState)
        {
            if (currentState == newState) 
            {
                return;
            } 
            if (newState != null) 
            {
                if (Parent != null) 
                {
                    try
                    { 
                        if (currentState != null)
                        {
                            currentState.Stop(); 
                        } 
                        newState.Begin();
                    } 
                    //the control with throw Exception if transition occurs while it is not in the tree
                    catch
                    { 
                    }
                }
                currentState = newState; 
            } 
        }
 
        #endregion
    }
} 

