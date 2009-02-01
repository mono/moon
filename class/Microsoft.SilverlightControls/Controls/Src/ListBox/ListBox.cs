// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
using System.Collections; 
using System.Collections.Generic; 
using System.Collections.ObjectModel;
using System.Collections.Specialized; 
using System.Diagnostics;
using System.Windows;
using System.Windows.Controls; 
using System.Windows.Data;
using System.Windows.Input;
using System.Windows.Media; 
 
#if WPF
using PropertyChangedCallback = System.Windows.FrameworkPropertyMetadata; 
namespace System.Windows.Controls.Placeholders
{
    enum MouseButton { Left }; 
    //public class ItemsControl : System.Windows.Controls.ItemsControl { }
}
#endif 
 
#if WPF
namespace WPF 
#else
namespace System.Windows.Controls
#endif 
{
    /// <summary>
    /// Control that implements a list of selectable items. 
    /// </summary> 
    [TemplatePart(Name = ListBox.ElementScrollViewerName, Type = typeof(ScrollViewer))]
    public class ListBox : System.Windows.Controls.Primitives.Selector
    {
        /// <summary>
        /// Gets or sets the index of the first item in the current selection 
        /// or returns negative one (-1) if the selection is empty.
        /// </summary>
        public int SelectedIndex 
        { 
            get { return (int)GetValue(SelectedIndexProperty); }
            set { SetValue(SelectedIndexProperty, value); } 
        }

        /// <summary> 
        /// Identifies the SelectedIndex dependency property.
        /// </summary>
        public static readonly DependencyProperty SelectedIndexProperty = DependencyProperty.Register( 
                "SelectedIndex", typeof(int), typeof(ListBox), 
                new PropertyMetadata(new PropertyChangedCallback(OnSelectedIndexChanged)));
 
        /// <summary>
        /// Gets or sets the first item in the current selection or returns
        /// null if the selection is empty. 
        /// </summary>
        public object SelectedItem
        { 
            get { return (object)GetValue(SelectedItemProperty); } 
            set { SetValue(SelectedItemProperty, value); }
        } 

        /// <summary>
        /// Identifies the SelectedItem dependency property. 
        /// </summary>
        public static readonly DependencyProperty SelectedItemProperty = DependencyProperty.Register(
                "SelectedItem", typeof(object), typeof(ListBox), 
                new PropertyMetadata(new PropertyChangedCallback(OnSelectedItemChanged))); 

        /// <summary> 
        /// Gets the currently selected items.
        /// </summary>
        public IList SelectedItems 
        {
            get { return (IList)GetValue(SelectedItemsProperty); }
        } 
 
        /// <summary>
        /// Identifies the SelectedItems dependency property. 
        /// </summary>
        public static readonly DependencyProperty SelectedItemsProperty = DependencyProperty.Register(
                "SelectedItems", typeof(IList), typeof(ListBox), 
                new PropertyMetadata(new PropertyChangedCallback(OnSelectedItemsChanged)));
#if false
        /// <summary> 
        /// Gets the selection behavior for a ListBox. 
        /// </summary>
        public SelectionMode SelectionMode 
        {
            get { return (SelectionMode)GetValue(SelectionModeProperty); }
            private set { SetValue(SelectionModeProperty, value); } 
        }

        /// <summary> 
        /// Identifies the SelectionMode dependency property. 
        /// </summary>
        public static readonly DependencyProperty SelectionModeProperty = DependencyProperty.Register( 
                "SelectionMode", typeof(SelectionMode), typeof(ListBox),
                new PropertyMetadata(new PropertyChangedCallback(OnSelectionModeChanged)));
#endif
        /// <summary>
        /// Gets or sets the Style that is applied to the container element generated for each item.
        /// </summary> 
        public Style ItemContainerStyle 
        {
            get { return (Style)GetValue(ItemContainerStyleProperty); } 
            set { SetValue(ItemContainerStyleProperty, value); }
        }
 
        /// <summary>
        /// Identifies the ItemContainerStyle dependency property.
        /// </summary> 
        public static readonly DependencyProperty ItemContainerStyleProperty = DependencyProperty.Register( 
                "ItemContainerStyle", typeof(Style), typeof(ListBox),
                new PropertyMetadata(new PropertyChangedCallback(OnItemContainerStyleChanged))); 

        /// <summary>
        /// Identifies the IsSelectionActive dependency property. 
        /// </summary>
        public static readonly DependencyProperty IsSelectionActiveProperty = DependencyProperty.RegisterAttached(
            "IsSelectionActive", typeof(bool), typeof(ListBox), 
            new PropertyMetadata(new PropertyChangedCallback(OnIsSelectionActiveChanged))); 

        /// <summary> 
        /// Identifies the optional ScrollViewer element from the template.
        /// </summary>
        internal ScrollViewer ElementScrollViewer { get; set; } 
        private const string ElementScrollViewerName = "ScrollViewerElement";

        /// <summary> 
        /// Maps objects in the Items collection to the corresponding 
        /// ListBoxItem containers
        /// </summary> 
        private IDictionary<object, ListBoxItem> ObjectToListBoxItem
        {
            get 
            {
                if (null == _objectToListBoxItem)
                { 
                    _objectToListBoxItem = new Dictionary<object, ListBoxItem>(); 
                }
                return _objectToListBoxItem; 
            }
        }
        private Dictionary<object, ListBoxItem> _objectToListBoxItem; 

        /// <summary>
        /// Occurs when the selection of a ListBox changes. 
        /// </summary> 
        public event SelectionChangedEventHandler SelectionChanged;
 
        /// <summary>
        /// Set to true iff the ProcessingSelectionPropertyChange method is executing (to prevent recursion)
        /// </summary> 
        private bool _processingSelectionPropertyChange;

        /// <summary> 
        /// Tracks whether changes to read-only DependencyProperties are allowed. 
        /// </summary>
        private bool _readOnlyDependencyPropertyChangesAllowed; 

        /// <summary>
        /// Tracks the ListBoxItem that just lost focus. 
        /// </summary>
        private ListBoxItem _listBoxItemOldFocus;
 
        /// <summary> 
        /// Tracks the ListBoxItem that should get focus when the ListBox gets focus
        /// </summary> 
        /// <remarks>
        /// Helps implement WPF's KeyboardNavigation.SetTabOnceActiveElement
        /// which is not present in Silverlight. 
        /// </remarks>
        private ListBoxItem _tabOnceActiveElement;
 
        /// <summary> 
        /// Tracks whether to suppress the next "lost focus" event because it was self-caused.
        /// </summary> 
        private bool _suppressNextLostFocus;

        /// <summary> 
        /// Tracks the index of the focused element.
        /// </summary>
        private int _focusedIndex = -1; 
 
        /// <summary>
        /// Gets a value that indicates whether the keyboard focus is within the ListBox. 
        /// </summary>
        /// <param name="element">The element from which to read the attached property.</param>
        /// <returns>Value of the property, true if the keyboard focus is within the Selector.</returns> 
        public static bool GetIsSelectionActive(DependencyObject element)
        {
            if (null == element) 
            { 
                throw new ArgumentNullException("element");
            } 
            return (bool)element.GetValue(IsSelectionActiveProperty);
        }
 
        /// <summary>
        /// Sets a value that indicates whether the keyboard focus is within the ListBox.
        /// </summary> 
        /// <param name="element">The element on which to set the attached property.</param> 
        /// <param name="value">The value to set.</param>
        private static void SetIsSelectionActive(DependencyObject element, bool value) 
        {
            ListBox listBox = element as ListBox;
            Debug.Assert(null != listBox); 
            Debug.Assert(!listBox._readOnlyDependencyPropertyChangesAllowed);
            try
            { 
                listBox._readOnlyDependencyPropertyChangesAllowed = true; 
                listBox.SetValue(IsSelectionActiveProperty, value);
            } 
            finally
            {
                listBox._readOnlyDependencyPropertyChangesAllowed = false; 
            }
        }
 
        /// <summary> 
        /// Initializes a new instance of the ListBox class.
        /// </summary> 
        public ListBox()
        {
	    DefaultStyleKey = typeof (ListBox);
#if WPF 
            KeyboardNavigation.SetDirectionalNavigation(this, KeyboardNavigationMode.Contained);
            KeyboardNavigation.SetTabNavigation(this, KeyboardNavigationMode.Once);
            Focusable = true; 
#else 
            // DirectionalNavigation not supported by Silverlight
            TabNavigation = KeyboardNavigationMode.Once; 
            // Focusable not supported by Silverlight
#endif
            IsTabStop = false; 
            KeyDown += delegate(object sender, KeyEventArgs e)
            {
                OnKeyDown(e); 
            }; 
            GotFocus += delegate(object sender, RoutedEventArgs e)
            { 
                OnGotFocus(e);
            };
            LostFocus += delegate(object sender, RoutedEventArgs e) 
            {
                OnLostFocus(e);
            }; 
            ObservableCollection<object> observableCollection = new ObservableCollection<object>(); 
            observableCollection.CollectionChanged += new NotifyCollectionChangedEventHandler(OnSelectedItemsCollectionChanged);
            SetValue(SelectedItemsProperty, observableCollection); 
            SelectedIndex = -1;
//            SelectionMode = SelectionMode.Single;
            // Set default values for ScrollViewer attached properties 
            ScrollViewer.SetHorizontalScrollBarVisibility(this, ScrollBarVisibility.Auto);
            ScrollViewer.SetVerticalScrollBarVisibility(this, ScrollBarVisibility.Auto);
        } 
 
#if WPF
        public override void OnApplyTemplate() 
#else
        /// <summary>
        /// Invoked whenever application code or internal processes call 
        /// ApplyTemplate.
        /// </summary>
        public override void OnApplyTemplate() 
#endif 
        {
            base.OnApplyTemplate(); 
            ElementScrollViewer = GetTemplateChild(ElementScrollViewerName) as ScrollViewer;
            if (null != ElementScrollViewer)
            { 
                ElementScrollViewer.TemplatedParentHandlesScrolling = true;
                // Update ScrollViewer values
                ElementScrollViewer.HorizontalScrollBarVisibility = ScrollViewer.GetHorizontalScrollBarVisibility(this); 
                ElementScrollViewer.VerticalScrollBarVisibility = ScrollViewer.GetVerticalScrollBarVisibility(this); 
            }
        } 

        /// <summary>
        /// Determines if the specified item is (or is eligible to be) its own container. 
        /// </summary>
        /// <param name="item">The item to check.</param>
        /// <returns>Returns true if the item is (or is eligible to be) its own container; otherwise, false.</returns> 
        protected override bool IsItemItsOwnContainerOverride(object item) 
        {
            return (item is ListBoxItem); 
        }

        /// <summary> 
        /// Creates or identifies the element that is used to display the given item.
        /// </summary>
        /// <returns>The element that is used to display the given item.</returns> 
        protected override DependencyObject GetContainerForItemOverride() 
        {
#if WPF 
            return new ListBoxItem();
#else
            // 
            ListBoxItem listBoxItem = new ListBoxItem();
            if (null != ItemContainerStyle)
            { 
                listBoxItem.Style = ItemContainerStyle; 
            }
            return listBoxItem; 
#endif
        }
 
        /// <summary>
        /// Prepares the specified element to display the specified item.
        /// </summary> 
        /// <param name="element">Element used to display the specified item.</param> 
        /// <param name="item">Specified item.</param>
        protected override void PrepareContainerForItemOverride(DependencyObject element, object item) 
        {
            base.PrepareContainerForItemOverride(element, item);
            ListBoxItem listBoxItem = element as ListBoxItem; 
            Debug.Assert(null != listBoxItem);
            // Prepare the ListBoxItem state
            listBoxItem.ParentListBox = this; 
            // Prepare the ListBoxItem wrapper 
            bool setContent = true;
            if (listBoxItem != item) 
            {
                // If not a ListBoxItem, propagate the ListBox's ItemTemplate
                if (null != ItemTemplate) 
                {
                    // ItemsControl owns recreating containers if ItemTemplate ever changes
                    listBoxItem.ContentTemplate = ItemTemplate; 
                } 
#if !WPF
                else if (!string.IsNullOrEmpty(DisplayMemberPath)) 
                {
                    // Create a binding for displaying the DisplayMemberPath (which always renders as a string)
                    Binding binding = new Binding(DisplayMemberPath); 
                    binding.Converter = new DisplayMemberValueConverter();
                    listBoxItem.SetBinding(ContentControl.ContentProperty, binding);
                    setContent = false; 
                } 
#endif
                // Push the item into the ListBoxItem container 
                listBoxItem.Item = item;
                if (setContent)
                { 
                    listBoxItem.Content = item;
                }
                ObjectToListBoxItem[item] = listBoxItem; 
            } 
            // Apply ItemContainerStyle
            if ((null != ItemContainerStyle) && (null == listBoxItem.Style)) 
            {
                // Silverlight does not support cascading styles, so only use ItemContainerStyle
                // if a style is not already set on the ListBoxItem 
                listBoxItem.Style = ItemContainerStyle;
            }
            // If IsSelected, select the new item 
            if (listBoxItem.IsSelected) 
            {
                SelectedItem = listBoxItem.Item ?? listBoxItem; 
            }
            // If necessary, update an invalidated SelectedIndex (but do not fire SelectionChanged)
            else if (-1 != SelectedIndex) 
            {
                int trueSelectedIndex = Items.IndexOf(SelectedItem);
                if (trueSelectedIndex != SelectedIndex) 
                { 
                    try
                    { 
                        _processingSelectionPropertyChange = true;
                        SelectedIndex = Items.IndexOf(SelectedItem);
                    } 
                    finally
                    {
                        _processingSelectionPropertyChange = false; 
                    } 
                }
            } 
        }

        /// <summary> 
        /// Undoes the effects of PrepareContainerForItemOverride.
        /// </summary>
        /// <param name="element">The container element.</param> 
        /// <param name="item">The item.</param> 
        protected override void ClearContainerForItemOverride(DependencyObject element, object item)
        { 
            base.ClearContainerForItemOverride(element, item);
            ListBoxItem listBoxItem = element as ListBoxItem;
            Debug.Assert(null != listBoxItem); 
#if !WPF
            // Silverlight bug workaround
            if (null == item) 
            { 
                item = (null == listBoxItem.Item) ? listBoxItem : listBoxItem.Item;
            } 
#endif
            // If necessary, unselect the selected item that is being removed
            if ((listBoxItem.Item ?? listBoxItem) == SelectedItem) 
            {
                SelectedItem = null;
            } 
            // Clear the ListBoxItem state 
            listBoxItem.IsSelected = false;
            listBoxItem.ParentListBox = null; 
            if (listBoxItem != item)
            {
                Debug.Assert(ObjectToListBoxItem.ContainsKey(item)); 
                ObjectToListBoxItem.Remove(item);
            }
        } 
 
        /*
        /// <summary> 
        /// Invoked when the Items property changes.
        /// </summary>
        /// <param name="e">Information about the change.</param> 
        /// <remarks>Not supported by Silverlight. When this is supported,
        /// the workarounds in PrepareContainerForItemOverride and
        /// ClearContainerForItemOverride can be removed.</remarks> 
        protected override void OnItemsChanged(NotifyCollectionChangedEventArgs e) 
        {
            base.OnItemsChanged(e); 
            switch (e.Action)
            {
                case NotifyCollectionChangedAction.Add: 
                    if (e.NewStartingIndex <= SelectedIndex)
                    {
                        try 
                        { 
                            _processingSelectionPropertyChange = true;
                            SelectedIndex = Items.IndexOf(SelectedItem); 
                        }
                        finally
                        { 
                            _processingSelectionPropertyChange = false;
                        }
                    } 
                    break; 
                case NotifyCollectionChangedAction.Remove:
                    if (e.OldItems.Contains(SelectedItem)) 
                    {
                        SelectedItem = null;
                    } 
                    break;
                case NotifyCollectionChangedAction.Reset:
                    SelectedIndex = -1; 
                    break; 
            }
        } 
        */

        /// <summary> 
        /// Called when the control got focus.
        /// </summary>
        /// <param name="e">The event data.</param> 
        protected virtual void OnGotFocus(RoutedEventArgs e) 
        {
            SetIsSelectionActive(this, true); 
        }

        /// <summary> 
        /// Called when the control lost focus.
        /// </summary>
        /// <param name="e">The event data.</param> 
        protected virtual void OnLostFocus(RoutedEventArgs e) 
        {
            SetIsSelectionActive(this, false); 
            if (_suppressNextLostFocus)
            {
                // Suppressed 
                _suppressNextLostFocus = false;
            }
            else 
            { 
                // Focus is leaving the ListBox; stop tracking the previous ListBoxItem
                _listBoxItemOldFocus = null; 
            }
        }
 
        /// <summary>
        /// Selects the specified item in response to user input.
        /// </summary> 
        /// <param name="item">Item from the Items collection.</param> 
        private void SetSelectedItem(object item)
        { 
            SelectedItem = item;
            if (null != item)
            { 
                ListBoxItem listBoxItem = item as ListBoxItem;
                if (null == listBoxItem)
                { 
                    listBoxItem = ObjectToListBoxItem[item]; 
                }
                Debug.Assert(null != listBoxItem); 
#if WPF
                Keyboard.Focus(listBoxItem);
#else 
                listBoxItem.Focus();
#endif
            } 
        } 

        /// <summary> 
        /// Causes the object to scroll into view.
        /// </summary>
        /// <param name="item">Object to scroll.</param> 
        public void ScrollIntoView(object item)
        {
            if ((null != ElementScrollViewer) && Items.Contains(item)) 
            { 
                Rect itemsHostRect;
                Rect listBoxItemRect; 
                if (!IsOnCurrentPage(item, out itemsHostRect, out listBoxItemRect))
                {
                    if (IsVerticalOrientation()) 
                    {
                        // Scroll into view vertically (first make the right bound visible, then the left)
                        double verticalOffset = ElementScrollViewer.VerticalOffset; 
                        double verticalDelta = 0; 
                        if (itemsHostRect.Bottom < listBoxItemRect.Bottom)
                        { 
                            verticalDelta = listBoxItemRect.Bottom - itemsHostRect.Bottom;
                            verticalOffset += verticalDelta;
                        } 
                        if (listBoxItemRect.Top - verticalDelta < itemsHostRect.Top)
                        {
                            verticalOffset -= itemsHostRect.Top - (listBoxItemRect.Top - verticalDelta); 
                        } 
                        ElementScrollViewer.ScrollToVerticalOffset(verticalOffset);
                    } 
                    else
                    {
                        // Scroll into view horizontally (first make the bottom bound visible, then the top) 
                        double horizontalOffset = ElementScrollViewer.HorizontalOffset;
                        double horizontalDelta = 0;
                        if (itemsHostRect.Right < listBoxItemRect.Right) 
                        { 
                            horizontalDelta = listBoxItemRect.Right - itemsHostRect.Right;
                            horizontalOffset += horizontalDelta; 
                        }
                        if (listBoxItemRect.Left - horizontalDelta < itemsHostRect.Left)
                        { 
                            horizontalOffset -= itemsHostRect.Left - (listBoxItemRect.Left - horizontalDelta);
                        }
                        ElementScrollViewer.ScrollToHorizontalOffset(horizontalOffset); 
                    } 
                }
            } 
        }

        /// <summary> 
        /// Called by ListBoxItem instances when they are clicked.
        /// </summary>
        /// <param name="listBoxItem">The ListBoxItem.</param> 
        internal void NotifyListItemClicked(ListBoxItem listBoxItem) 
        {
            _tabOnceActiveElement = listBoxItem; 
            if (listBoxItem.IsSelected)
            {
                if (ModifierKeys.Control == (Keyboard.Modifiers & ModifierKeys.Control)) 
                {
                    SetSelectedItem(null);
                } 
            } 
            else
            { 
                object item = listBoxItem.Item ?? listBoxItem;
                SetSelectedItem(item);
                ScrollIntoView(item); 
            }
        }
 
        /// <summary> 
        /// Called by ListBoxItem instances when they get focus
        /// </summary> 
        /// <param name="listBoxItemNewFocus">ListBoxItem that got focus</param>
        internal void NotifyListItemGotFocus(ListBoxItem listBoxItemNewFocus)
        { 
#if !WPF
            // If _tabOnceActiveElement is not set or valid, set it to the first item
            if (((null == _tabOnceActiveElement) || 
                 (this != _tabOnceActiveElement.ParentListBox)) && 
                (1 <= Items.Count))
            { 
                _tabOnceActiveElement = GetListBoxItemForObject(Items[0]);
            }
            // If getting focus for something other than a valid _tabOnceActiveElement, pass focus to it 
            if ((null != _tabOnceActiveElement) &&
                (listBoxItemNewFocus != _tabOnceActiveElement))
            { 
                _tabOnceActiveElement.Focus(); 
                return;
            } 
#endif

            // Track the focused index 
            _focusedIndex = Items.IndexOf(listBoxItemNewFocus.Item ?? listBoxItemNewFocus);

            // Select the focused ListBoxItem iff transitioning from another focused ListBoxItem 
            if ((null != listBoxItemNewFocus) && 
                (null != _listBoxItemOldFocus) &&
                (this == _listBoxItemOldFocus.ParentListBox) && 
                (listBoxItemNewFocus != _listBoxItemOldFocus))
            {
                SelectedItem = listBoxItemNewFocus.Item ?? listBoxItemNewFocus; 
            }
            _listBoxItemOldFocus = null;
        } 
 
        /// <summary>
        /// Called by ListBoxItem instances when they lose focus 
        /// </summary>
        /// <param name="listBoxItemOldFocus">ListBoxItem that lost focus</param>
        internal void NotifyListItemLostFocus(ListBoxItem listBoxItemOldFocus) 
        {
            // Stop tracking state
            _focusedIndex = -1; 
            _listBoxItemOldFocus = listBoxItemOldFocus; 
        }
 
        /// <summary>
        /// Responds to a list box selection change by raising a SelectionChanged event.
        /// </summary> 
        /// <param name="e">Provides data for SelectionChangedEventArgs.</param>
        protected virtual void OnSelectionChanged(SelectionChangedEventArgs e)
        { 
            SelectionChangedEventHandler handler = SelectionChanged; 
            if (null != handler)
            { 
                handler(this, e);
            }
        } 

        /// <summary>
        /// Responds to the KeyDown event. 
        /// </summary> 
        /// <param name="e">Provides data for KeyEventArgs.</param>
        [System.Diagnostics.CodeAnalysis.SuppressMessage("Microsoft.Maintainability", "CA1502:AvoidExcessiveComplexity", Justification="Straightforward switch-based key handling method that barely triggers the warning")] 
        protected virtual void OnKeyDown(KeyEventArgs e)
        {
            if (!e.Handled) 
            {
                bool handled = false;
                int newFocusedIndex = -1; 
                switch (e.Key) 
                {
                    case Key.Space: 
                    case Key.Enter:
                        if ((Key.Enter != e.Key) || KeyboardNavigation.GetAcceptsReturn(this))
                        { 
                            if (ModifierKeys.Alt != (Keyboard.Modifiers & (ModifierKeys.Control | ModifierKeys.Alt)))
                            {
                                // KeyEventArgs.OriginalSource (used by WPF) isn't available in Silverlight; use FocusManager.GetFocusedElement instead 
                                ListBoxItem listBoxItem = FocusManager.GetFocusedElement( 
#if WPF
                                    FocusManager.GetFocusScope(this) 
#endif
                                    ) as ListBoxItem;
                                if (null != listBoxItem) 
                                {
                                    if ((ModifierKeys.Control == (Keyboard.Modifiers & ModifierKeys.Control)) && listBoxItem.IsSelected)
                                    { 
                                        SetSelectedItem(null); 
                                    }
                                    else 
                                    {
                                        SetSelectedItem(listBoxItem.Item ?? listBoxItem);
                                    } 
                                    handled = true;
                                }
                            } 
                        } 
                        break;
                    case Key.Home: 
                        newFocusedIndex = 0;
                        break;
                    case Key.End: 
                        newFocusedIndex = Items.Count - 1;
                        break;
                    case Key.PageUp: 
                        newFocusedIndex = NavigateByPage(false); 
                        break;
                    case Key.PageDown: 
                        newFocusedIndex = NavigateByPage(true);
                        break;
                    case Key.Left: 
                        if (IsVerticalOrientation())
                        {
                            ElementScrollViewerScrollInDirection(Key.Left); 
                        } 
                        else
                        { 
                            newFocusedIndex = _focusedIndex - 1;
                        }
                        break; 
                    case Key.Up:
                        if (IsVerticalOrientation())
                        { 
                            newFocusedIndex = _focusedIndex - 1; 
                        }
                        else 
                        {
                            ElementScrollViewerScrollInDirection(Key.Up);
                        } 
                        break;
                    case Key.Right:
                        if (IsVerticalOrientation()) 
                        { 
                            ElementScrollViewerScrollInDirection(Key.Right);
                        } 
                        else
                        {
                            newFocusedIndex = _focusedIndex + 1; 
                        }
                        break;
                    case Key.Down: 
                        if (IsVerticalOrientation()) 
                        {
                            newFocusedIndex = _focusedIndex + 1; 
                        }
                        else
                        { 
                            ElementScrollViewerScrollInDirection(Key.Down);
                        }
                        break; 
                    // case Key.Divide:  // Feature only used by SelectionMode.Extended 
                    // case Key.Oem2:  // Key not supported by Silverlight
                    //    break; 
                    // case Key.Oem5:  // Key not supported by Silverlight
                    //     break;
                    default: 
                        Debug.Assert(!handled);
                        break;
                } 
                if ((-1 != newFocusedIndex) && 
                    (-1 != _focusedIndex) &&
                    (newFocusedIndex != _focusedIndex) && 
                    (0 <= newFocusedIndex) &&
                    (newFocusedIndex < Items.Count))
                { 
                    // A key press will change the focused ListBoxItem
                    ListBoxItem listBoxItem = GetListBoxItemForObject(Items[newFocusedIndex]);
                    Debug.Assert(null != listBoxItem); 
                    _tabOnceActiveElement = listBoxItem; 
                    ScrollIntoView(listBoxItem.Item ?? listBoxItem);
                    _suppressNextLostFocus = true; 
                    listBoxItem.Focus();
                    handled = true;
                } 
                if (handled)
                {
                    e.Handled = true; 
                } 
            }
        } 

        /// <summary>
        /// Call ElementScrollViewer.ScrollInDirection if possible. 
        /// </summary>
        /// <param name="key">Key corresponding to the direction.</param>
        private void ElementScrollViewerScrollInDirection(Key key) 
        { 
            if (null != ElementScrollViewer)
            { 
                ElementScrollViewer.ScrollInDirection(key);
            }
        } 

        /// <summary>
        /// Responds to the CollectionChanged event for SelectedItems 
        /// </summary> 
        /// <param name="sender">Source of the event.</param>
        /// <param name="e">Provides data for NotifyCollectionChangedEventArgs.</param> 
        private void OnSelectedItemsCollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
        {
            if (!_processingSelectionPropertyChange) 
            {
                throw new InvalidOperationException(Resource.ListBox_OnSelectedItemsCollectionChanged_WrongMode);
            } 
        } 

        /// <summary> 
        /// Implements the SelectedIndexProperty PropertyChangedCallback.
        /// </summary>
        /// <param name="d">The DependencyObject for which the property changed.</param> 
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param>
        private static void OnSelectedIndexChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            ListBox listBox = d as ListBox; 
            Debug.Assert(null != listBox);
            Debug.Assert(typeof(int).IsInstanceOfType(e.OldValue)); 
            Debug.Assert(typeof(int).IsInstanceOfType(e.NewValue));
            listBox.OnSelectedIndexChanged((int)e.OldValue, (int)e.NewValue);
        } 

        /// <summary>
        /// Called when the SelectedIndex property has changed. 
        /// </summary> 
        /// <param name="oldValue">The value of the property before the change.</param>
        /// <param name="newValue">The value of the property after the change.</param> 
        protected virtual void OnSelectedIndexChanged(int oldValue, int newValue)
        {
            object oldValueItem = ((-1 != oldValue) && (oldValue < Items.Count)) ? Items[oldValue] : null; 
            object newValueItem = (-1 != newValue) ? Items[newValue] : null;
            ProcessSelectionPropertyChange(SelectedIndexProperty, oldValueItem, newValueItem);
        } 
 
        /// <summary>
        /// Implements the SelectedItemProperty PropertyChangedCallback. 
        /// </summary>
        /// <param name="d">The DependencyObject for which the property changed.</param>
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param> 
        private static void OnSelectedItemChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ListBox listBox = d as ListBox; 
            Debug.Assert(null != listBox); 
            listBox.OnSelectedItemChanged(e.OldValue, e.NewValue);
        } 

        /// <summary>
        /// Called when the SelectedItem property has changed. 
        /// </summary>
        /// <param name="oldValue">The value of the property before the change.</param>
        /// <param name="newValue">The value of the property after the change.</param> 
        protected virtual void OnSelectedItemChanged(object oldValue, object newValue) 
        {
            ProcessSelectionPropertyChange(SelectedItemProperty, oldValue, newValue); 
        }

        /// <summary> 
        /// Implements the SelectedItemsProperty PropertyChangedCallback.
        /// </summary>
        /// <param name="d">The DependencyObject for which the property changed.</param> 
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param> 
        private static void OnSelectedItemsChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            ListBox listBox = d as ListBox;
            Debug.Assert(null != listBox);
            Debug.Assert(typeof(IList).IsInstanceOfType(e.OldValue) || (null == e.OldValue)); 
            Debug.Assert(typeof(IList).IsInstanceOfType(e.NewValue) || (null == e.NewValue));
            listBox.OnSelectedItemsChanged((IList)e.OldValue, (IList)e.NewValue);
        } 
 
        /// <summary>
        /// Perform the actions necessary to handle a selection property change. 
        /// </summary>
        /// <param name="changedProperty">Selection property that changed.</param>
        /// <param name="oldValue">Old value of the property.</param> 
        /// <param name="newValue">New value of the property.</param>
        private void ProcessSelectionPropertyChange(DependencyProperty changedProperty, object oldValue, object newValue)
        { 
            // Avoid recursion 
            if (!_processingSelectionPropertyChange)
            { 
                try
                {
                    _processingSelectionPropertyChange = true; 

                    // Trace the removed/added items for SelectionChanged
                    List<object> removedItems = new List<object>(); 
                    List<object> addedItems = new List<object>(); 

                    // If old value present, update the associated ListBoxItem 
                    if (null != oldValue)
                    {
                        ListBoxItem oldListBoxItem = GetListBoxItemForObject(oldValue); 
                        if (null != oldListBoxItem)
                        {
                            oldListBoxItem.IsSelected = false; 
                        } 
                        removedItems.Add(oldValue);
                    } 

                    // If new value present, update the associated ListBoxItem
                    object newSelectedItem = null; 
                    int newSelectedIndex = -1;
                    if (null != newValue)
                    { 
                        ListBoxItem newListBoxItem = GetListBoxItemForObject(newValue); 
                        if (null != newListBoxItem)
                        { 
                            newListBoxItem.IsSelected = true;
                        }
                        newSelectedItem = newValue; 
                        newSelectedIndex = Items.IndexOf(newSelectedItem);
                        addedItems.Add(newValue);
                    } 
 
                    // Update the *other* associated ListBox properties
                    if (SelectedIndexProperty != changedProperty) 
                    {
                        SelectedIndex = newSelectedIndex;
                    } 
                    if (SelectedItemProperty != changedProperty)
                    {
                        SelectedItem = newSelectedItem; 
                    } 
                    SelectedItems.Clear();
                    if (-1 != newSelectedIndex) 
                    {
                        SelectedItems.Add(newValue);
                    } 

                    // Notify of SelectionChanged
                    OnSelectionChanged(new SelectionChangedEventArgs( 
#if WPF 
                        System.Windows.Controls.ListBox.SelectedEvent,
#endif 
                        removedItems, addedItems));
                }
                finally 
                {
                    _processingSelectionPropertyChange = false;
                } 
            } 
        }
 
        /// <summary>
        /// Called when the SelectedItems property has changed.
        /// </summary> 
        /// <param name="oldValue">The value of the property before the change.</param>
        /// <param name="newValue">The value of the property after the change.</param>
        protected virtual void OnSelectedItemsChanged(IList oldValue, IList newValue) 
        { 
            if (null != oldValue)
            { 
                throw new InvalidOperationException(Resource.ListBox_OnSelectedItemsChanged_ReadOnly);
            }
        } 

