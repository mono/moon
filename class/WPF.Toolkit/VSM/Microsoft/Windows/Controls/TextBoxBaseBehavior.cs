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
    ///     Provides VisualStateManager behavior for TextBox controls.
    /// </summary>
    public class TextBoxBaseBehavior : ControlBehavior
    {
        /// <summary>
        ///     This behavior targets TextBoxBase derived Controls.
        /// </summary>
        protected override internal Type TargetType
        {
            get { return typeof(TextBoxBase); }
        }

        /// <summary>
        ///     Attaches to property changes and events.
        /// </summary>
        /// <param name="control">An instance of the control.</param>
        protected override void OnAttach(Control control)
        {
            base.OnAttach(control);

            TextBoxBase textBoxBase = (TextBoxBase)control;
            Type targetType = typeof(TextBoxBase);
            EventHandler handler = delegate { UpdateState(textBoxBase, true); };

            AddValueChanged(TextBoxBase.IsMouseOverProperty, targetType, textBoxBase, handler);
            AddValueChanged(TextBoxBase.IsEnabledProperty, targetType, textBoxBase, handler);
            AddValueChanged(TextBoxBase.IsReadOnlyProperty, targetType, textBoxBase, handler);
        }

        /// <summary>
        ///     Called to update the control's visual state.
        /// </summary>
        /// <param name="control">The instance of the control being updated.</param>
        /// <param name="useTransitions">Whether to use transitions or not.</param>
        protected override void UpdateState(Control control, bool useTransitions)
        {
            TextBoxBase textBoxBase = (TextBoxBase)control;

            if (!textBoxBase.IsEnabled)
            {
                VisualStateManager.GoToState(textBoxBase, "Disabled", useTransitions);
            }
            else if (textBoxBase.IsReadOnly)
            {
                VisualStateManager.GoToState(textBoxBase, "ReadOnly", useTransitions);
            }
            else if (textBoxBase.IsMouseOver)
            {
                VisualStateManager.GoToState(textBoxBase, "MouseOver", useTransitions);
            }
            else
            {
                VisualStateManager.GoToState(textBoxBase, "Normal", useTransitions);
            }

            base.UpdateState(control, useTransitions);
        }
    }
}
