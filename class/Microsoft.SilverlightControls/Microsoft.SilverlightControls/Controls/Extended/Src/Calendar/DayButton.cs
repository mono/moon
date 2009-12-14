// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Diagnostics;
using System.Windows.Media.Animation; 
using System.Windows.Input; 

namespace System.Windows.Controlsb1 
{
    /// <summary>
    /// Represents a button control used in Calendar Control, which reacts to the Click event. 
    /// </summary>
    [TemplatePart(Name = DayButton.DAYBUTTON_elementRootName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = DayButton.DAYBUTTON_elementFocusVisualName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateNormalName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateDisabledName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateSelectedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateNormalInactiveName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateNormalTodayName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateNormalInactiveTodayName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateDisabledTodayName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateDisabledInactiveName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateDisabledInactiveTodayName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateSelectedTodayName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateSelectedInactiveName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateSelectedInactiveTodayName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateMouseOverName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateMouseOverInactiveName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateMouseOverSelectedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateMouseOverTodayName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateMouseOverSelectedInactiveName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_stateMouseOverSelectedTodayName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_stateMouseOverInactiveTodayName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_statePressName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_statePressInactiveName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_statePressSelectedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_statePressTodayName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_statePressSelectedInactiveName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = DayButton.DAYBUTTON_statePressSelectedTodayName, Type = typeof(Storyboard))]
    [TemplatePart(Name = DayButton.DAYBUTTON_statePressInactiveTodayName, Type = typeof(Storyboard))]
 
 
    public sealed class DayButton : CalendarButtonBase
    { 
        #region Constants
        private const string DAYBUTTON_elementFocusVisualName = "FocusVisualElement";
        private const string DAYBUTTON_elementRootName = "RootElement"; 
        private const string DAYBUTTON_stateDisabledName = "Disabled State";
        private const string DAYBUTTON_stateDisabledInactiveName = "Disabled Inactive State";
        private const string DAYBUTTON_stateDisabledInactiveTodayName = "Disabled Inactive Today State"; 
        private const string DAYBUTTON_stateDisabledTodayName = "Disabled Today State"; 
        private const string DAYBUTTON_stateMouseOverName = "MouseOver State";
        private const string DAYBUTTON_stateMouseOverInactiveName = "MouseOver Inactive State"; 
        private const string DAYBUTTON_stateMouseOverInactiveTodayName = "MouseOver Inactive Today State";
        private const string DAYBUTTON_stateMouseOverSelectedName = "MouseOver Selected State";
        private const string DAYBUTTON_stateMouseOverSelectedInactiveName = "MouseOver Inactive Selected State"; 
        private const string DAYBUTTON_stateMouseOverSelectedTodayName = "MouseOver Selected Today State";
        private const string DAYBUTTON_stateMouseOverTodayName = "MouseOver Today State";
        private const string DAYBUTTON_stateNormalName = "Normal State"; 
        private const string DAYBUTTON_stateNormalInactiveName = "Normal Inactive State"; 
        private const string DAYBUTTON_stateNormalInactiveTodayName = "Normal Inactive Today State";
        private const string DAYBUTTON_stateNormalTodayName = "Normal Today State"; 
        private const string DAYBUTTON_statePressName = "Pressed State";
        private const string DAYBUTTON_statePressInactiveName = "Pressed Inactive State";
        private const string DAYBUTTON_statePressInactiveTodayName = "Pressed Inactive Today"; 
        private const string DAYBUTTON_statePressSelectedName = "Pressed Selected State";
        private const string DAYBUTTON_statePressSelectedInactiveName = "Pressed Inactive Selected";
        private const string DAYBUTTON_statePressSelectedTodayName = "Pressed Selected Today"; 
        private const string DAYBUTTON_statePressTodayName = "Pressed Today State"; 
        private const string DAYBUTTON_stateSelectedName = "Normal Selected State";
        private const string DAYBUTTON_stateSelectedInactiveName = "Normal Inactive Selected State"; 
        private const string DAYBUTTON_stateSelectedInactiveTodayName = "Normal Inactive Selected Today State";
        private const string DAYBUTTON_stateSelectedTodayName = "Normal Selected Today State";
 
        #endregion Constants

        #region Data 
 
        private UIElement _elementFocusVisual;
        private FrameworkElement _elementRoot; 

        private bool _isCurrent;
        private bool _isDisabled; 
        private bool _isInactive;
        private bool _isMouseOverOverride;
        private bool _isSelected; 
        private bool _isToday; 
        private bool _mouseOverFlag;
        private Storyboard _stateDisabled; 
        private Storyboard _stateDisabledInactive;
        private Storyboard _stateDisabledInactiveToday;
        private Storyboard _stateDisabledToday; 
        private Storyboard _stateMouseOver;
        private Storyboard _stateMouseOverInactive;
        private Storyboard _stateMouseOverInactiveToday; 
        private Storyboard _stateMouseOverSelected; 
        private Storyboard _stateMouseOverSelectedInactive;
        private Storyboard _stateMouseOverSelectedToday; 
        private Storyboard _stateMouseOverToday;
        private Storyboard _stateNormal;
        private Storyboard _stateNormalInactive; 
        private Storyboard _stateNormalInactiveToday;
        private Storyboard _stateNormalToday;
        private Storyboard _statePress; 
        private Storyboard _statePressInactive; 
        private Storyboard _statePressInactiveToday;
        private Storyboard _statePressSelected; 
        private Storyboard _statePressSelectedInactive;
        private Storyboard _statePressSelectedToday;
        private Storyboard _statePressToday; 
        private Storyboard _stateSelected;
        private Storyboard _stateSelectedInactive;
        private Storyboard _stateSelectedInactiveToday; 
        private Storyboard _stateSelectedToday; 

