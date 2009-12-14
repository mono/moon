// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Collections.Generic;
using System.Diagnostics;
using System.Windows.Automation.Peers;
using System.Windows.Input;
using System.Windows.Media;

namespace System.Windows.Controls
{
    /// <summary>
    /// A child item of TabControl.
    /// </summary>
    [TemplatePart(Name = TabItem.ElementTemplateTopSelectedName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = TabItem.ElementTemplateBottomSelectedName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = TabItem.ElementTemplateLeftSelectedName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = TabItem.ElementTemplateRightSelectedName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = TabItem.ElementTemplateTopUnselectedName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = TabItem.ElementTemplateBottomUnselectedName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = TabItem.ElementTemplateLeftUnselectedName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = TabItem.ElementTemplateRightUnselectedName, Type = typeof(FrameworkElement))]
    [TemplatePart(Name = TabItem.ElementHeaderTopSelectedName, Type = typeof(ContentControl))]
    [TemplatePart(Name = TabItem.ElementHeaderBottomSelectedName, Type = typeof(ContentControl))]
    [TemplatePart(Name = TabItem.ElementHeaderLeftSelectedName, Type = typeof(ContentControl))]
    [TemplatePart(Name = TabItem.ElementHeaderRightSelectedName, Type = typeof(ContentControl))]
    [TemplatePart(Name = TabItem.ElementHeaderTopUnselectedName, Type = typeof(ContentControl))]
    [TemplatePart(Name = TabItem.ElementHeaderBottomUnselectedName, Type = typeof(ContentControl))]
    [TemplatePart(Name = TabItem.ElementHeaderLeftUnselectedName, Type = typeof(ContentControl))]
    [TemplatePart(Name = TabItem.ElementHeaderRightUnselectedName, Type = typeof(ContentControl))]
    [TemplateVisualState(Name = VisualStates.StateNormal, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateMouseOver, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateDisabled, GroupName = VisualStates.GroupCommon)]
    [TemplateVisualState(Name = VisualStates.StateUnselected, GroupName = VisualStates.GroupSelection)]
    [TemplateVisualState(Name = VisualStates.StateSelected, GroupName = VisualStates.GroupSelection)]
    [TemplateVisualState(Name = VisualStates.StateUnfocused, GroupName = VisualStates.GroupFocus)]
    [TemplateVisualState(Name = VisualStates.StateFocused, GroupName = VisualStates.GroupFocus)]
    public partial class TabItem : ContentControl
    {

        #region Constructor

        /// <summary>
        /// Default TabItem Constructor
        /// </summary>
        public TabItem() : base()
        {
            MouseLeftButtonDown += new MouseButtonEventHandler(OnMouseLeftButtonDown);
            MouseEnter += new MouseEventHandler(OnMouseEnter);
            MouseLeave += new MouseEventHandler(OnMouseLeave);
            GotFocus += delegate { IsFocused = true; };
            LostFocus += delegate { IsFocused = false; };
            IsEnabledChanged += new DependencyPropertyChangedEventHandler(OnIsEnabledChanged);
            DefaultStyleKey = typeof(TabItem);
        }

        /// <summary>
        /// Apply a template to the TabItem.
        /// </summary>
        public override void OnApplyTemplate()
        {
            base.OnApplyTemplate();

            // Clear previous content from old ContentControl
            ContentControl cc = GetContentControl(IsSelected, TabStripPlacement);
            if (cc != null)
            {
                cc.Content = null;
            }

            // Get the parts
            ElementTemplateTopSelected = GetTemplateChild(ElementTemplateTopSelectedName) as FrameworkElement;
            ElementTemplateBottomSelected = GetTemplateChild(ElementTemplateBottomSelectedName) as FrameworkElement;
            ElementTemplateLeftSelected = GetTemplateChild(ElementTemplateLeftSelectedName) as FrameworkElement;
            ElementTemplateRightSelected = GetTemplateChild(ElementTemplateRightSelectedName) as FrameworkElement;
            ElementTemplateTopUnselected = GetTemplateChild(ElementTemplateTopUnselectedName) as FrameworkElement;
            ElementTemplateBottomUnselected = GetTemplateChild(ElementTemplateBottomUnselectedName) as FrameworkElement;
            ElementTemplateLeftUnselected = GetTemplateChild(ElementTemplateLeftUnselectedName) as FrameworkElement;
            ElementTemplateRightUnselected = GetTemplateChild(ElementTemplateRightUnselectedName) as FrameworkElement;

            ElementHeaderTopSelected = GetTemplateChild(ElementHeaderTopSelectedName) as ContentControl;
            ElementHeaderBottomSelected = GetTemplateChild(ElementHeaderBottomSelectedName) as ContentControl;
            ElementHeaderLeftSelected = GetTemplateChild(ElementHeaderLeftSelectedName) as ContentControl;
            ElementHeaderRightSelected = GetTemplateChild(ElementHeaderRightSelectedName) as ContentControl;
            ElementHeaderTopUnselected = GetTemplateChild(ElementHeaderTopUnselectedName) as ContentControl;
            ElementHeaderBottomUnselected = GetTemplateChild(ElementHeaderBottomUnselectedName) as ContentControl;
            ElementHeaderLeftUnselected = GetTemplateChild(ElementHeaderLeftUnselectedName) as ContentControl;
            ElementHeaderRightUnselected = GetTemplateChild(ElementHeaderRightUnselectedName) as ContentControl;

            // Load Header
            UpdateHeaderVisuals();

            // Update visuals
            ChangeVisualState(false);
        }

