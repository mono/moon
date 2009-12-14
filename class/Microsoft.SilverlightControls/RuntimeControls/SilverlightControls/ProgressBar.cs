// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


using System; 
using System.Windows; 
using System.Windows.Input;
using System.Windows.Controls; 
using System.Windows.Controls.Primitives;
using System.Windows.Automation.Peers;
 
namespace System.Windows.Controls
{
    /// <summary> 
    /// The ProgressBar class 
    /// </summary>
    /// <seealso cref="RangeBase" /> 
    [TemplatePart(Name = ProgressBar.ElementTrackName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = ProgressBar.ElementIndicatorName, Type = typeof(FrameworkElement))]
    [TemplateVisualState(Name = ProgressBar.StateDeterminate, GroupName = ProgressBar.GroupCommon)] 
    [TemplateVisualState(Name = ProgressBar.StateIndeterminate, GroupName = ProgressBar.GroupCommon)]
    public class ProgressBar : RangeBase
    { 
        #region Constructor 

        /// <summary> 
        /// Instantiates a new instance of Progressbar.
        /// </summary>
        public ProgressBar() 
            : base()
        {
            DefaultStyleKey = typeof(ProgressBar); 
        } 

        #endregion 

        #region IsIndeterminate
 
        /// <summary>
        /// The DependencyProperty for the IsIndeterminate property.
        /// </summary> 
        public static readonly DependencyProperty IsIndeterminateProperty = 
                DependencyProperty.RegisterCore(
                        "IsIndeterminate", 
                        typeof(bool),
                        typeof(ProgressBar),
                        new PropertyMetadata(OnIsIndeterminateChanged)); 

        /// <summary>
        /// Determines if ProgressBar shows actual values (false) 
        /// or generic, continuous progress feedback (true). 
        /// </summary>
        /// <value></value> 
        public bool IsIndeterminate
        {
            get { return (bool)GetValue(IsIndeterminateProperty); } 
            set { SetValue(IsIndeterminateProperty, value); }
        }
 
        /// <summary> 
        /// Called when IsIndeterminateProperty is changed on "d".
        /// </summary> 
        private static void OnIsIndeterminateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ProgressBar progressBar = (ProgressBar)d; 
            if (progressBar != null)
            {
                progressBar.OnIsIndeterminateChanged(); 
                progressBar.SetProgressBarIndicatorLength(); 
            }
        } 

        #endregion Properties
 
        #region Event Handler

        private void OnTrackSizeChanged(object sender, SizeChangedEventArgs e) 
        { 
            SetProgressBarIndicatorLength();
        } 

        #endregion
 
        #region Method Overrides

        /// <summary> 
        ///     This method is invoked when the Value property changes. 
        ///     ProgressBar updates its style parts when Value changes.
        /// </summary> 
        /// <param name="oldValue">The old value of the Value property.</param>
        /// <param name="newValue">The new value of the Value property.</param>
        protected override void OnValueChanged(double oldValue, double newValue) 
        {
            base.OnValueChanged(oldValue, newValue);
            SetProgressBarIndicatorLength(); 
        } 

        /// <summary> 
        /// Called when the Template's tree has been generated
        /// </summary>
        public override void OnApplyTemplate() 
        {
            base.OnApplyTemplate();
 
            if (ElementTrack != null) 
            {
                ElementTrack.SizeChanged -= OnTrackSizeChanged; 
            }

            ElementTrack = GetTemplateChild(ElementTrackName) as FrameworkElement; 
            ElementIndicator = GetTemplateChild(ElementIndicatorName) as FrameworkElement;

            if (ElementTrack != null) 
            { 
                ElementTrack.SizeChanged += OnTrackSizeChanged;
            } 

            // Sync the logical and visual states of the control
            UpdateVisualState(false); 
        }

        protected override AutomationPeer OnCreateAutomationPeer ()
        {
            return new ProgressBarAutomationPeer (this);
        }
        #endregion
 
        #region Private Methods 

        /// <summary> 
        /// Updates the width of the indicator based on the current value, min and max.
        /// </summary>
        private void SetProgressBarIndicatorLength() 
        {
            double min = Minimum;
            double max = Maximum; 
            double val = Value; 

 
            if (ElementTrack != null && ElementIndicator != null)
            {
                // When IsIndeterminate or max == min, have the indicator stretch the 
                // whole length of track, otherwise, calculate it.

                FrameworkElement parent = System.Windows.Media.VisualTreeHelper.GetParent(ElementIndicator) as FrameworkElement; 
 
                if (parent != null)
                { 
                    double extraSpace = ElementIndicator.Margin.Left + ElementIndicator.Margin.Right;

                    // There is no polymorphic way to get everything with Padding, so we have to check this way. 

                    Border border = parent as Border;
                    if (border != null) 
                    { 
                        extraSpace += border.Padding.Left + border.Padding.Right;
                    } 
                    else
                    {
                        Control control = parent as Control; 
                        if (control != null)
                        {
                            extraSpace += control.Padding.Left + control.Padding.Right; 
                        } 
                    }
 
                    double percent = IsIndeterminate || max == min ? 1.0 : (val - min) / (max - min);
                    double parentWidth = Math.Max(0, parent.ActualWidth - extraSpace);
                    ElementIndicator.Width = percent * parentWidth; 
                }
            }
        } 
 
        /// <summary>
        /// Responds to changes to IsIndeterminate by updating the visual state. 
        /// </summary>
        private void OnIsIndeterminateChanged()
        { 
            UpdateVisualState(true);
        }
 
        #endregion 

        #region Data 

        /// <summary>
        /// Reference to the Track child. 
        /// </summary>
        internal FrameworkElement ElementTrack { get; set; }
        internal const string ElementTrackName = "ProgressBarTrack"; 
 
        /// <summary>
        /// Reference to the Indicator child. 
        /// </summary>
        internal FrameworkElement ElementIndicator { get; set; }
        internal const string ElementIndicatorName = "ProgressBarIndicator"; 

        /// <summary>
        /// Transition into the Normal state in the ProgressBar template. 
        /// </summary> 
        internal const string StateDeterminate = "Determinate";
 
        /// <summary>
        /// Transition into Disabled state in the ProgressBar template.
        /// </summary> 
        internal const string StateIndeterminate = "Indeterminate";

        /// <summary> 
        /// Interaction state group of the ProgressBar. 
        /// </summary>
        internal const string GroupCommon = "CommonStates"; 

        #endregion
 
        #region Visual States
        /// <summary>
        /// Update the current visual state of the ProgressBar. 
        /// </summary> 
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to 
        /// snap directly to the new visual state.
        /// </param>
        internal void UpdateVisualState(bool useTransitions) 
        {
            if (!IsIndeterminate)
            { 
                GoToState(useTransitions, StateDeterminate); 
            }
            else 
            {
                GoToState(useTransitions, StateIndeterminate);
            } 
        }
        #endregion
    } 
} 
