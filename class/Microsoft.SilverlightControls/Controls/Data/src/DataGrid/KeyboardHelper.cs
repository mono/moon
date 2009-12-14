// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Windows.Input;
 
namespace System.Windows.Controlsb1 
{
    internal static class KeyboardHelper 
    {
        public static void GetMetaKeyState(out bool ctrl, out bool shift)
        { 
            ctrl = (Keyboard.Modifiers & ModifierKeys.Control) == ModifierKeys.Control;
            shift = (Keyboard.Modifiers & ModifierKeys.Shift) == ModifierKeys.Shift;
        } 
 
        public static void GetMetaKeyState(out bool ctrl, out bool shift, out bool alt)
        { 
            GetMetaKeyState(out ctrl, out shift);
            alt = (Keyboard.Modifiers & ModifierKeys.Alt) == ModifierKeys.Alt;
        } 
    }
}
