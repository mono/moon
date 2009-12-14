// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Input;


namespace System.Windows.Controls
{
    internal static class KeyboardHelper
    {
        public static void GetMetaKeyState(out bool ctrl)
        {
            ctrl = (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control;
            // 
            // 
            // 
            // 
            // 
            // 
            ctrl |= (Keyboard.Modifiers & ModifierKeys.Apple) == ModifierKeys.Apple;
        }

        public static void GetMetaKeyState(out bool ctrl, out bool shift)
        {
            ctrl = (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control;
            // 
            // 
            // 
            // 
            // 
            // 
            ctrl |= (Keyboard.Modifiers & ModifierKeys.Apple) == ModifierKeys.Apple;
            shift = (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift;
        }
    }
}
