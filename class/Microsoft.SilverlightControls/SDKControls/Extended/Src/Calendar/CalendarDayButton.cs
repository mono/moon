// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Globalization;
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
    [TemplateVisualState(Name = CalendarDayButton.StateRegularDay, GroupName = CalendarDayButton.GroupDay)]
    [TemplateVisualState(Name = CalendarDayButton.StateToday, GroupName = CalendarDayButton.GroupDay)]
    [TemplateVisualState(Name = CalendarDayButton.StateNormalDay, GroupName = CalendarDayButton.GroupBlackout)]
    [TemplateVisualState(Name = CalendarDayButton.StateBlackoutDay, GroupName = CalendarDayButton.GroupBlackout)]
    public sealed class CalendarDayButton : Button
    {
        #region Constants
        /// <summary>
        /// Default content for the CalendarDayButton
        /// </summary>
        private const int DEFAULTCONTENT = 1;

        /// <summary>
        /// Identifies the Today state.
        /// </summary>
        internal const string StateToday = "Today";

        /// <summary>
        /// Identifies the RegularDay state.
        /// </summary>
        internal const string StateRegularDay = "RegularDay";

        /// <summary>
        /// Name of the Day state group.
        /// </summary>
        internal const string GroupDay = "DayStates";

        /// <summary>
        /// Identifies the BlackoutDay state.
        /// </summary>
        internal const string StateBlackoutDay = "BlackoutDay";

        /// <summary>
        /// Identifies the NormalDay state.
        /// </summary>
        internal const string StateNormalDay = "NormalDay";

        /// <summary>
        /// Name of the BlackoutDay state group.
        /// </summary>
        internal const string GroupBlackout = "BlackoutDayStates";

        #endregion Constants

        #region Data

        private bool _isCurrent;
        private bool _isDisabled;
        private bool _isInactive;
        private bool _isMouseOverOverride;
        private bool _isSelected;

        private bool _isToday;

        #endregion Data

        #region Events

        /// <summary>
        /// Occurs when MouseLeftButton is down.
        /// </summary>
        public event MouseButtonEventHandler CalendarDayButtonMouseDown;

        /// <summary>
        /// Occurs when MouseLeftButton is up.
        /// </summary>
        public event MouseButtonEventHandler CalendarDayButtonMouseUp;

        #endregion Events

        /// <summary>
        /// Represents the CalendarDayButton that is used in Calendar Control.
        /// </summary>
        public CalendarDayButton()
            : base()
        {
            // Attach the necessary events to their virtual counterparts
            Loaded += delegate { ChangeVisualState(false); };
            this.IsTabStop = false;
            DefaultStyleKey = typeof(CalendarDayButton);
            Content = DEFAULTCONTENT.ToString(CultureInfo.CurrentCulture);
        }

        #region Internal Properties

        internal int Index
        {
            get;
            set;
        }

        internal bool IsDisabled
        {
            get
            {
                return this._isDisabled;
            }
            set
            {
                this._isDisabled = value;
                ChangeVisualState(true);
            }
        }

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

        // 
        internal bool IsMouseOverOverride
        {
            get
            {
                return this._isMouseOverOverride;
            }
            set
            {
                this._isMouseOverOverride = value;
                //When the DatePicker popUp is opened, none of the CalendarDayButtons should be in MouseOver state.
                //The CalendarDayButton that was selected before the PopUp was closed, stays in MouseOver state. We are
                //overriding this state.
                if (this.IsMouseOver == true)
                {
                    this._isMouseOverOverride = true;
                    ChangeVisualState(true);
                }
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

        internal bool IsToday
        {
            get
            {
                return this._isToday;
            }
            set
            {
                this._isToday = value;
                ChangeVisualState(true);
            }
        }

        internal bool IsCurrent
        {
            get
            {
                return this._isCurrent;
            }
            set
            {
                this._isCurrent = value;
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
        /// Creates the automation peer for the CalendarDayButton.
        /// </summary>
        /// <returns></returns>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new CalendarDayButtonAutomationPeer(this);
        }

        /// <summary>
        /// Overrides the OnMouseLeftButtonDown method of Button in order 
        /// to be able to get MouseLeftButtonDown events
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
        {
            base.OnMouseLeftButtonDown(e);
            OnCalendarDayButtonMouseDown(e);
        }

        /// <summary>
        /// Overrides the OnMouseLeftButtonUp method of Button in order 
        /// to be able to get MouseLeftButtonUp events
        /// </summary>
        /// <param name="e"></param>
        protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
        {
            base.OnMouseLeftButtonUp(e);
            OnCalendarDayButtonMouseUp(e);
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
            if (_isMouseOverOverride)
            {
                if (this.IsPressed)
                {
                    VisualStates.GoToState(this, useTransitions, VisualStates.StatePressed);
                }
                if (this.IsEnabled)
                {
                    VisualStates.GoToState(this, useTransitions, VisualStates.StateNormal);
                }
                else
                {
                    VisualStates.GoToState(this, useTransitions, VisualStates.StateDisabled);
                }
            }

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

            // Update the DayStates group
            if (IsToday)
            {
                VisualStates.GoToState(this, useTransitions, StateToday, StateRegularDay);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, StateRegularDay);
            }

            // Update the BlackoutDayStates group
            if (IsDisabled)
            {
                VisualStates.GoToState(this, useTransitions, StateBlackoutDay, StateNormalDay);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, StateNormalDay);
            }

            // Update the FocusStates group
            //IsCurrent implies the focused element on Calendar control
            if (IsCurrent && IsEnabled)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateCalendarButtonFocused, VisualStates.StateCalendarButtonUnfocused);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateCalendarButtonUnfocused);
            }
        }

        internal void OnCalendarDayButtonMouseDown(MouseButtonEventArgs e)
        {
            MouseButtonEventHandler handler = this.CalendarDayButtonMouseDown;
            if (null != handler)
            {
                handler(this, e);
            }
        }

        internal void OnCalendarDayButtonMouseUp(MouseButtonEventArgs e)
        {
            MouseButtonEventHandler handler = this.CalendarDayButtonMouseUp;
            
            if (null != handler)
            {
                handler(this, e);
            }
        }

        //We need to send a MouseUp event for the CalendarDayButton 
        //that stays in Pressed state after MouseCapture is released since there is no actual MouseUpEvent for the release
        internal void SendMouseUpEvent(MouseButtonEventArgs e)
        {
            e.Handled = false;
            base.OnMouseLeftButtonUp(e);
        }

        #endregion Internal Methods
    }
}
