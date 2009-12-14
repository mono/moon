// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Automation.Peers;
using System.Windows.Input;

namespace System.Windows.Controls.Primitives
{
    /// <summary>
    /// Represents a button control used in Calendar Control, which reacts to the Click event.
    /// </summary>
    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StatePressed, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateUnselected, GroupName = VisualStates.GroupSelection)]
    [TemplateVisualState(Name = VisualStates.StateSelected, GroupName = VisualStates.GroupSelection)]
    [TemplateVisualState(Name = VisualStates.StateCalendarButtonUnfocused, GroupName = VisualStates.GroupCalendarButtonFocus)]
    [TemplateVisualState(Name = VisualStates.StateCalendarButtonFocused, GroupName = VisualStates.GroupCalendarButtonFocus)]
    [TemplateVisualState(Name = VisualStates.StateInactive, GroupName = VisualStates.GroupActive)]
    [TemplateVisualState(Name = VisualStates.StateActive, GroupName = VisualStates.GroupActive)]
    public sealed class CalendarButton : Button
    {
        #region Data

        private bool _isInactive;
        private bool _isFocusedOverride;
        private bool _isSelected;

        #endregion Data

        #region Events

        /// <summary>
        /// Occurs when MouseLeftButton is down.
        /// </summary>
        public event MouseButtonEventHandler CalendarButtonMouseDown;

        /// <summary>
        /// Occurs when MouseLeftButton is up.
        /// </summary>
        public event MouseButtonEventHandler CalendarButtonMouseUp;

        #endregion Events

        /// <summary>
        /// Represents the CalendarButton that is used in Calendar Control.
        /// </summary>
        public CalendarButton()
            : base()
        {
            // Attach the necessary events to their virtual counterparts
            Loaded += delegate { ChangeVisualState(false); };
            this.IsTabStop = false;
            DefaultStyleKey = typeof(CalendarButton);
            Content = DateTimeHelper.GetCurrentDateFormat().AbbreviatedMonthNames[0];
        }

        #region Internal Properties

        internal bool IsInactive
        {
            get
            {
                return this._isInactive;
            }
            set
            {
                this._isInactive = value;
                ChangeVisualState(true);
            }
        }

        internal bool IsFocusedOverride
        {
            get
            {
                return this._isFocusedOverride;
            }
            set
            {
                this._isFocusedOverride = value;
                ChangeVisualState(true);
            }
        }

        internal bool IsSelected
        {
            get
            {
                return this._isSelected;
            }
            set
            {
                this._isSelected = value;
                ChangeVisualState(true);
            }
        }

        internal Calendar Owner
        {
            get;
            set;
        }

        #endregion Internal Properties

        #region Public Methods

        /// <summary>
        /// Apply a template to the button.
        /// </summary>
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            // Sync the logical and visual states of the control
            ChangeVisualState(false);
        }

        #endregion Public Methods

        #region Protected Methods

        /// <summary>
        /// Creates the automation peer for the DayButton.
        /// </summary>
        /// <returns></returns>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new CalendarButtonAutomationPeer(this);
        }

        /// <summary>
        /// Overrides the OnMouseLeftButtonDown method of Button in order 
        /// to be able to get MouseLeftButtonDown events
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            base.OnMouseLeftButtonDown(e);
            OnCalendarButtonMouseDown(e);
        }

        /// <summary>
        /// Overrides the OnMouseLeftButtonUp method of Button in order 
        /// to be able to get MouseLeftButtonUp events
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
        {
            base.OnMouseLeftButtonUp(e);
            OnCalendarButtonMouseUp(e);
        }

        #endregion Protected Methods

        #region Internal Methods

        /// <summary>
        /// Change to the correct visual state for the button.
        /// </summary>
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state.
        /// </param>
        internal void ChangeVisualState(bool useTransitions)
        {
            // Update the SelectionStates group
            if (_isSelected)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateSelected, VisualStates.StateUnselected);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateUnselected);
            }

            // Update the ActiveStates group
            if (IsInactive)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateInactive);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateActive, VisualStates.StateInactive);
            }

            // Update the FocusStates group
            if (IsFocusedOverride && IsEnabled)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateCalendarButtonFocused, VisualStates.StateCalendarButtonUnfocused);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateCalendarButtonUnfocused);
            }

        }

        internal void OnCalendarButtonMouseDown(MouseButtonEventArgs e)
        {
            MouseButtonEventHandler handler = this.CalendarButtonMouseDown;
            if (null != handler)
            {
                handler(this, e);
            }
        }

        internal void OnCalendarButtonMouseUp(MouseButtonEventArgs e)
        {
            MouseButtonEventHandler handler = this.CalendarButtonMouseUp;

            if (null != handler)
            {
                handler(this, e);
            }
        }

        //We need to send a MouseUp event for the CalendarButton 
        //that stays in Pressed state after MouseCapture is released since there is no actual MouseUpEvent for the release
        internal void SendMouseUpEvent(MouseButtonEventArgs e)
        {
            e.Handled = false;
            base.OnMouseLeftButtonUp(e);
        }

        #endregion Internal Methods
    }
}
