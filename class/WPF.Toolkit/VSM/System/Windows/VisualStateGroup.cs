// -------------------------------------------------------------------
// Copyright (c) Microsoft Corporation. All Rights Reserved.
// -------------------------------------------------------------------

using System;
using System.Collections;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Media.Animation;

using Mono;

namespace System.Windows
{
    /// <summary>
    ///     A group of mutually exclusive visual states.
    /// </summary>
    [ContentProperty("States")]
    public class VisualStateGroup : DependencyObject
    {
        /// <summary>
        ///     The Name of the VisualStateGroup.
        /// </summary>
        public string Name
        {
            get { return _name; }
            internal set { _name = value; }
        }

        /// <summary>
        ///     VisualStates in the group.
        /// </summary>
        public IList States
        {
            get
            {
                if (_states == null)
                {
                    _states = new Collection<VisualState>();
                }

                return _states;
            }
        }

        /// <summary>
        ///     Transitions between VisualStates in the group.
        /// </summary>
        public IList Transitions
        {
            get
            {
                if (_transitions == null)
                {
                    _transitions = new Collection<VisualTransition>();
                }

                return _transitions;
            }
        }

        /// <summary>
        ///     VisualState that is currently applied.
        /// </summary>
        internal VisualState CurrentState
        {
            get; set;
        }

        internal VisualState GetState(string stateName)
        {
            for (int stateIndex = 0; stateIndex < States.Count; ++stateIndex)
            {
                VisualState state = (VisualState)States[stateIndex];
                if (state.Name == stateName)
                {
                    return state;
                }
            }

            return null;
        }

        internal Collection<Storyboard> CurrentStoryboards
        {
            get
            {
                if (_currentStoryboards == null)
                {
                    _currentStoryboards = new Collection<Storyboard>();
                }

                return _currentStoryboards;
            }
        }

        internal void StartNewThenStopOld(FrameworkElement element, params Storyboard[] newStoryboards)
        {
            // Start the new Storyboards
            for (int index = 0; index < newStoryboards.Length; ++index)
            {
                if (newStoryboards[index] == null)
                {
                    continue;
                }

                element.Resources.Add (newStoryboards [index].native.ToString (), newStoryboards[index]);
                try {
                    newStoryboards[index].Begin();
                } catch {
                    // If an exception is thrown calling begin, clear all the SBs out of the tree before propagating
                    for (int i = 0; i <= index; i++)
                        if (newStoryboards [i] != null)
                        element.Resources.Remove (newStoryboards [i].native.ToString ());

                    throw;
                }
                // Silverlight had an issue where initially, a checked CheckBox would not show the check mark
                // until the second frame. They chose to do a Seek(0) at this point, which this line
                // is supposed to mimic. It does not seem to be equivalent, though, and WPF ends up
                // with some odd animation behavior. I haven't seen the CheckBox issue on WPF, so
                // commenting this out for now.
                // newStoryboards[index].SeekAlignedToLastTick(element, TimeSpan.Zero, TimeSeekOrigin.BeginTime);
            }

            // Stop the old Storyboards
            for (int index = 0; index < CurrentStoryboards.Count; ++index)
            {
                if (CurrentStoryboards[index] == null)
                {
                    continue;
                }

                element.Resources.Remove (CurrentStoryboards [index].native.ToString ());
                CurrentStoryboards[index].Stop();
            }

            // Hold on to the running Storyboards
            CurrentStoryboards.Clear();
            for (int index = 0; index < newStoryboards.Length; ++index)
            {
                CurrentStoryboards.Add(newStoryboards[index]);
            }
        }

		internal void RaiseCurrentStateChanging(FrameworkElement element, VisualState oldState, VisualState newState, Control control)
		{
			RaiseCurrentStateChanging (new VisualStateChangedEventArgs(oldState, newState, control));
		}

		internal void RaiseCurrentStateChanged(FrameworkElement element, VisualState oldState, VisualState newState, Control control)
		{
			RaiseCurrentStateChanged(new VisualStateChangedEventArgs(oldState, newState, control));
		}

		internal void RaiseCurrentStateChanging (VisualStateChangedEventArgs e)
		{
			EventHandler<VisualStateChangedEventArgs> h = (EventHandler<VisualStateChangedEventArgs>) EventList [CurrentStateChangingEvent];
			if (h != null)
				h (this, e);
		}

		internal void RaiseCurrentStateChanged (VisualStateChangedEventArgs e)
		{
			EventHandler<VisualStateChangedEventArgs> h = (EventHandler<VisualStateChangedEventArgs>) EventList [CurrentStateChangedEvent];
			if (h != null)
				h (this, e);
		}

		static object CurrentStateChangingEvent = new object ();
		static object CurrentStateChangedEvent = new object ();

        /// <summary>
        ///     Raised when transition begins
        /// </summary>
		public event EventHandler<VisualStateChangedEventArgs> CurrentStateChanging {
			add {
				RegisterEvent (CurrentStateChangingEvent, "CurrentStateChanging", Events.current_state_changing, value);
			}
			remove {
				UnregisterEvent (CurrentStateChangingEvent, "CurrentStateChanging", Events.current_state_changing, value);
			}
		}

        /// <summary>
        ///     Raised when transition ends and new state storyboard begins.
        /// </summary>
		public event EventHandler<VisualStateChangedEventArgs> CurrentStateChanged {
			add {
				RegisterEvent (CurrentStateChangedEvent, "CurrentStateChanged", Events.current_state_changed, value);
			}
			remove {
				UnregisterEvent (CurrentStateChangedEvent, "CurrentStateChanged", Events.current_state_changed, value);
			}
		}


        private Collection<Storyboard> _currentStoryboards;
        private Collection<VisualState> _states;
        private Collection<VisualTransition> _transitions;
        private string _name = String.Empty;
    }
}
