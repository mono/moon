// -------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All Rights Reserved.
// -------------------------------------------------------------------

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;

namespace Microsoft.Windows.Controls
{
    /// <summary>
    ///     Provides VisualStateManager behavior for ProgressBar controls.
    /// </summary>
    public class ProgressBarBehavior : ControlBehavior
    {
        /// <summary>
        ///     This behavior targets ProgressBar derived Controls.
        /// </summary>
        protected override internal Type TargetType
        {
            get { return typeof(ProgressBar); }
        }

        /// <summary>
        ///     Attaches to property changes and events.
        /// </summary>
        /// <param name="control">An instance of the control.</param>
        protected override void OnAttach(Control control)
        {
            base.OnAttach(control);

            ProgressBar progressBar = (ProgressBar)control;
            Type targetType = typeof(ProgressBar);
            EventHandler handler = delegate { UpdateState(progressBar, true); };

            AddValueChanged(ProgressBar.IsIndeterminateProperty, targetType, progressBar, handler);
        }

        /// <summary>
        ///     Called to update the control's visual state.
        /// </summary>
        /// <param name="control">The instance of the control being updated.</param>
        /// <param name="useTransitions">Whether to use transitions or not.</param>
        protected override void UpdateState(Control control, bool useTransitions)
        {
            ProgressBar progressBar = (ProgressBar)control;

            if (!progressBar.IsIndeterminate)
            {
                VisualStateManager.GoToState(progressBar, "Determinate", useTransitions);
            }
            else
            {
                VisualStateManager.GoToState(progressBar, "Indeterminate", useTransitions);
            }

            base.UpdateState(control, useTransitions);
        }
    }
}
