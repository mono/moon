// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 


using System; 
using System.Diagnostics; 
using System.Globalization;
using System.Windows; 
using System.Windows.Controls;
using System.Windows.Automation;
using System.Windows.Automation.Peers;
 
namespace System.Windows.Controls.Primitives
{
    /// <summary> 
    /// Represents the base class for elements that have a specific range. 
    /// Examples of such elements are ScrollBar, ProgressBar and Slider.  This
    /// class defines the relevant events and properties, and provides handlers 
    /// for the events.
    /// </summary>
    public abstract class RangeBase : Control 
    {
        #region CoercionWorkaroundHelpers
        /// This section contains static helper variables used to keep track 
        /// of which variable values have been updated/coerced, and when to 
        /// call the corresponding property changed methods.
        internal int _levelsFromRootCall; 
        internal double _initialMax;
        internal double _initialVal;
        internal double _requestedMax; 
        internal double _requestedVal;
        #endregion CoercionWorkaroundHelpers
 
        #region Minimum 
        /// <summary>
        /// Gets or sets the Minimum possible Value of the range element.  The 
        /// default value is zero.
        /// </summary>
        public double Minimum 
        {
            get { return (double)GetValue(MinimumProperty); }
            set { SetValue(MinimumProperty, value); } 
        } 

        /// <summary> 
        /// Identifies the Minimum dependency property.
        /// </summary>
        public static readonly DependencyProperty MinimumProperty = 
            DependencyProperty.RegisterCore(
                "Minimum",
                typeof(double), 
                typeof(RangeBase), 
                new PropertyMetadata(0.0d, OnMinimumPropertyChanged));
 
        /// <summary>
        /// MinimumProperty property changed handler.
        /// </summary> 
        /// <param name="d">RangeBase that changed its Minimum.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        /// 
        private static void OnMinimumPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            RangeBase range = d as RangeBase; 
            Debug.Assert(range != null);

            // Ensure it's a valid value 
            if (!IsValidDoubleValue(e.NewValue))
            {
                throw new ArgumentException(); 
            } 

            // Note: this section is a workaround, containing my 
            // logic to hold all calls to the property changed
            // methods until after all coercion has completed
            // ---------- 
            if (range._levelsFromRootCall == 0)
            {
                range._initialMax = range.Maximum; 
                range._initialVal = range.Value; 
            }
            range._levelsFromRootCall++; 
            // ----------

            range.CoerceMaximum(); 
            range.CoerceValue();

            // Note: this section completes my workaround to call 
            // the property changed logic if all coercion has completed 
            // ----------
            range._levelsFromRootCall--; 
            if (range._levelsFromRootCall == 0)
            {
                range.OnMinimumChanged((double)e.OldValue, (double)e.NewValue); 
		// UIA event
		range.RaiseChangeEvent (Change.Minimum, (double)e.OldValue, (double)e.NewValue);
                double maximum = range.Maximum;
                if (range._initialMax != maximum)
                { 
                    range.OnMaximumChanged(range._initialMax, maximum); 
		    // UIA event
		    range.RaiseChangeEvent(Change.Maximum, range._initialMax, maximum);
                }
                double value = range.Value; 
                if (range._initialVal != value)
                {
                    range.OnValueChanged(range._initialVal, value); 
                }
            }
            // ---------- 
        } 
        #endregion Minimum
 
        #region Maximum
        /// <summary>
        /// Gets or sets the Maximum possible Value of the range element. 
        /// The default values is one.
        /// </summary>
        public double Maximum 
        { 
            get { return (double)GetValue(MaximumProperty); }
            set { SetValue(MaximumProperty, value); } 
        }

        /// <summary> 
        /// Identifies the Maximum dependency property.
        /// </summary>
        public static readonly DependencyProperty MaximumProperty = 
            DependencyProperty.RegisterCore( 
                "Maximum",
                typeof(double), 
                typeof(RangeBase),
                new PropertyMetadata(1.0d, OnMaximumPropertyChanged));
 
        /// <summary>
        /// MaximumProperty property changed handler.
        /// </summary> 
        /// <param name="d">RangeBase that changed its Maximum.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnMaximumPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            RangeBase range = d as RangeBase;
            Debug.Assert(range != null); 

            // Ensure it's a valid value
            if (!IsValidDoubleValue(e.NewValue)) 
            { 
                throw new ArgumentException();
            } 

            // Note: this section is a workaround, containing my
            // logic to hold all calls to the property changed 
            // methods until after all coercion has completed
            // ----------
            if (range._levelsFromRootCall == 0) 
            { 
                range._requestedMax = (double)e.NewValue;
                range._initialMax = (double)e.OldValue; 
                range._initialVal = range.Value;
            }
            range._levelsFromRootCall++; 
            // ----------

            range.CoerceMaximum(); 
            range.CoerceValue(); 

