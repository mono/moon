// -------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All Rights Reserved.
// -------------------------------------------------------------------

using System.Windows.Controls;

namespace System.Windows
{
    /// <summary>
    ///     EventArgs for VisualStateGroup.CurrentStateChanging and CurrentStateChanged events.
    /// </summary>
    public sealed class VisualStateChangedEventArgs : EventArgs
    {
        internal VisualStateChangedEventArgs(VisualState oldState, VisualState newState, Control control)
        {
            OldState = oldState;
            NewState = newState;
            Control = control;
        }

        public VisualStateChangedEventArgs ()
        {
                
        }

        /// <summary>
        ///     The old state the control is transitioning from
        /// </summary>
        public VisualState OldState {
            get; set;
        }

        /// <summary>
        ///     The new state the control is transitioning to
        /// </summary>
        public VisualState NewState {
            get; set;
        }

        /// <summary>
        ///     The control involved in the state change
        /// </summary>
        public Control Control {
            get; set;
        }
    }
}