        /// <summary>
        /// Creates AutomationPeer (<see cref="UIElement.OnCreateAutomationPeer"/>)
        /// </summary>
        protected override AutomationPeer OnCreateAutomationPeer()
        {
            return new TabItemAutomationPeer(this);
        }

        #endregion Constructor

        #region Header

        /// <summary>
        /// Header is the data used for the header of each item in the control.
        /// </summary>
        public object Header
        {
            get { return GetValue(HeaderProperty); }
            set { SetValue(HeaderProperty, value); }
        }

        /// <summary>
        /// Identifies the Header dependency property.
        /// </summary>
        public static readonly DependencyProperty HeaderProperty = 
            DependencyProperty.Register(
                "Header", 
                typeof(object), 
                typeof(TabItem), 
                new PropertyMetadata(OnHeaderChanged));
        
        /// <summary>
        /// Header property changed handler
        /// </summary>
        /// <param name="d">TabItem that changed its Header.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnHeaderChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            TabItem ctrl = (TabItem)d;

            ctrl.HasHeader = (e.NewValue != null) ? true : false;
            ctrl.OnHeaderChanged(e.OldValue, e.NewValue);
        }

        /// <summary>
        /// This method is invoked when the Header property changes.
        /// </summary>
        /// <param name="oldHeader">The old value of the Header property.</param>
        /// <param name="newHeader">The new value of the Header property.</param>
        protected virtual void OnHeaderChanged(object oldHeader, object newHeader)
        {
            UpdateHeaderVisuals();
        }

        private void UpdateHeaderVisuals()
        {
            ContentControl header = GetContentControl(IsSelected, TabStripPlacement);

            if (header == null)
            {
                return;
            }

            if (HeaderTemplate == null)
            {
                header.Content = Header;
            }
            else
            {
                FrameworkElement headerContent = HeaderTemplate.LoadContent() as FrameworkElement;
                header.Content = headerContent;
            }
        }

        #endregion Header

        #region HasHeader

        /// <summary>
        /// True if Header is non-null, false otherwise.
        /// </summary>
        public bool HasHeader
        {
            get { return (bool)GetValue(HasHeaderProperty); }
            private set { SetValue(HasHeaderProperty, value); }
        }

        /// <summary>
        /// Identifies the HasHeader dependency property.
        /// </summary>
        public static readonly DependencyProperty HasHeaderProperty = 
            DependencyProperty.Register(
                "HasHeader", 
                typeof(bool), 
                typeof(TabItem), 
                null);
        
        #endregion HasHeader

        #region HeaderTemplate

        /// <summary>
        /// Header is the data used for the header of each item in the control.
        /// </summary>
        public DataTemplate HeaderTemplate
        {
            get { return (DataTemplate)GetValue(HeaderTemplateProperty); }
            set { SetValue(HeaderTemplateProperty, value); }
        }

        /// <summary>
        /// Identifies the HeaderTemplate dependency property.
        /// </summary>
        public static readonly DependencyProperty HeaderTemplateProperty =
            DependencyProperty.Register(
                "HeaderTemplate",
                typeof(DataTemplate),
                typeof(TabItem),
                new PropertyMetadata(OnHeaderTemplateChanged));

