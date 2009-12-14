// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Automation.Provider;
using System.Windows.Controls;

namespace System.Windows.Automation.Peers
{
    /// <summary>
    /// AutomationPeer for GridSplitter
    /// </summary>
    sealed public class GridSplitterAutomationPeer : FrameworkElementAutomationPeer, ITransformProvider
    {
        #region Constructors

        /// <summary>
        /// AutomationPeer for GridSplitter
        /// </summary>
        /// <param name="owner">GridSplitter</param>
        public GridSplitterAutomationPeer(GridSplitter owner)
            : base(owner)
        {
        }

        #endregion

        #region AutomationPeer Overrides

        /// <summary>
        /// Gets the control type for the element that is associated with the UI Automation peer.
        /// </summary>
        /// <returns>The control type.</returns>
        protected override AutomationControlType GetAutomationControlTypeCore()
        {
            return AutomationControlType.Thumb;
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
        /// Gets the control pattern that is associated with the specified System.Windows.Automation.Peers.PatternInterface.
        /// </summary>
        /// <param name="patternInterface">A value from the System.Windows.Automation.Peers.PatternInterface enumeration.</param>
        /// <returns>The object that supports the specified pattern, or null if unsupported.</returns>
        public override object GetPattern(PatternInterface patternInterface)
        {
            if (patternInterface == PatternInterface.Transform)
            {
                return this;
            }
            return base.GetPattern(patternInterface);
        }

        #endregion

        #region ITransformProvider

        bool ITransformProvider.CanMove { get { return true; } }

        bool ITransformProvider.CanResize { get { return false; } }

        bool ITransformProvider.CanRotate { get { return false; } }

        void ITransformProvider.Move(double x, double y)
        {
            GridSplitter owner = (GridSplitter)Owner;
            if (!IsEnabled())
            {
                throw new ElementNotEnabledException();
            }

            if (double.IsInfinity(x) || double.IsNaN(x))
            {
                // 

                return;
            }

            if (double.IsInfinity(y) || double.IsNaN(y))
            {
                // 

                return;
            }

            owner.InitializeAndMoveSplitter(x, y);
        }

        void ITransformProvider.Resize(double width, double height)
        {
            // 

        }

        void ITransformProvider.Rotate(double degrees)
        {
            // 

        }

        #endregion
    }
}
