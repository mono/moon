// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 





using System; 
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization; 
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media.Animation;
using System.Windows.Controls;
 
namespace System.Windows.Controls
{ 
    /// <summary>
    /// Represents a button that can be selected, but not cleared, by a user.
    /// The IsChecked property of a RadioButton can be set by clicking it, but 
    /// it can only be cleared progammatically.
    /// </summary>
    [TemplatePart(Name = RadioButton.ElementRootName, Type = typeof(FrameworkElement))] 
    [TemplatePart(Name = RadioButton.ElementFocusVisualName, Type = typeof(UIElement))] 
    [TemplatePart(Name = RadioButton.ElementContentFocusVisualName, Type = typeof(UIElement))]
    [TemplatePart(Name = RadioButton.StateCheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = RadioButton.StateNormalName, Type = typeof(Storyboard))]
    [TemplatePart(Name = RadioButton.StateMouseOverCheckedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = RadioButton.StateMouseOverUncheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = RadioButton.StatePressedCheckedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = RadioButton.StatePressedUncheckedName, Type = typeof(Storyboard))]
    [TemplatePart(Name = RadioButton.StateDisabledCheckedName, Type = typeof(Storyboard))] 
    [TemplatePart(Name = RadioButton.StateDisabledUncheckedName, Type = typeof(Storyboard))] 
    public partial class RadioButton : ToggleButton
    { 
        /// <summary>
        /// Mapping of group names to the elements in those groups.
        /// </summary> 
        private static Dictionary<string, List<WeakReference>> _groupNameToElements;

        #region GroupName 
        /// <summary> 
        /// Gets or sets mutually exclusive RadioButton groups.
        /// </summary> 
        public string GroupName
        {
            get { return GetValue(GroupNameProperty) as string; } 
            set { SetValue(GroupNameProperty, value); }
        }
 
        /// <summary> 
        /// Identifies the GroupName dependency property.
        /// </summary> 
        public static readonly DependencyProperty GroupNameProperty =
            DependencyProperty.Register(
                "GroupName", 
                typeof(string),
                typeof(RadioButton),
                new PropertyMetadata(OnGroupNamePropertyChanged)); 
 
        /// <summary>
        /// GroupNameProperty property changed handler. 
        /// </summary>
        /// <param name="d">RadioButton that changed its GroupName.</param>
        /// <param name="e">DependencyPropertyChangedEventArgs.</param> 
        private static void OnGroupNamePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            RadioButton source = d as RadioButton; 
            Debug.Assert(source != null, 
                "The source is not an instance of RadioButton!");
 
            // Unregister the old name
            Debug.Assert(typeof(string).IsInstanceOfType(e.OldValue) || (e.OldValue == null),
                "The old value is not an instance of string!"); 
            string oldValue = (string) e.OldValue;
            if (!string.IsNullOrEmpty(oldValue))
            { 
                Unregister(oldValue, source); 
            }
 
            // Register the new name
            Debug.Assert(typeof(string).IsInstanceOfType(e.NewValue) || (e.NewValue == null),
                "The value is not an instance of string!"); 
            string value = (string) e.NewValue;
            if (!string.IsNullOrEmpty(value))
            { 
                Register(value, source); 
            }
        } 
        #endregion GroupName

        /// <summary> 
        /// Initializes a new instance of the RadioButton class.
        /// </summary>
        public RadioButton() 
        { 
            // Ignore the ENTER key by default
            KeyboardNavigation.SetAcceptsReturn(this, false); 

            // Ignore ToggleButton's indeterminate states
            _ignoreIndeterminateStates = true; 
        }

        /// <summary> 
        /// Move the button to its next IsChecked value. 
        /// </summary>
        protected override void OnToggle() 
        {
            IsChecked = true;
        } 

        /// <summary>
        /// Raise the Checked event. 
        /// </summary> 
        protected override void OnChecked(RoutedEventArgs e)
        { 
            UpdateRadioButtonGroup();
            base.OnChecked(e);
        } 

