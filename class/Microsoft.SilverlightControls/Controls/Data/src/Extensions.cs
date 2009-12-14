// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Collections.Generic;
using System.Diagnostics; 
 
namespace System.Windows.Controlsb1
{ 
    internal static class Extensions
    {
        private static Dictionary<DependencyObject, Dictionary<DependencyProperty, bool>> _suspendedHandlers = new Dictionary<DependencyObject, Dictionary<DependencyProperty, bool>>(); 

        public static bool IsHandlerSuspended(this DependencyObject obj, DependencyProperty dependencyProperty)
        { 
            if (_suspendedHandlers.ContainsKey(obj)) 
            {
                return _suspendedHandlers[obj].ContainsKey(dependencyProperty); 
            }
            else
            { 
                return false;
            }
        } 
 
        public static void SetValueNoCallback(this DependencyObject obj, DependencyProperty property, object value)
        { 
            obj.SuspendHandler(property, true);
            try
            { 
                obj.SetValue(property, value);
            }
            finally 
            { 
                obj.SuspendHandler(property, false);
            } 

        }
 
        private static void SuspendHandler(this DependencyObject obj, DependencyProperty dependencyProperty, bool suspend)
        {
            if (_suspendedHandlers.ContainsKey(obj)) 
            { 
                Dictionary<DependencyProperty, bool> suspensions = _suspendedHandlers[obj];
 
                if (suspend)
                {
                    Debug.Assert(!suspensions.ContainsKey(dependencyProperty)); 
                    suspensions[dependencyProperty] = true; // true = dummy value
                }
                else 
                { 
                    Debug.Assert(suspensions.ContainsKey(dependencyProperty));
                    suspensions.Remove(dependencyProperty); 
                }
            }
            else 
            {
                Debug.Assert(suspend);
                _suspendedHandlers[obj] = new Dictionary<DependencyProperty, bool>(); 
                _suspendedHandlers[obj][dependencyProperty] = true; 
            }
        } 
    }
}