            // Note: this section completes my workaround to call 
            // the property changed logic if all coercion has completed
            // ----------
            range._levelsFromRootCall--; 
            if (range._levelsFromRootCall == 0)
            {
                double maximum = range.Maximum; 
                if (range._initialMax != maximum) 
                {
                    range.OnMaximumChanged(range._initialMax, maximum);
		
		    // UIA event
		    range.RaiseChangeEvent(Change.Maximum, range._initialMax, maximum);
                }
                double value = range.Value;
                if (range._initialVal != value) 
                {
                    range.OnValueChanged(range._initialVal, value);
                } 
            } 
            // ----------
        } 
        #endregion Maximum

        #region LargeChange 
        /// <summary>
        /// Gets or sets a value to be added to or subtracted from the Value of
        /// a RangeBase control.  The default values is one. 
        /// </summary> 
        public double LargeChange
        { 
            get { return (double)GetValue(LargeChangeProperty); }
            set { SetValue(LargeChangeProperty, value); }
        } 

        /// <summary>
        /// Identifies the LargeChange dependency property. 
        /// </summary> 
        public static readonly DependencyProperty LargeChangeProperty =
            DependencyProperty.RegisterCore( 
                "LargeChange",
                typeof(double),
                typeof(RangeBase), 
                new PropertyMetadata(1.0d, OnLargeChangePropertyChanged));

        /// <summary> 
        /// LargeChangeProperty property changed handler. 
        /// </summary>
        /// <param name="d">RangeBase that changed its LargeChange.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnLargeChangePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            RangeBase range = d as RangeBase;
            Debug.Assert(range != null);
 
            // Ensure it's a valid value 
            if (!IsValidChange(e.NewValue))
            { 
                throw new ArgumentException();
            }

	    // UIA event
	    range.RaiseChangeEvent (Change.Large, (double) e.OldValue, (double) e.NewValue);
        } 
        #endregion LargeChange

        #region SmallChange 
        /// <summary> 
        /// Gets or sets a value to be added to or subtracted from the Value of
        /// a RangeBase control.  The default values is 0.1. 
        /// </summary>
        public double SmallChange
        { 
            get { return (double)GetValue(SmallChangeProperty); }
            set { SetValue(SmallChangeProperty, value); }
        } 
 
        /// <summary>
        /// Identifies the SmallChange dependency property. 
        /// </summary>
        public static readonly DependencyProperty SmallChangeProperty =
            DependencyProperty.RegisterCore( 
                "SmallChange",
                typeof(double),
                typeof(RangeBase), 
                new PropertyMetadata(0.1d, OnSmallChangePropertyChanged)); 

        /// <summary> 
        /// SmallChangeProperty property changed handler.
        /// </summary>
        /// <param name="d">RangeBase that changed its SmallChange.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnSmallChangePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            RangeBase range = d as RangeBase; 
            Debug.Assert(range != null);
 
            // Ensure it's a valid value
            if (!IsValidChange(e.NewValue))
            { 
                throw new ArgumentException();
            }

	    // UIA event
	    range.RaiseChangeEvent (Change.Small, (double) e.OldValue, (double) e.NewValue);
        } 
        #endregion SmallChange 

        #region Value 
        /// <summary>
        /// Gets or sets the current Value of the range element.  The default
        /// value is zero. 
        /// </summary>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1721:PropertyNamesShouldNotMatchGetMethods", Justification = "Compatibility with WPF")]
        public double Value 
        { 
            get { return (double)GetValue(ValueProperty); }
            set { SetValue(ValueProperty, value); } 
        }

        /// <summary> 
        /// Identifies the Value dependency property.
        /// </summary>
        public static readonly DependencyProperty ValueProperty = 
            DependencyProperty.RegisterCore( 
                "Value",
                typeof(double), 
                typeof(RangeBase),
                new PropertyMetadata(0.0d, OnValuePropertyChanged));
 
        /// <summary>
        /// ValueProperty property changed handler.
        /// </summary> 
        /// <param name="d">RangeBase that changed its Value.</param> 
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnValuePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            RangeBase range = d as RangeBase;
            Debug.Assert(range != null); 

            // Ensure it's a valid value
            if (!IsValidDoubleValue(e.NewValue)) 
            { 
                throw new ArgumentException();
            } 

            // Note: this section is a workaround, containing my
            // logic to hold all calls to the property changed 
            // methods until after all coercion has completed
            // ----------
            if (range._levelsFromRootCall == 0) 
            { 
                range._requestedVal = (double)e.NewValue;
                range._initialVal = (double)e.OldValue; 
            }
            range._levelsFromRootCall++;
            // ---------- 

            range.CoerceValue();
 
            // Note: this section completes my workaround to call 
            // the property changed logic if all coercion has completed
            // ---------- 
            range._levelsFromRootCall--;
            if (range._levelsFromRootCall == 0)
            { 
                double value = range.Value;
                if (range._initialVal != value)
                { 
                    range.OnValueChanged(range._initialVal, value); 
	
		    // Raises UIA event
		    if (range.AutomationPeer != null)
			    range.AutomationPeer.RaisePropertyChangedEvent (RangeValuePatternIdentifiers.ValueProperty, 
			                                                    e.OldValue, 
									    e.NewValue);
                }
            } 
            // ----------
        }
        #endregion Value 

        /// <summary>
        /// Occurs when the range value changes. 
        /// </summary> 
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Design", "CA1009:DeclareEventHandlersCorrectly", Justification = "Compat with WPF.")]
        public event RoutedPropertyChangedEventHandler<double> ValueChanged; 

        #region Protected Methods
        /// <summary> 
        /// Initializes a new instance of the RangeBase class.
        /// </summary>
        protected RangeBase() 
        { 
        }
 
        /// <summary>
        /// Called when the Maximum property changes.
        /// </summary> 
        /// <param name="oldMaximum">Old value of the Maximum property.</param>
        /// <param name="newMaximum">New value of the Maximum property.</param>
        protected virtual void OnMaximumChanged(double oldMaximum, double newMaximum) 
        { 
        }
 
        /// <summary>
        /// Called when the Minimum property changes.
        /// </summary> 
        /// <param name="oldMinimum">Old value of the Minimum property.</param>
        /// <param name="newMinimum">New value of the Minimum property.</param>
        protected virtual void OnMinimumChanged(double oldMinimum, double newMinimum) 
        { 
        }
 
        /// <summary>
        /// Called when the Value property changes.
        /// </summary> 
        /// <param name="oldValue">Old value of the Value property.</param>
        /// <param name="newValue">New value of the Value property.</param>
        protected virtual void OnValueChanged(double oldValue, double newValue) 
        { 
            RoutedPropertyChangedEventHandler<double> handler = ValueChanged;
            if (handler != null) 
            {
                handler(this, new RoutedPropertyChangedEventArgs<double>(oldValue, newValue));
            } 
        }
        #endregion
 
        /// <summary> 
        /// Ensure the Maximum is greater than or equal to the minimum.
        /// </summary> 
        private void CoerceMaximum()
        {
            double minimum = Minimum; 
            double maximum = Maximum;
            if (_requestedMax != maximum && _requestedMax >= minimum)
            { 
                SetValue(MaximumProperty, _requestedMax); 
            }
            else if (maximum < minimum) 
            {
                SetValue(MaximumProperty, minimum);
            } 
        }

        /// <summary> 
        /// Ensure the value falls between the Minimum and Maximum values. 
        /// This function assumes that (Maximum >= Minimum)
        /// </summary> 
        private void CoerceValue()
        {
            double minimum = Minimum; 
            double maximum = Maximum;
            double value = Value;
 
            if (_requestedVal != value && _requestedVal >= minimum && _requestedVal <= maximum) 
            {
                SetValue(ValueProperty, _requestedVal); 
            }
            else
            { 
                if (value < minimum)
                {
                    SetValue(ValueProperty, minimum); 
                } 
                if (value > maximum)
                { 
                    SetValue(ValueProperty, maximum);
                }
            } 
        }

        /// <summary> 
        /// Check if a value is a value double. 
        /// </summary>
        /// <param name="value">Value.</param> 
        /// <returns>true if a valid double; false otherwise.</returns>
        /// <remarks>
        /// This method is set to private, and is only expected to be 
        /// called from our property changed handlers.
        /// </remarks>
        private static bool IsValidDoubleValue(object value) 
        { 
            Debug.Assert(typeof(double).IsInstanceOfType(value));
            double number = (double)value; 
            return !double.IsNaN(number) && !double.IsInfinity(number);
        }
 
        /// <summary>
        /// Check if a value is a valid change for the two change properties.
        /// </summary> 
        /// <param name="value">Value.</param> 
        /// <returns>true if a valid value; false otherwise.</returns>
        private static bool IsValidChange(object value) 
        {
            return IsValidDoubleValue(value) && (((double)value) >= 0);
        } 

        /// <summary>
        /// Provides a string representation of a RangeBase object. 
        /// </summary> 
        /// <returns>
        /// Returns the string representation of a RangeBase object. 
        /// </returns>
        public override string ToString()
        { 
            return string.Format(CultureInfo.InvariantCulture, FormatString, base.ToString(), Minimum, Maximum, Value);
        }
 
        internal bool GoToState(bool useTransitions, string stateName) 
        {
            Debug.Assert(stateName != null); 
            return VisualStateManager.GoToState(this, stateName, useTransitions);
        }
 
        /// <summary>
        /// Format string for RangeBase
        /// </summary> 
        private const string FormatString = "{0} Minimum:{1} Maximum:{2} Value:{3}"; 

	#region UIA Events

	internal void RaiseChangeEvent (Change change, double oldValue, double newValue)
	{
		if (UIAPropertyChanged != null)
			UIAPropertyChanged (this,
			                    new ChangeEventArgs () { Change   = change,
					                             OldValue = oldValue,
								     NewValue = newValue });
	}

	internal event EventHandler<ChangeEventArgs> UIAPropertyChanged;

	internal class ChangeEventArgs : EventArgs {
		public Change Change { get; set; }
		public double OldValue { get; set; }
		public double NewValue { get; set; }
	}

	internal enum Change {
		Large,
		Small,
		Maximum,
		Minimum
	};

	#endregion
    }
} 