        /// <summary>
        /// Update this button's entire group 
        /// </summary> 
        private void UpdateRadioButtonGroup()
        { 
            string groupName = GroupName;
            if (!string.IsNullOrEmpty(groupName))
            { 
                // Get the root of this RadioButton's visual tree
                DependencyObject visualRoot = KeyboardNavigation.GetVisualRoot(this);
 
                Debug.Assert(_groupNameToElements != null, "_groupNameToElements should already exist!"); 
                List<WeakReference> elements = _groupNameToElements[groupName];
                Debug.Assert(elements != null, string.Format(CultureInfo.InvariantCulture, 
                    "GroupName {0} not found in _groupNameToElements!", groupName));

                // Synchronize the rest of the elements in the group 
                int index = 0;
                while (index < elements.Count)
                { 
                    RadioButton button = elements[index].Target as RadioButton; 
                    if (button == null)
                    { 
                        // Remove any weak references that were collected
                        elements.RemoveAt(index);
                    } 
                    else
                    {
                        // Uncheck the button if it's in the same group and 
                        // was already checked 
                        if ((button != this) &&
                            (button.IsChecked == true) && 
                            (visualRoot == KeyboardNavigation.GetVisualRoot(button)))
                        {
                            button.IsChecked = false; 
                        }
                        index++;
                    } 
                } 
            }
            else 
            {
                // Synchronize the rest of the elements in the nameless group
                Panel parent = Parent as Panel; 
                if (parent != null)
                {
                    // 
 
                    foreach (UIElement element in parent.Children)
                    { 
                        RadioButton button = element as RadioButton;

                        // Uncheck the button if it's in the same nameless group 
                        // and was already checked
                        if ((button != null) &&
                            (button != this) && 
                            string.IsNullOrEmpty(button.GroupName) && 
                            (button.IsChecked == true))
                        { 
                            button.IsChecked = false;
                        }
                    } 
                }
            }
        } 
 
        /// <summary>
        /// Regsiter a radio button with a group. 
        /// </summary>
        /// <param name="groupName">Name of the RadioButton's group.</param>
        /// <param name="radioButton">RadioButton.</param> 
        private static void Register(string groupName, RadioButton radioButton)
        {
            Debug.Assert(!string.IsNullOrEmpty(groupName), "No groupName provided!"); 
            Debug.Assert(radioButton != null, "No radioButton provided!"); 

            if (_groupNameToElements == null) 
            {
                _groupNameToElements = new Dictionary<string, List<WeakReference>>(1);
            } 

            List<WeakReference> elements = null;
            if (!_groupNameToElements.TryGetValue(groupName, out elements) || (elements == null)) 
            { 
                elements = new List<WeakReference>(1);
                _groupNameToElements[groupName] = elements; 
            }
            else
            { 
                PurgeExpiredReferences(elements, null);
            }
 
            elements.Add(new WeakReference(radioButton)); 
        }
 
        /// <summary>
        /// Unregister a radio button with a group.
        /// </summary> 
        /// <param name="groupName">Name of the RadioButton's group.</param>
        /// <param name="radioButton">RadioButton.</param>
        private static void Unregister(string groupName, RadioButton radioButton) 
        { 
            Debug.Assert(!string.IsNullOrEmpty(groupName), "No groupName provided!");
            Debug.Assert(radioButton != null, "No radioButton provided!"); 

            if (_groupNameToElements != null)
            { 
                List<WeakReference> elements = _groupNameToElements[groupName];
                if (elements != null)
                { 
                    PurgeExpiredReferences(elements, radioButton); 
                    if (elements.Count == 0)
                    { 
                        _groupNameToElements.Remove(groupName);
                    }
                } 
            }
        }
 
        /// <summary> 
        /// Purge the list of any dead elements or any elements matching a
        /// specific element to remove. 
        /// </summary>
        /// <param name="elements">List of elements to purge.</param>
        /// <param name="elementToRemove"> 
        /// Element to purge along with any expired references.
        /// </param>
        private static void PurgeExpiredReferences(List<WeakReference> elements, object elementToRemove) 
        { 
            Debug.Assert(elements != null, "No elements provided!");
 
            int index = 0;
            while (index < elements.Count)
            { 
                object target = elements[index].Target;
                if ((target == null) || (target == elementToRemove))
                { 
                    elements.RemoveAt(index); 
                }
                else 
                {
                    index++;
                } 
            }
        }
    } 
} 
