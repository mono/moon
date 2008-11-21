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
    ///     Provides VisualStateManager behavior for ButtonBase controls.
    /// </summary>
    public class ButtonBaseBehavior : ControlBehavior
    {
        /// <summary>
        ///     This behavior targets ButtonBase derived Controls.
        /// </summary>
        protected override internal Type TargetType
        {
            get { return typeof(ButtonBase); }
        }

        /// <summary>
        ///     Attaches to property changes and events.
        /// </summary>
        /// <param name="control">An instance of the control.</param>
        protected override void OnAttach(Control control)
        {
            base.OnAttach(control);

            ButtonBase button = (ButtonBase)control;
            Type targetType = typeof(ButtonBase);
            EventHandler handler = delegate { UpdateState(button, true); };

            AddValueChanged(ButtonBase.IsMouseOverProperty, targetType, button, handler);
            AddValueChanged(ButtonBase.IsEnabledProperty, targetType, button, handler);
            AddValueChanged(ButtonBase.IsPressedProperty, targetType, button, handler);
        }

        /// <summary>
        ///     Called to update the control's visual state.
        /// </summary>
        /// <param name="control">The instance of the control being updated.</param>
        /// <param name="useTransitions">Whether to use transitions or not.</param>
        protected override void UpdateState(Control control, bool useTransitions)
        {
            ButtonBase button = (ButtonBase)control;

            if (!button.IsEnabled)
            {
                VisualStateManager.GoToState(button, "Disabled", useTransitions);
            }
            else if (button.IsPressed)
            {
                VisualStateManager.GoToState(button, "Pressed", useTransitions);
            }
            else if (button.IsMouseOver)
            {
                VisualStateManager.GoToState(button, "MouseOver", useTransitions);
            }
            else
            {
                VisualStateManager.GoToState(button, "Normal", useTransitions);
            }

            base.UpdateState(control, useTransitions);
        }
    }
}