        #endregion Data 

        internal DayButton()
            : base() 
        {
        }
 
        #region Internal Properties 

        internal bool IsDisabled 
        {
            get
            { 
                return this._isDisabled;
            }
            set 
            { 
                this._isDisabled = value;
                ChangeVisualState(); 
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
                ChangeVisualState();
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
                this.OnMouseEnter(new MouseEventArgs()); 
                this.OnMouseLeave(new MouseEventArgs());
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
                ChangeVisualState();
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
                ChangeVisualState(); 
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
                ChangeVisualState();
            }
        } 
 
        #endregion Internal Properties
 

        #region Protected Methods
 
        /// <summary>
        /// Invoked whenever application code or an internal process,
        /// such as a rebuilding layout pass, calls the ApplyTemplate method. 
        /// </summary> 

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502", Justification = "Calendar has 27 states")]

        public override void OnApplyTemplate()
        { 
            base.OnApplyTemplate();

            // Get the elements 
            _elementRoot = GetTemplateChild(DAYBUTTON_elementRootName) as FrameworkElement; 
            _elementFocusVisual = GetTemplateChild(DAYBUTTON_elementFocusVisualName) as UIElement;
 
            // Get the states
            if (_elementRoot != null)
            { 
                _stateNormal = _elementRoot.Resources[DAYBUTTON_stateNormalName] as Storyboard;
                _stateDisabled = _elementRoot.Resources[DAYBUTTON_stateDisabledName] as Storyboard;
                _stateSelected = _elementRoot.Resources[DAYBUTTON_stateSelectedName] as Storyboard; 
                _stateNormalInactive = _elementRoot.Resources[DAYBUTTON_stateNormalInactiveName] as Storyboard; 
                _stateNormalToday = _elementRoot.Resources[DAYBUTTON_stateNormalTodayName] as Storyboard;
                _stateNormalInactiveToday = _elementRoot.Resources[DAYBUTTON_stateNormalInactiveTodayName] as Storyboard; 
                _stateDisabledToday = _elementRoot.Resources[DAYBUTTON_stateDisabledTodayName] as Storyboard;
                _stateDisabledInactive = _elementRoot.Resources[DAYBUTTON_stateDisabledInactiveName] as Storyboard;
                _stateDisabledInactiveToday = _elementRoot.Resources[DAYBUTTON_stateDisabledInactiveTodayName] as Storyboard; 
                _stateSelectedToday = _elementRoot.Resources[DAYBUTTON_stateSelectedTodayName] as Storyboard;
                _stateSelectedInactive = _elementRoot.Resources[DAYBUTTON_stateSelectedInactiveName] as Storyboard;
                _stateSelectedInactiveToday = _elementRoot.Resources[DAYBUTTON_stateSelectedInactiveTodayName] as Storyboard; 
                _stateMouseOver = _elementRoot.Resources[DAYBUTTON_stateMouseOverName] as Storyboard; 
                _stateMouseOverInactive = _elementRoot.Resources[DAYBUTTON_stateMouseOverInactiveName] as Storyboard;
                _stateMouseOverSelected = _elementRoot.Resources[DAYBUTTON_stateMouseOverSelectedName] as Storyboard; 
                _stateMouseOverToday = _elementRoot.Resources[DAYBUTTON_stateMouseOverTodayName] as Storyboard;
                _stateMouseOverSelectedInactive = _elementRoot.Resources[DAYBUTTON_stateMouseOverSelectedInactiveName] as Storyboard;
                _stateMouseOverSelectedToday = _elementRoot.Resources[DAYBUTTON_stateMouseOverSelectedTodayName] as Storyboard; 
                _stateMouseOverInactiveToday = _elementRoot.Resources[DAYBUTTON_stateMouseOverInactiveTodayName] as Storyboard;
                _statePress = _elementRoot.Resources[DAYBUTTON_statePressName] as Storyboard;
                _statePressInactive = _elementRoot.Resources[DAYBUTTON_statePressInactiveName] as Storyboard; 
                _statePressSelected = _elementRoot.Resources[DAYBUTTON_statePressSelectedName] as Storyboard; 
                _statePressToday = _elementRoot.Resources[DAYBUTTON_statePressTodayName] as Storyboard;
                _statePressSelectedInactive = _elementRoot.Resources[DAYBUTTON_statePressSelectedInactiveName] as Storyboard; 
                _statePressSelectedToday = _elementRoot.Resources[DAYBUTTON_statePressSelectedTodayName] as Storyboard;
                _statePressInactiveToday = _elementRoot.Resources[DAYBUTTON_statePressInactiveTodayName] as Storyboard;
            } 

            // Sync the logical and visual states of the control
            ChangeVisualState(); 
        } 