#if false
        /// <summary>
        /// Implements the SelectionModeProperty PropertyChangedCallback. 
        /// </summary> 
        /// <param name="d">The DependencyObject for which the property changed.</param>
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param> 
        private static void OnSelectionModeChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ListBox listBox = d as ListBox; 
            Debug.Assert(null != listBox);
            Debug.Assert(typeof(SelectionMode).IsInstanceOfType(e.OldValue));
            Debug.Assert(typeof(SelectionMode).IsInstanceOfType(e.NewValue)); 
            listBox.OnSelectionModeChanged((SelectionMode)e.OldValue, (SelectionMode)e.NewValue); 
        }
 
        /// <summary>
        /// Called when the SelectionMode property has changed.
        /// </summary> 
        /// <param name="oldValue">The value of the property before the change.</param>
        /// <param name="newValue">The value of the property after the change.</param>
        protected virtual void OnSelectionModeChanged(SelectionMode oldValue, SelectionMode newValue) 
        { 
            if (SelectionMode.Single != newValue)
            { 
                throw new ArgumentException(Resource.ListBox_OnSelectionModeChanged_OnlySingleSelection);
            }
        } 
#endif
        /// <summary>
        /// Implements the ItemContainerStyleProperty PropertyChangedCallback. 
        /// </summary> 
        /// <param name="d">The DependencyObject for which the property changed.</param>
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param> 
        private static void OnItemContainerStyleChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            ListBox listBox = d as ListBox; 
            Debug.Assert(null != listBox);
            Debug.Assert(typeof(Style).IsInstanceOfType(e.OldValue) || (null == e.OldValue));
            Debug.Assert(typeof(Style).IsInstanceOfType(e.NewValue) || (null == e.NewValue)); 
            listBox.OnItemContainerStyleChanged((Style)e.OldValue, (Style)e.NewValue); 
        }
 
        /// <summary>
        /// Called when the ItemContainerStyle property has changed.
        /// </summary> 
        /// <param name="oldItemContainerStyle">The value of the property before the change.</param>
        /// <param name="newItemContainerStyle">The value of the property after the change.</param>
        protected virtual void OnItemContainerStyleChanged(Style oldItemContainerStyle, Style newItemContainerStyle) 
        { 
            foreach (object item in Items)
            { 
                ListBoxItem listBoxItem = GetListBoxItemForObject(item);
                if (null != listBoxItem)  // May be null if GetContainerForItemOverride has not been called yet
                { 
                    if ((null == listBoxItem.Style) || (oldItemContainerStyle == listBoxItem.Style))
                    {
                        // Silverlight does not support cascading styles, so only use the new value 
                        // if it will replace the old value 
#if !WPF
                        if (null != listBoxItem.Style) 
                        {
                            throw new NotSupportedException(Resource.ListBox_OnItemContainerStyleChanged_CanNotSetStyle);
                        } 
#endif
                        listBoxItem.Style = newItemContainerStyle;
                    } 
                } 
            }
        } 

        /// <summary>
        /// Implements the IsSelectionActive PropertyChangedCallback. 
        /// </summary>
        /// <param name="d">The DependencyObject for which the property changed.</param>
        /// <param name="e">Provides data for DependencyPropertyChangedEventArgs.</param> 
        private static void OnIsSelectionActiveChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        {
            ListBox listBox = d as ListBox; 
            Debug.Assert(null != listBox);
            Debug.Assert(typeof(bool).IsInstanceOfType(e.OldValue));
            Debug.Assert(typeof(bool).IsInstanceOfType(e.NewValue)); 
            listBox.OnIsSelectionActiveChanged((bool)e.OldValue, (bool)e.NewValue);
        }
 
        /// <summary> 
        /// Called when the IsSelectionActive property has changed.
        /// </summary> 
        /// <param name="oldValue">The value of the property before the change.</param>
        /// <param name="newValue">The value of the property after the change.</param>
        protected virtual void OnIsSelectionActiveChanged(bool oldValue, bool newValue) 
        {
            if (_readOnlyDependencyPropertyChangesAllowed)
            { 
                if (null != SelectedItem) 
                {
                    ListBoxItem selectedListBoxItem = GetListBoxItemForObject(SelectedItem); 
                    if (null != selectedListBoxItem)
                    {
                        selectedListBoxItem.ChangeVisualState(); 
                    }
                }
            } 
            else 
            {
                throw new InvalidOperationException(Resource.ListBox_OnIsSelectionActiveChanged_ReadOnly); 
            }
        }
 
        /// <summary>
        /// Gets the ListBoxItem corresponding to the specified item.
        /// </summary> 
        /// <param name="value">The item being looked up.</param> 
        /// <returns>Corresponding ListBoxItem.</returns>
        private ListBoxItem GetListBoxItemForObject(object value) 
        {
            ListBoxItem selectedListBoxItem = value as ListBoxItem;
            if (null == selectedListBoxItem) 
            {
                ObjectToListBoxItem.TryGetValue(value, out selectedListBoxItem);
            } 
            return selectedListBoxItem; 
        }
 
        /// <summary>
        /// Indicate whether the orientation of the ListBox's items is vertical.
        /// </summary> 
        /// <returns>True if the orientation is vertical; false otherwise.</returns>
        private bool IsVerticalOrientation()
        {
            DependencyObject x = VisualTreeHelper.GetChild (this, 0);
            x = VisualTreeHelper.GetChild(x, 0);
            StackPanel stackPanel = x as StackPanel; 
            return (null == stackPanel) ? true : (stackPanel.Orientation == Orientation.Vertical);
        } 

        /// <summary>
        /// Indicate whether the specified item is currently visible. 
        /// </summary>
        /// <param name="item">The item.</param>
        /// <returns>True if the item is visible; false otherwise.</returns> 
        /// <remarks>Similar to WPF's corresponding ItemsControl method.</remarks> 
        private bool IsOnCurrentPage(object item)
        { 
            Rect itemsHostRect;
            Rect listBoxItemRect;
            return IsOnCurrentPage(item, out itemsHostRect, out listBoxItemRect); 
        }

        /// <summary> 
        /// Indicate whether the specified item is currently visible. 
        /// </summary>
        /// <param name="item">The item.</param> 
        /// <param name="itemsHostRect">Rect for the item host element.</param>
        /// <param name="listBoxItemRect">Rect for the ListBoxItem element.</param>
        /// <returns>True if the item is visible; false otherwise.</returns> 
        /// <remarks>Similar to WPF's corresponding ItemsControl method.</remarks>
        private bool IsOnCurrentPage(object item, out Rect itemsHostRect, out Rect listBoxItemRect)
        { 
            ListBoxItem listBoxItem = GetListBoxItemForObject(item); 
            Debug.Assert(null != listBoxItem);
            // Get Rect for item host element 
            DependencyObject ItemsHost = VisualTreeHelper.GetChild(this, 0);
            ItemsHost = VisualTreeHelper.GetChild(ItemsHost, 0);
            FrameworkElement itemsHost =
                (null != ElementScrollViewer) ?
                    ((null != ElementScrollViewer.ElementScrollContentPresenter) ? ElementScrollViewer.ElementScrollContentPresenter as FrameworkElement : ElementScrollViewer as FrameworkElement) : 
                    ItemsHost as FrameworkElement;
            Debug.Assert(null != itemsHost);
            itemsHostRect = new Rect(new Point(), new Point(itemsHost.RenderSize.Width, itemsHost.RenderSize.Height)); 
            // Adjust Rect to account for padding 
            Control itemsHostControl = itemsHost as Control;
            if (null != itemsHostControl) 
            {
                Thickness padding = itemsHostControl.Padding;
                itemsHostRect = new Rect( 
                    itemsHostRect.Left + padding.Left,
                    itemsHostRect.Top + padding.Top,
                    itemsHostRect.Width - padding.Left - padding.Right, 
                    itemsHostRect.Height - padding.Top - padding.Bottom); 
            }
            // Get relative Rect for ListBoxItem 
            GeneralTransform generalTransform = listBoxItem.TransformToVisual(itemsHost);
            listBoxItemRect = new Rect(generalTransform.Transform(new Point()), generalTransform.Transform(new Point(listBoxItem.RenderSize.Width, listBoxItem.RenderSize.Height)));
            // Return result 
            return (IsVerticalOrientation() ?
                (itemsHostRect.Top <= listBoxItemRect.Top) && (listBoxItemRect.Bottom <= itemsHostRect.Bottom) :
                (itemsHostRect.Left <= listBoxItemRect.Left) && (listBoxItemRect.Right <= itemsHostRect.Right)); 
        } 

        /// <summary> 
        /// Get the first visible item.
        /// </summary>
        /// <param name="startingIndex">Starting index to search from.</param> 
        /// <param name="forward">Search forward if true; backward otherwise.</param>
        /// <returns>Index of first visible item.</returns>
        /// <remarks>Similar to WPF's corresponding ItemsControl method.</remarks> 
        private int GetFirstItemOnCurrentPage(int startingIndex, bool forward) 
        {
            int delta = (forward ? 1 : -1); 
            int firstItemOnCurrentPage = -1;
            int probeIndex = startingIndex;
            // Scan looking for the first visible element 
            while ((0 <= probeIndex) && (probeIndex < Items.Count) && !IsOnCurrentPage(Items[probeIndex]))
            {
                firstItemOnCurrentPage = probeIndex; 
                probeIndex += delta; 
            }
            // Then scan looking for the last visible element 
            while ((0 <= probeIndex) && (probeIndex < Items.Count) && IsOnCurrentPage(Items[probeIndex]))
            {
                firstItemOnCurrentPage = probeIndex; 
                probeIndex += delta;
            }
            return firstItemOnCurrentPage; 
        } 

        /// <summary> 
        /// Move the focus forward/backward one page.
        /// </summary>
        /// <param name="forward">Forward if true; backward otherwise</param> 
        /// <returns>New focused index.</returns>
        /// <remarks>Similar to WPF's corresponding ItemsControl method.</remarks>
        private int NavigateByPage(bool forward) 
        { 
            int newFocusedIndex = -1;
            object item = (-1 != _focusedIndex) ? Items[_focusedIndex] : null; 
            // Get it visible to start with
            if ((null != item) && !IsOnCurrentPage(item))
            { 
                ScrollIntoView(item);
                if (null != ElementScrollViewer)
                { 
                    ElementScrollViewer.UpdateLayout(); 
                }
            } 
            // Inlined implementation of NavigateByPageInternal
            if (null == item)
            { 
                // Select something
                newFocusedIndex = GetFirstItemOnCurrentPage(_focusedIndex, forward);
            } 
            else 
            {
                int firstItemOnCurrentPage = GetFirstItemOnCurrentPage(_focusedIndex, forward); 
                if (firstItemOnCurrentPage != _focusedIndex)
                {
                    // Select the "edge" element 
                    newFocusedIndex = firstItemOnCurrentPage;
                }
                else 
                { 
                    if (null != ElementScrollViewer)
                    { 
                        // Scroll a page in the relevant direction
                        if (IsVerticalOrientation())
                        { 
                            ElementScrollViewer.ScrollToVerticalOffset(Math.Max(0, Math.Min(ElementScrollViewer.ScrollableHeight,
                                ElementScrollViewer.VerticalOffset + (ElementScrollViewer.ViewportHeight * (forward ? 1 : -1)))));
                        } 
                        else 
                        {
                            ElementScrollViewer.ScrollToHorizontalOffset(Math.Max(0, Math.Min(ElementScrollViewer.ScrollableWidth, 
                                ElementScrollViewer.HorizontalOffset + (ElementScrollViewer.ViewportWidth * (forward ? 1 : -1)))));
                        }
                        ElementScrollViewer.UpdateLayout(); 
                    }
                    // Select the "edge" element
                    newFocusedIndex = GetFirstItemOnCurrentPage(_focusedIndex, forward); 
                } 
            }
            return newFocusedIndex; 
        }

#if WPF 
        /// <summary>
        /// Expose WPF's internal ItemsControl.ItemsHost member for private use (it is public in Silverlight).
        /// </summary> 
        private Panel ItemsHost 
        {
            get 
            {
                System.Reflection.PropertyInfo propertyInfo = GetType().BaseType.GetProperty("ItemsHost", System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Instance);
                return propertyInfo.GetValue(this, null) as Panel; 
            }
        }
#endif 
    } 
}
