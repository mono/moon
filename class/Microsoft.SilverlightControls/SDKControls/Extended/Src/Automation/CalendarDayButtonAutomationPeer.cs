// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Diagnostics;
using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;


namespace System.Windows.Automation.Peers
{
    /// <summary>
    /// AutomationPeer for CalendarDayButton
    /// </summary>
    public sealed class CalendarDayButtonAutomationPeer : FrameworkElementAutomationPeer, IGridItemProvider, IInvokeProvider, ISelectionItemProvider, ITableItemProvider
    {
        /// <summary>
        /// Initializes a new instance of the AutomationPeer for CalendarDayButton.
        /// </summary>
        /// <param name="owner">CalendarDayButton</param>
        public CalendarDayButtonAutomationPeer(CalendarDayButton owner)
            :base(owner)
        {
        }

        #region Private Properties

        private System.Windows.Controls.Calendar OwningCalendar
        {
            get
            {
                return this.OwningCalendarDayButton.Owner;
            }
        }

        private IRawElementProviderSimple OwningCalendarAutomationPeer
        {
            get
            {
                if (this.OwningCalendar != null)
                {
                    AutomationPeer peer = CreatePeerForElement(this.OwningCalendar);

                    if (peer != null)
                    {
                        return ProviderFromPeer(peer);
                    }
                }
                return null;
            }
        }

        private CalendarDayButton OwningCalendarDayButton
        {
            get
            {
                return this.Owner as CalendarDayButton;
            }
        }

        #endregion Private Properties

        #region Public Methods

        /// <summary>
        /// Gets the control pattern that is associated with the specified System.Windows.Automation.Peers.PatternInterface.
        /// </summary>
        /// <param name="patternInterface">A value from the System.Windows.Automation.Peers.PatternInterface enumeration.</param>
        /// <returns>The object that supports the specified pattern, or null if unsupported.</returns>
        public override object GetPattern(PatternInterface patternInterface)
        {
            if ((patternInterface == PatternInterface.GridItem || patternInterface == PatternInterface.Invoke || patternInterface == PatternInterface.SelectionItem || patternInterface == PatternInterface.TableItem) &&
                (this.OwningCalendar != null && this.OwningCalendarDayButton != null))
            {
                return this;
            }
            return base.GetPattern(patternInterface);
        }

        #endregion Public Methods

        #region Protected Methods

        /// <summary>
        /// Gets the control type for the element that is associated with the UI Automation peer.
        /// </summary>
        /// <returns>The control type.</returns>
        protected override AutomationControlType GetAutomationControlTypeCore()
        {
            return AutomationControlType.Button;
        }

        /// <summary>
        /// Called by GetClassName that gets a human readable name that, in addition to AutomationControlType, 
        /// differentiates the control represented by this AutomationPeer.
        /// </summary>
        /// <returns>The string that contains the name.</returns>
        protected override string GetClassNameCore()
        {
            return Owner.GetType().Name;
        }

        /// <summary>
        /// Overrides the GetHelpTextCore method for CalendarDayButtonAutomationPeer
        /// </summary>
        /// <returns></returns>
        protected override string GetHelpTextCore()
        {
            if (this.OwningCalendarDayButton != null && this.OwningCalendarDayButton.DataContext != null)
            {
                DateTime dataContext = (DateTime)this.OwningCalendarDayButton.DataContext;

                if (this.OwningCalendarDayButton.IsDisabled)
                {
                    return string.Format(DateTimeHelper.GetCurrentDateFormat(), Resource.CalendarAutomationPeer_BlackoutDayHelpText, dataContext.Date.ToString(DateTimeHelper.GetCurrentDateFormat().LongDatePattern, DateTimeHelper.GetCurrentDateFormat()));
                }
                return dataContext.Date.ToString(DateTimeHelper.GetCurrentDateFormat().LongDatePattern, DateTimeHelper.GetCurrentDateFormat());
            }

            return base.GetHelpTextCore();
        }

        /// <summary>
        /// Overrides the GetLocalizedControlTypeCore method for CalendarDayButtonAutomationPeer
        /// </summary>
        /// <returns></returns>
        protected override string GetLocalizedControlTypeCore()
        {
            return Resource.CalendarAutomationPeer_DayButtonLocalizedControlType;
        }

        /// <summary>
        /// Overrides the GetNameCore method for CalendarDayButtonAutomationPeer
        /// </summary>
        /// <returns></returns>
        protected override string GetNameCore()
        {
            string nameCore = base.GetNameCore();

            if (string.IsNullOrEmpty(nameCore))
            {
                AutomationPeer labeledByCore = this.GetLabeledByCore();
                if (labeledByCore != null)
                {
                    nameCore = labeledByCore.GetName();
                }
                if (string.IsNullOrEmpty(nameCore) && this.OwningCalendarDayButton != null && this.OwningCalendarDayButton.Content != null)
                {
                    nameCore = string.Format(DateTimeHelper.GetCurrentDateFormat(), this.OwningCalendarDayButton.Content.ToString());
                }
            }
            return nameCore;
        }