        /// <summary> 
        /// Responds to the MouseEnter event.
        /// </summary>
        /// <param name="e">The event data for the MouseEnter event.</param> 
        protected override void OnMouseEnter(MouseEventArgs e)
        {
            _mouseOverFlag = true; 
            base.OnMouseEnter(e); 
        }
 
        /// <summary>
        /// Responds to the MouseLeave event.
        /// </summary> 
        /// <param name="e">The event data for the MouseLeave event.</param>
        protected override void OnMouseLeave(MouseEventArgs e)
        { 
            _mouseOverFlag = false; 
            base.OnMouseLeave(e);
        } 

        #endregion Protected Methods
 
        #region Internal Methods

        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502", Justification = "Calendar has 27 states")] 
        internal override void ChangeVisualState() 
        {
            if (!IsEnabled) 
            {
                if (IsInactive && IsToday)
                { 
                    ChangeVisualState(_stateDisabledInactiveToday);
                }
                else 
                { 
                    if (IsInactive)
                    { 
                        ChangeVisualState(_stateDisabledInactive);
                    }
                    else 
                    {
                        if (IsToday)
                        { 
                            ChangeVisualState(_stateDisabledToday); 
                        }
                        else 
                        {
                            ChangeVisualState(_stateDisabled);
                        } 
                    }
                }
            } 
 
            else if (IsPressed)
            { 
                if (IsSelected && IsToday)
                {
                    ChangeVisualState(_statePressSelectedToday); 
                }
                else
                { 
                    if (IsInactive && IsToday) 
                    {
                        ChangeVisualState(_statePressInactiveToday); 
                    }
                    else
                    { 
                        if (IsSelected && IsInactive)
                        {
                            ChangeVisualState(_statePressSelectedInactive); 
                        } 
                        else
                        { 
                            if (IsToday)
                            {
                                ChangeVisualState(_statePressToday); 
                            }
                            else
                            { 
                                if (IsSelected) 
                                {
                                    ChangeVisualState(_statePressSelected); 
                                }
                                else
                                { 
                                    if (IsInactive)
                                    {
                                        ChangeVisualState(_statePressInactive); 
                                    } 
                                    else
                                    { 
                                        ChangeVisualState(_statePress);
                                    }
                                } 
                            }
                        }
                    } 
                } 

            } 
            else if (IsMouseOver && _mouseOverFlag)
            {
                if (IsSelected && IsToday) 
                {
                    ChangeVisualState(_stateMouseOverSelectedToday);
                } 
                else 
                {
                    if (IsInactive && IsToday) 
                    {
                        ChangeVisualState(_stateMouseOverInactiveToday);
                    } 
                    else
                    {
                        if (IsSelected && IsInactive) 
                        { 
                            ChangeVisualState(_stateMouseOverSelectedInactive);
                        } 
                        else
                        {
                            if (IsToday) 
                            {
                                ChangeVisualState(_stateMouseOverToday);
                            } 
                            else 
                            {
                                if (IsSelected) 
                                {
                                    ChangeVisualState(_stateMouseOverSelected);
                                } 
                                else
                                {
                                    if (IsInactive) 
                                    { 
                                        ChangeVisualState(_stateMouseOverInactive);
                                    } 
                                    else
                                    {
                                        ChangeVisualState(_stateMouseOver); 
                                    }
                                }
                            } 
                        } 
                    }
                } 
            }
            else
            { 
                if (IsSelected && IsInactive && IsToday)
                {
                    ChangeVisualState(_stateSelectedInactiveToday); 
                } 
                else
                { 
                    if (IsSelected && IsToday)
                    {
                        ChangeVisualState(_stateSelectedToday); 
                    }
                    else
                    { 
                        if (IsInactive && IsToday) 
                        {
                            ChangeVisualState(_stateNormalInactiveToday); 
                        }
                        else
                        { 
                            if (IsSelected && IsInactive)
                            {
                                ChangeVisualState(_stateSelectedInactive); 
                            } 
                            else
                            { 
                                if (IsToday)
                                {
                                    ChangeVisualState(_stateNormalToday); 
                                }
                                else
                                { 
                                    if (IsSelected) 
                                    {
                                        ChangeVisualState(_stateSelected); 
                                    }
                                    else
                                    { 
                                        if (IsInactive)
                                        {
                                            ChangeVisualState(_stateNormalInactive); 
                                        } 
                                        else
                                        { 
                                            ChangeVisualState(_stateNormal);
                                        }
                                    } 
                                }
                            }
                        } 
                    } 
                }
            } 

            if (_elementFocusVisual != null)
            { 
                if (IsCurrent && !IsInactive)
                {
                    _elementFocusVisual.Visibility = Visibility.Visible; 
                } 
                else
                { 
                    _elementFocusVisual.Visibility = Visibility.Collapsed;
                }
            } 
        }
        #endregion Internal Methods
    } 
} 
