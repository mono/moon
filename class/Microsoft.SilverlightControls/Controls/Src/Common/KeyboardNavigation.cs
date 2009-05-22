// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System;
 
namespace System.Windows.Input
{
    /// <summary> 
    /// Provides logical and directional navigation between focusable objects.
    /// </summary>
    internal static class KeyboardNavigation 
    {
        #region AcceptsReturn
        /// <summary> 
        /// Gets the value of the AcceptsReturn attached property for the 
        /// specified element.
        /// </summary> 
        /// <param name="element">
        /// The element from which to read the attached property.
        /// </param> 
        /// <returns>The value of the AcceptsReturn property.</returns>
        public static bool GetAcceptsReturn(DependencyObject element)
        { 
            if (element == null) 
            {
                throw new ArgumentNullException("element"); 
            }

            return (bool) element.GetValue(AcceptsReturnProperty); 
        }

        /// <summary> 
        /// Sets the value of the AcceptsReturn attached property for the 
        /// specified element.
        /// </summary> 
        /// <param name="element">
        /// The element to write the attached property to.
        /// </param> 
        /// <param name="enabled">The property value to set.</param>
        public static void SetAcceptsReturn(DependencyObject element, bool enabled)
        { 
            if (element == null) 
            {
                throw new ArgumentNullException("element"); 
            }

            element.SetValue(AcceptsReturnProperty, enabled); 
        }

        /// <summary> 
        /// Identifies the AcceptsReturn attached property. 
        /// </summary>
        public static readonly DependencyProperty AcceptsReturnProperty = 
            DependencyProperty.RegisterAttachedCore(
                "AcceptsReturn",
                typeof(bool), 
                typeof(KeyboardNavigation),
                null);
        #endregion AcceptsReturn 
 
        /// <summary>
        /// Get the root of the object's visual tree. 
        /// </summary>
        /// <param name="d">DependencyObject.</param>
        /// <returns>Root of the object's visual tree.</returns> 
        internal static DependencyObject GetVisualRoot(DependencyObject d)
        {
            DependencyObject root = d; 
            for (;;) 
            {
                FrameworkElement element = root as FrameworkElement; 
                if (element == null)
                {
                    break; 
                }

                DependencyObject parent = element.Parent as DependencyObject; 
                if (parent == null) 
                {
                    break; 
                }

                root = parent; 
            }
            return root;
        } 
    } 
}