        #endregion Protected Methods

        #region IGridItemProvider

        int IGridItemProvider.Column
        {
            get
            {
                return (int)this.OwningCalendarDayButton.GetValue(Grid.ColumnProperty);
            }
        }

        int IGridItemProvider.ColumnSpan
        {
            get { return 1; }
        }

        IRawElementProviderSimple IGridItemProvider.ContainingGrid
        {
            get
            {
                return this.OwningCalendarAutomationPeer;
            }
        }

        int IGridItemProvider.Row
        {
            get
            {
                //we decrement the Row value by one since the first row is composed of DayTitles
                Debug.Assert((int)this.OwningCalendarDayButton.GetValue(Grid.RowProperty) > 0);
                return (int)this.OwningCalendarDayButton.GetValue(Grid.RowProperty) - 1;
            }
        }

        int IGridItemProvider.RowSpan
        {
            get { return 1; }
        }

        #endregion IGridItemProvider

        #region IInvokeProvider

        void IInvokeProvider.Invoke()
        {
            if (EnsureSelection())
            {
                this.OwningCalendar.SelectedDates.Clear();

                if (this.OwningCalendarDayButton.DataContext != null)
                {
                    this.OwningCalendar.SelectedDates.Add((DateTime)this.OwningCalendarDayButton.DataContext);
                    this.OwningCalendar.OnDayClick((DateTime)this.OwningCalendarDayButton.DataContext);
                }
            }
        }

        #endregion IInvokeProvider

        #region ISelectionItemProvider

        bool ISelectionItemProvider.IsSelected { get { return this.OwningCalendarDayButton.IsSelected; } }

        IRawElementProviderSimple ISelectionItemProvider.SelectionContainer
        {
            get
            {
                return this.OwningCalendarAutomationPeer;
            }
        }

        void ISelectionItemProvider.AddToSelection()
        {
            // Return if the item is already selected or a day is already selected in the SingleSelectionMode
            if (((ISelectionItemProvider)this).IsSelected)
            {
                return;
            }

            if (EnsureSelection() && this.OwningCalendarDayButton.DataContext != null)
            {
                if (this.OwningCalendar.SelectionMode == CalendarSelectionMode.SingleDate)
                {
                    this.OwningCalendar.SelectedDate = (DateTime)this.OwningCalendarDayButton.DataContext;
                }
                else
                {
                    this.OwningCalendar.SelectedDates.Add((DateTime)this.OwningCalendarDayButton.DataContext);
                }
            }
        }

        void ISelectionItemProvider.RemoveFromSelection()
        {
            // Return if the item is not already selected.
            if (!(((ISelectionItemProvider)this).IsSelected))
            {
                return;
            }

            if (this.OwningCalendarDayButton.DataContext != null)
            {
                this.OwningCalendar.SelectedDates.Remove((DateTime)this.OwningCalendarDayButton.DataContext);
            }
        }

        void ISelectionItemProvider.Select()
        {
            if (EnsureSelection())
            {
                this.OwningCalendar.SelectedDates.Clear();

                if (this.OwningCalendarDayButton.DataContext != null)
                {
                    this.OwningCalendar.SelectedDates.Add((DateTime)this.OwningCalendarDayButton.DataContext);
                }
            }
        }

        #endregion ISelectionItemProvider

        #region ITableItemProvider

        IRawElementProviderSimple[] ITableItemProvider.GetColumnHeaderItems()
        {
            if (this.OwningCalendar != null && this.OwningCalendarAutomationPeer != null)
            {
                IRawElementProviderSimple[] headers = ((ITableProvider)CreatePeerForElement(this.OwningCalendar)).GetColumnHeaders();

                if ( headers != null)
                {
                    int column = ((IGridItemProvider)this).Column;
                    return new IRawElementProviderSimple[] { headers[column] };
                }
            }
            return null;
        }

        IRawElementProviderSimple[] ITableItemProvider.GetRowHeaderItems()
        {
            return null;
        }

        #endregion ITableItemProvider

        #region Private Methods

        private bool EnsureSelection()
        {
            if (!this.OwningCalendarDayButton.IsEnabled)
            {
                throw new ElementNotEnabledException();
            }

            //If the day is a blackout day or the SelectionMode is None, selection is not allowed
            if (this.OwningCalendarDayButton.IsDisabled || this.OwningCalendarDayButton.Visibility == Visibility.Collapsed ||
                this.OwningCalendar.SelectionMode == CalendarSelectionMode.None)
            {
                return false;
            }

            return true;
        }

        #endregion Private Methods
    }
}
