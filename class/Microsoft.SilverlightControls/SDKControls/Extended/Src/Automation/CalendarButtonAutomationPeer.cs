// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Automation.Provider;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace System.Windows.Automation.Peers
{
    /// <summary>
    /// AutomationPeer for CalendarButton
    /// </summary>
    public sealed class CalendarButtonAutomationPeer : FrameworkElementAutomationPeer, IGridItemProvider, IInvokeProvider, ISelectionItemProvider
    {
        /// <summary>
        /// Initializes a new instance of the AutomationPeer for CalendarButton.
        /// </summary>
        /// <param name="owner">CalendarButton</param>
        public CalendarButtonAutomationPeer(CalendarButton owner)
            : base(owner)
        {

        }

        #region Private Properties


        private Calendar OwningCalendar
        {
            get
            {
                return this.OwningCalendarButton.Owner;
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

        private CalendarButton OwningCalendarButton
        {
            get
            {
                return this.Owner as CalendarButton;
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
            if ((patternInterface == PatternInterface.GridItem || patternInterface == PatternInterface.Invoke || patternInterface == PatternInterface.SelectionItem)
                && (this.OwningCalendar != null && this.OwningCalendar.MonthControl != null && this.OwningCalendarButton != null))
            {
                return this;
            }
            return base.GetPattern(patternInterface);
        }

        #endregion Public methods

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
        /// Overrides the GetLocalizedControlTypeCore method for CalendarButtonAutomationPeer
        /// </summary>
        /// <returns></returns>
        protected override string GetLocalizedControlTypeCore()
        {
            return Resource.CalendarAutomationPeer_CalendarButtonLocalizedControlType;
        }

        /// <summary>
        /// Overrides the GetHelpTextCore method for CalendarButtonAutomationPeer
        /// </summary>
        /// <returns></returns>
        protected override string GetHelpTextCore()
        {
            if (this.OwningCalendarButton != null && this.OwningCalendarButton.DataContext != null)
            {
                DateTime dataContext = (DateTime)this.OwningCalendarButton.DataContext;
                return dataContext.Date.ToString(DateTimeHelper.GetCurrentDateFormat().LongDatePattern, DateTimeHelper.GetCurrentDateFormat());
            }

            return base.GetHelpTextCore();
        }

        /// <summary>
        /// Overrides the GetNameCore method for CalendarButtonAutomationPeer
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
                if (string.IsNullOrEmpty(nameCore) && this.OwningCalendarButton.Content != null)
                {
                    nameCore = string.Format(DateTimeHelper.GetCurrentDateFormat(), this.OwningCalendarButton.Content.ToString());
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
                return (int)this.OwningCalendarButton.GetValue(Grid.ColumnProperty);
            }
        }

        int IGridItemProvider.ColumnSpan { get { return 1; } }

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
                return (int)this.OwningCalendarButton.GetValue(Grid.RowProperty);
            }
        }

        int IGridItemProvider.RowSpan { get { return 1; } }

        #endregion IGridItemProvider

        #region IInvokeProvider

        void IInvokeProvider.Invoke()
        {
            if (this.OwningCalendarButton.IsEnabled)
            {
                this.OwningCalendar.MonthControl.UpdateYearViewSelection(this.OwningCalendarButton);
                this.OwningCalendar.MonthControl.Month_CalendarButtonMouseUp(this.OwningCalendarButton, null);
            }
            else
            {
                throw new ElementNotEnabledException();
            }
        }

        #endregion IInvokeProvider

        #region ISelectionItemProvider

        bool ISelectionItemProvider.IsSelected { get { return this.OwningCalendarButton.IsFocusedOverride; } }

        IRawElementProviderSimple ISelectionItemProvider.SelectionContainer
        {
            get
            {
                return this.OwningCalendarAutomationPeer;
            }
        }

        void ISelectionItemProvider.AddToSelection()
        {
            // 

            return;
        }

        void ISelectionItemProvider.RemoveFromSelection()
        {
            // 

            return;
        }

        void ISelectionItemProvider.Select()
        {
            if (EnsureSelection() && !this.OwningCalendarButton.IsFocusedOverride)
            {
                foreach (CalendarButton button in this.OwningCalendar.MonthControl.YearView.Children)
                {
                    if (button.IsFocusedOverride)
                    {
                        button.IsFocusedOverride = false;
                        break;
                    }
                }
                this.OwningCalendarButton.IsFocusedOverride = true;
            }
        }

        #endregion ISelectionItemProvider

        #region Private Methods

        private bool EnsureSelection()
        {
            if (!this.OwningCalendarButton.IsEnabled)
            {
                throw new ElementNotEnabledException();
            }

            if (this.OwningCalendarButton.Visibility == Visibility.Collapsed)
            {
                return false;
            }

            return true;
        }

        #endregion Private Methods
    }
}
