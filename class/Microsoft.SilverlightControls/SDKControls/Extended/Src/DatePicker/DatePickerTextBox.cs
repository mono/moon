// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Windows.Input;
using System.Windows.Markup;

namespace System.Windows.Controls.Primitives
{
    /// <summary>
    /// DatePickerTextBox is a specialized form of TextBox which displays custom visuals when its contents are empty
    /// </summary>
    [TemplatePart(Name = DatePickerTextBox.ElementContentName, Type = typeof(ContentControl))]
    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateUnfocused, GroupName = VisualStates.GroupFocus)]
    [TemplateVisualState(Name = VisualStates.StateFocused, GroupName = VisualStates.GroupFocus)]
    [TemplateVisualState(Name = VisualStates.StateUnwatermarked, GroupName = VisualStates.GroupWatermark)]
    [TemplateVisualState(Name = VisualStates.StateWatermarked, GroupName = VisualStates.GroupWatermark)]
    public sealed partial class DatePickerTextBox : TextBox
    {
        #region Constants

        private const string ElementContentName = "Watermark";

        #endregion

        #region Constructor
        /// <summary>
        /// Initializes a new instance of the <see cref="DatePickerTextBox"/> class.
        /// </summary>
        public DatePickerTextBox()
        {
            DefaultStyleKey = typeof(DatePickerTextBox);
            SetDefaults();

            this.MouseEnter += OnMouseEnter;
            this.MouseLeave += OnMouseLeave;
            this.Loaded += OnLoaded;
            this.LostFocus += OnLostFocus;
            this.GotFocus += OnGotFocus;
            this.TextChanged += OnTextChanged;
            this.IsEnabledChanged += new DependencyPropertyChangedEventHandler(OnIsEnabledChanged);
        }
        #endregion

        #region Internal

        internal ContentControl elementContent;
        internal bool isHovered;
        internal bool hasFocus;

        internal void OnLoaded(object sender, RoutedEventArgs e)
        {
            ApplyTemplate();
            ChangeVisualState(false);
        }

        /// <summary>
        /// Change to the correct visual state for the textbox.
        /// </summary>
        internal void ChangeVisualState()
        {
            ChangeVisualState(true);
        }

        /// <summary>
        /// Change to the correct visual state for the textbox.
        /// </summary>
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state.
        /// </param>
        internal void ChangeVisualState(bool useTransitions)
        {
            // Update the CommonStates group
            if (!IsEnabled)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateDisabled, VisualStates.StateNormal);
            }
            else if (isHovered)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateMouseOver, VisualStates.StateNormal);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateNormal);
            }

            // Update the FocusStates group
            if (hasFocus && IsEnabled)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateFocused, VisualStates.StateUnfocused);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateUnfocused);
            }

            // Update the WatermarkStates group
            if (this.Watermark != null && string.IsNullOrEmpty(this.Text))
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateWatermarked, VisualStates.StateUnwatermarked);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateUnwatermarked);
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

            elementContent = ExtractTemplatePart<ContentControl>(ElementContentName);

            OnWatermarkChanged();

            ChangeVisualState(false);
        }

        #endregion

        #region Public

        #region Watermark
        /// <summary>
        /// Watermark dependency property
        /// </summary>
        public static readonly DependencyProperty WatermarkProperty = DependencyProperty.Register(
            "Watermark", typeof(object), typeof(DatePickerTextBox), new PropertyMetadata(OnWatermarkPropertyChanged));

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



        private T ExtractTemplatePart<T>(string partName) where T : DependencyObject
        {
            DependencyObject obj = GetTemplateChild(partName);
            return ExtractTemplatePart<T>(partName, obj);
        }


        private static T ExtractTemplatePart<T>(string partName, DependencyObject obj) where T : DependencyObject
        {
            Debug.Assert(obj == null || typeof(T).IsInstanceOfType(obj),
             string.Format(CultureInfo.InvariantCulture, Resource.DatePickerTextBox_TemplatePartIsOfIncorrectType, partName, typeof(T).Name));
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

                ChangeVisualState();
            }
        }

        /// <summary>
        /// Called when the IsEnabled property changes.
        /// </summary>
        /// <param name="sender">Sender object</param>
        /// <param name="e">Property changed args</param>
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            Debug.Assert(e.NewValue is bool);
            bool isEnabled = (bool)e.NewValue;

            IsReadOnly = !isEnabled;
            if (!isEnabled)
            {
                isHovered = false;
            }

            ChangeVisualState();
        }

        private void OnLostFocus(object sender, RoutedEventArgs e)
        {
            hasFocus = false;
            ChangeVisualState();
        }


        private void OnMouseEnter(object sender, MouseEventArgs e)
        {
            isHovered = true;

            if (!hasFocus)
            {
                ChangeVisualState();
            }
        }

        private void OnMouseLeave(object sender, MouseEventArgs e)
        {
            isHovered = false;

            if (!hasFocus)
            {
                ChangeVisualState();
            }
        }


        private void OnTextChanged(object sender, TextChangedEventArgs e)
        {
            ChangeVisualState();
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
            DatePickerTextBox datePickerTextBox = sender as DatePickerTextBox;
            Debug.Assert(datePickerTextBox != null, "The source is not an instance of a DatePickerTextBox!");
            datePickerTextBox.OnWatermarkChanged();
            datePickerTextBox.ChangeVisualState();

        }

        private void SetDefaults()
        {
            IsEnabled = true;
            this.Watermark = Resource.DatePickerTextBox_DefaultWatermarkText;
        }

        #endregion
    }
}