        /// <summary>
        /// HeaderTemplate property changed handler
        /// </summary>
        /// <param name="d">TabItem that changed its HeaderTemplate.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnHeaderTemplateChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            TabItem ctrl = (TabItem)d;
            ctrl.OnHeaderTemplateChanged((DataTemplate)e.OldValue, (DataTemplate)e.NewValue);
        }

        /// <summary>
        /// This method is invoked when the HeaderTemplate property changes.
        /// </summary>
        /// <param name="oldHeaderTemplate">The old value of the HeaderTemplate property.</param>
        /// <param name="newHeaderTemplate">The new value of the HeaderTemplate property.</param>
        protected virtual void OnHeaderTemplateChanged(DataTemplate oldHeaderTemplate, DataTemplate newHeaderTemplate)
        {
            UpdateHeaderVisuals();
        }

        #endregion HeaderTemplate

        #region IsSelected

        /// <summary>
        /// Whether this TabItem is currently selected
        /// </summary>
        public bool IsSelected
        {
            get { return (bool)GetValue(IsSelectedProperty); }
            set { SetValue(IsSelectedProperty, value); }
        }

        /// <summary>
        /// Identifies the IsSelected dependency property.
        /// </summary>
        public static readonly DependencyProperty IsSelectedProperty = 
            DependencyProperty.Register(
                "IsSelected", 
                typeof(bool), 
                typeof(TabItem), 
                new PropertyMetadata(OnIsSelectedChanged));

        /// <summary>
        /// IsSelected changed handler
        /// </summary>
        /// <param name="d">TabItem that changed IsSelected.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsSelectedChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            TabItem tabItem = d as TabItem;
            Debug.Assert(tabItem != null);

            bool isSelected = (bool)e.NewValue;

            RoutedEventArgs args = new RoutedEventArgs();

            if (isSelected)
            {
                tabItem.OnSelected(args);
            }
            else
            {
                tabItem.OnUnselected(args);
            }

            // fire the IsSelectedChanged event for automation
            if (AutomationPeer.ListenerExists(AutomationEvents.SelectionItemPatternOnElementSelected))
            {
                TabControl parentSelector = tabItem.TabControlParent;
                if (parentSelector != null)
                {
                    TabItemAutomationPeer tabItemPeer = GetTabItemAutomationPeer(tabItem);
                    if (tabItemPeer != null)
                    {
                        tabItemPeer.RaiseAutomationIsSelectedChanged(isSelected);
                    }
                }
            }

            tabItem.IsTabStop = isSelected;
            tabItem.UpdateVisualState();
        }

        /// <summary>
        /// We use this function to get the TabItemAutomationPeer associated with the TabItem
        /// </summary>
        /// <param name="item">TabItem that we are seeking to find the AutomationPeer for</param>
        /// <returns>The TabItemAutomationPeer for the specified TabItem</returns>
        internal static TabItemAutomationPeer GetTabItemAutomationPeer(TabItem item)
        {
            TabControlAutomationPeer tabControlPeer = TabControlAutomationPeer.FromElement(item.TabControlParent) as TabControlAutomationPeer;
                
            if(tabControlPeer == null)
            {
                tabControlPeer = TabControlAutomationPeer.CreatePeerForElement(item.TabControlParent) as TabControlAutomationPeer;
            }

            if (tabControlPeer != null)
            {
                List<AutomationPeer> children = tabControlPeer.GetChildren();
                if (children != null)
                {
                    foreach (AutomationPeer peer in children)
                    {
                        TabItemAutomationPeer tabItemPeer = peer as TabItemAutomationPeer;
                        if (tabItemPeer != null && tabItemPeer.Owner == item)
                        {
                            return tabItemPeer;
                        }
                    }
                }
            }

            return null;
        }

        /// <summary>
        /// Event indicating that the IsSelected property is now true.
        /// </summary>
        /// <param name="e">Event arguments</param>
        protected virtual void OnSelected(RoutedEventArgs e)
        {
            if (TabControlParent != null)
            {
                TabControlParent.SelectedItem = this;
            }
        }

        /// <summary>
        /// Event indicating that the IsSelected property is now false.
        /// </summary>
        /// <param name="e">Event arguments</param>
        protected virtual void OnUnselected(RoutedEventArgs e)
        {
            if (TabControlParent != null && TabControlParent.SelectedItem == this)
            {
                TabControlParent.SelectedIndex = -1;
            }
        }

        #endregion IsSelected

        #region TabStripPlacement

        /// <summary>
        /// The placement of the TabItem in the UI (Top, Bottom, Left, Right)
        /// </summary>
        public Dock TabStripPlacement
        {
            get { return ((TabControlParent == null) ? Dock.Top : TabControlParent.TabStripPlacement);}
        }

        #endregion TabStripPlacement

        #region OnContentChanged

        /// <summary>
        /// This method is invoked when the Content property changes.
        /// </summary>
        /// <param name="oldContent">The old value of the Content property.</param>
        /// <param name="newContent">The new value of the Content property.</param>
        protected override void OnContentChanged(object oldContent, object newContent)
        {
            base.OnContentChanged(oldContent, newContent);

            TabControl tabControl = TabControlParent;
            if (tabControl != null)
            {
                // If this is the selected TabItem then we should update TabControl.SelectedContent
                if (IsSelected)
                {
                    tabControl.SelectedContent = newContent;
                }
            }
        }

        #endregion OnContentChanged

        #region OnKeyPressed

        /// <summary>
        /// This is the method that responds to the KeyDown event.
        /// </summary>
        /// <param name="e"></param>
        protected override void OnKeyDown(KeyEventArgs e)
        {
            base.OnKeyDown(e);
            if (e.Handled)
            {
                return;
            }

            TabItem nextTabItem = null;

            int direction = 0;
            int startIndex = TabControlParent.Items.IndexOf(this);
            switch (e.Key)
            {
                case Key.Right:
                case Key.Up:
                    direction = 1;
                    break;
                case Key.Left:
                case Key.Down:
                    direction = -1;
                    break;
                default:
                    return;
            }

            nextTabItem = TabControlParent.FindNextTabItem(startIndex, direction);

            if (nextTabItem != null && nextTabItem != TabControlParent.SelectedItem)
            {
                e.Handled = true;
                TabControlParent.SelectedItem = nextTabItem;
                nextTabItem.Focus();
            }
        }

        #endregion OnKeyPressed

        #region IsEnabled
        
        /// <summary>
        /// Called when the IsEnabled property changes.
        /// </summary>
        /// <param name="sender">Control that triggers this property change</param>
        /// <param name="e">Property changed args</param>
        private void OnIsEnabledChanged(object sender, DependencyPropertyChangedEventArgs e)
        {
            Debug.Assert(e.NewValue is bool);
            bool isEnabled = (bool)e.NewValue;
            ContentControl header = GetContentControl(IsSelected, TabStripPlacement);
            if (header != null)
            {
                if (!isEnabled)
                {
                    _isMouseOver = false;
                }

                UpdateVisualState();
            }
        }

        #endregion IsEnabled

        #region IsFocused
        /// <summary>
        /// Gets a value that determines whether this element has logical focus.
        /// </summary>
        public bool IsFocused
        {
            get { return (bool)GetValue(IsFocusedProperty); }
            internal set { SetValue(IsFocusedProperty, value); }
        }

        /// <summary>
        /// Identifies the IsFocused dependency property.
        /// </summary>
        public static readonly DependencyProperty IsFocusedProperty =
            DependencyProperty.Register(
                "IsFocused",
                typeof(bool),
                typeof(TabItem),
                new PropertyMetadata(OnIsFocusedPropertyChanged));

        /// <summary>
        /// IsFocusedProperty property changed handler.
        /// </summary>
        /// <param name="d">TabItem that changed IsFocused.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param>
        private static void OnIsFocusedPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            TabItem ti = d as TabItem;
            Debug.Assert(ti != null);

            ti.OnIsFocusChanged(e);
        }

        /// <summary>
        /// Called when the IsFocused property changes.
        /// </summary>
        /// <param name="e">
        /// The data for DependencyPropertyChangedEventArgs.
        /// </param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Naming", "CA1704:IdentifiersShouldBeSpelledCorrectly", MessageId = "e", Justification = "Compat with WPF.")]
        protected virtual void OnIsFocusChanged(DependencyPropertyChangedEventArgs e)
        {
            UpdateVisualState();
        }

        #endregion IsFocused

        #region Change State

        /// <summary>
        /// Change to the correct visual state for the TabItem.
        /// </summary>
        internal void UpdateVisualState()
        {
            ChangeVisualState(true);
        }

        /// <summary>
        /// Change to the correct visual state for the TabItem.
        /// </summary>
        /// <param name="useTransitions">
        /// true to use transitions when updating the visual state, false to
        /// snap directly to the new visual state.
        /// </param>
        private void ChangeVisualState(bool useTransitions)
        {
            // Choose the appropriate TabItem template to display
            // based on which TabStripPlacement we are using and 
            // whether the item is selected.
            UpdateTabItemVisuals();

            // Update the CommonStates group
            if (!IsEnabled || (TabControlParent != null && !TabControlParent.IsEnabled))
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateDisabled, VisualStates.StateNormal);
            }
            else if (_isMouseOver && !IsSelected)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateMouseOver, VisualStates.StateNormal);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateNormal);
            }

            // Update the SelectionStates group
            if (IsSelected)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateSelected, VisualStates.StateUnselected);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateUnselected);
            }

            // Update the FocusStates group
            if (IsFocused && IsEnabled)
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateFocused, VisualStates.StateUnfocused);
            }
            else
            {
                VisualStates.GoToState(this, useTransitions, VisualStates.StateUnfocused);
            }
        }

        private void UpdateTabItemVisuals()
        {
            // update the template that is displayed
            FrameworkElement currentTemplate = GetTemplate(IsSelected, TabStripPlacement);

            if (_previousTemplate != null && _previousTemplate != currentTemplate)
            {
                _previousTemplate.Visibility = Visibility.Collapsed;
            }
            _previousTemplate = currentTemplate;
            if (currentTemplate != null)
            {
                currentTemplate.Visibility = Visibility.Visible;
            }

            // update the ContentControl's header
            ContentControl currentHeader = GetContentControl(IsSelected, TabStripPlacement);

            if (_previousHeader != null && _previousHeader != currentHeader)
            {
                _previousHeader.Content = null;
            }
            _previousHeader = currentHeader;
            UpdateHeaderVisuals();
        }

        #endregion Change State

        #region Mouse Handlers

        /// <summary>
        /// Handles when the mouse leaves the control
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">MouseEventArgs.</param>
        private void OnMouseLeave(object sender, MouseEventArgs e)
        {           
            _isMouseOver = false;
            UpdateVisualState();
        }

        /// <summary>
        /// Handles when the mouse enters the control
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">MouseEventArgs.</param>
        private void OnMouseEnter(object sender, MouseEventArgs e)
        {           
            _isMouseOver = true;
            UpdateVisualState();
        }

        /// <summary>
        /// Handles the mouse left button down
        /// </summary>
        /// <param name="sender">The source of the event.</param>
        /// <param name="e">MouseButtonEventArgs.</param>
        private void OnMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if (IsEnabled && TabControlParent != null && !IsSelected)
            {
                TabControlParent.SelectedIndex = TabControlParent.Items.IndexOf(this);
            }

            if (!e.Handled)
            {
                e.Handled = Focus();
            }
        }

        #endregion Mouse Handlers

        #region TabControlParent
        /// <summary>
        /// This property is used to get a reference to the TabControl that holds this TabItem.
        /// It will step up the UI tree to find the TabControl that contains this TabItem.
        /// </summary>
        private TabControl TabControlParent
        {
            get
            {
                // We need this for when the TabControl/TabItem is not in the
                // visual tree yet.
                TabControl tabCtrl = Parent as TabControl;
                if (tabCtrl != null)
                {
                    return tabCtrl;
                }

                // Once the TabControl is added to the visual tree, the TabItem's
                // parent becomes the TabPanel, so we now have to step up the
                // visual tree to find the owning TabControl.
                DependencyObject obj = this as DependencyObject;
                while (obj != null)
                {
                    TabControl tc = obj as TabControl;
                    if (tc != null)
                        return tc;
                    obj = VisualTreeHelper.GetParent(obj) as DependencyObject;
                }
                return null;
            }
        }

        #endregion TabControlParent

        #region HelperFunctions

        internal FrameworkElement GetTemplate(bool isSelected, Dock tabPlacement)
        {
            switch (tabPlacement)
            {
                case Dock.Top:
                    return isSelected ? ElementTemplateTopSelected : ElementTemplateTopUnselected;
                case Dock.Bottom:
                    return isSelected ? ElementTemplateBottomSelected : ElementTemplateBottomUnselected;
                case Dock.Left:
                    return isSelected ? ElementTemplateLeftSelected : ElementTemplateLeftUnselected;
                case Dock.Right:
                    return isSelected ? ElementTemplateRightSelected : ElementTemplateRightUnselected;
            }
            return null;
        }

        internal ContentControl GetContentControl(bool isSelected, Dock tabPlacement)
        {
            switch (tabPlacement)
            {
                case Dock.Top:
                    return isSelected ? ElementHeaderTopSelected : ElementHeaderTopUnselected;
                case Dock.Bottom:
                    return isSelected ? ElementHeaderBottomSelected : ElementHeaderBottomUnselected;
                case Dock.Left:
                    return isSelected ? ElementHeaderLeftSelected : ElementHeaderLeftUnselected;
                case Dock.Right:
                    return isSelected ? ElementHeaderRightSelected : ElementHeaderRightUnselected;
            }
            return null;
        }

        #endregion HelperFunctions

        #region Template Parts
        /// <summary>
        /// TabStripPlacement Top Selected template.
        /// </summary>
        internal FrameworkElement ElementTemplateTopSelected { get; set; }
        internal const string ElementTemplateTopSelectedName = "TemplateTopSelected";

        /// <summary>
        /// TabStripPlacement Bottom Selected template.
        /// </summary>
        internal FrameworkElement ElementTemplateBottomSelected { get; set; }
        internal const string ElementTemplateBottomSelectedName = "TemplateBottomSelected";

        /// <summary>
        /// TabStripPlacement Left Selected template.
        /// </summary>
        internal FrameworkElement ElementTemplateLeftSelected { get; set; }
        internal const string ElementTemplateLeftSelectedName = "TemplateLeftSelected";

        /// <summary>
        /// TabStripPlacement Right Selected template.
        /// </summary>
        internal FrameworkElement ElementTemplateRightSelected { get; set; }
        internal const string ElementTemplateRightSelectedName = "TemplateRightSelected";

        /// <summary>
        /// TabStripPlacement Top Unselected template.
        /// </summary>
        internal FrameworkElement ElementTemplateTopUnselected { get; set; }
        internal const string ElementTemplateTopUnselectedName = "TemplateTopUnselected";

        /// <summary>
        /// TabStripPlacement Bottom Unselected template.
        /// </summary>
        internal FrameworkElement ElementTemplateBottomUnselected { get; set; }
        internal const string ElementTemplateBottomUnselectedName = "TemplateBottomUnselected";

        /// <summary>
        /// TabStripPlacement Left Unselected template.
        /// </summary>
        internal FrameworkElement ElementTemplateLeftUnselected { get; set; }
        internal const string ElementTemplateLeftUnselectedName = "TemplateLeftUnselected";

        /// <summary>
        /// TabStripPlacement Right Unselected template.
        /// </summary>
        internal FrameworkElement ElementTemplateRightUnselected { get; set; }
        internal const string ElementTemplateRightUnselectedName = "TemplateRightUnselected";

        /// <summary>
        /// Header of the TabStripPlacement Top Selected template.
        /// </summary>
        internal ContentControl ElementHeaderTopSelected { get; set; }
        internal const string ElementHeaderTopSelectedName = "HeaderTopSelected";

        /// <summary>
        /// Header of the TabStripPlacement Bottom Selected template.
        /// </summary>
        internal ContentControl ElementHeaderBottomSelected { get; set; }
        internal const string ElementHeaderBottomSelectedName = "HeaderBottomSelected";

        /// <summary>
        /// Header of the TabStripPlacement Left Selected template.
        /// </summary>
        internal ContentControl ElementHeaderLeftSelected { get; set; }
        internal const string ElementHeaderLeftSelectedName = "HeaderLeftSelected";

        /// <summary>
        /// Header of the TabStripPlacement Right Selected template.
        /// </summary>
        internal ContentControl ElementHeaderRightSelected { get; set; }
        internal const string ElementHeaderRightSelectedName = "HeaderRightSelected";

        /// <summary>
        /// Header of the TabStripPlacement Top Unselected template.
        /// </summary>
        internal ContentControl ElementHeaderTopUnselected { get; set; }
        internal const string ElementHeaderTopUnselectedName = "HeaderTopUnselected";

        /// <summary>
        /// Header of the TabStripPlacement Bottom Unselected template.
        /// </summary>
        internal ContentControl ElementHeaderBottomUnselected { get; set; }
        internal const string ElementHeaderBottomUnselectedName = "HeaderBottomUnselected";

        /// <summary>
        /// Header of the TabStripPlacement Left Unselected template.
        /// </summary>
        internal ContentControl ElementHeaderLeftUnselected { get; set; }
        internal const string ElementHeaderLeftUnselectedName = "HeaderLeftUnselected";

        /// <summary>
        /// Header of the TabStripPlacement Right Unselected template.
        /// </summary>
        internal ContentControl ElementHeaderRightUnselected { get; set; }
        internal const string ElementHeaderRightUnselectedName = "HeaderRightUnselected";

        #endregion Template Parts

        #region Member Variables

        private bool _isMouseOver { get; set; }
        private FrameworkElement _previousTemplate;
        private ContentControl _previousHeader;

        #endregion Member Variables
    }
}
