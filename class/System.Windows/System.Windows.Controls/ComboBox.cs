/*
 * ComboBox.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Automation.Peers;
using System.Windows.Controls.Primitives;
using System.Windows.Input;

namespace System.Windows.Controls
{
	public class ComboBox : Selector
	{
		public static readonly DependencyProperty IsDropDownOpenProperty;
		public static readonly DependencyProperty IsSelectionActiveProperty;
		public static readonly DependencyProperty ItemContainerStyleProperty;
		public static readonly DependencyProperty MaxDropDownHeightProperty;
		
		public event EventHandler DropDownClosed;
		public event EventHandler DropDownOpened;
		
		public ComboBox ()
		{
			throw new NotImplementedException ();
		}
		
		protected override Size ArrangeOverride (Size arrangeBounds)
		{
			throw new NotImplementedException ();
		}
		
		protected override DependencyObject GetContainerForItemOverride ()
		{
			throw new NotImplementedException ();
		}
		
		protected override bool IsItemItsOwnContainerOverride (object item)
		{
			throw new NotImplementedException ();
		}
		
		public override void OnApplyTemplate ()
		{
			throw new NotImplementedException ();
		}
		
		protected override AutomationPeer OnCreateAutomationPeer ()
		{
			throw new NotImplementedException ();
		}
		
		protected virtual void OnDropDownClosed (EventArgs e)
		{
			throw new NotImplementedException ();
		}
		
		protected virtual void OnDropDownOpened (EventArgs e)
		{
			throw new NotImplementedException ();
		}
		
		protected override void OnGotFocus (RoutedEventArgs e)
		{
			throw new NotImplementedException ();
		}
		
		protected override void OnKeyDown (KeyEventArgs e)
		{
			throw new NotImplementedException ();
		}
		
		protected override void OnLostFocus (RoutedEventArgs e)
		{
			throw new NotImplementedException ();
		}
		
		protected override void OnMouseEnter (MouseEventArgs e)
		{
			throw new NotImplementedException ();
		}
		
		protected override void OnMouseLeave (MouseEventArgs e)
		{
			throw new NotImplementedException ();
		}
		
		protected override void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			throw new NotImplementedException ();
		}
		
		protected override void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			throw new NotImplementedException ();
		}
		
		
		// Properties
		public bool IsDropDownOpen { get { throw new NotImplementedException (); } set { throw new NotImplementedException (); } }
		public bool IsEditable { get { throw new NotImplementedException (); } }
		public bool IsSelectionBoxHighlighted { get { throw new NotImplementedException (); } }
		public Style ItemContainerStyle { get { throw new NotImplementedException (); } set { throw new NotImplementedException (); } }
		public double MaxDropDownHeight { get { throw new NotImplementedException (); } set { throw new NotImplementedException (); } }
		public object SelectionBoxItem { get { throw new NotImplementedException (); } }
		public DataTemplate SelectionBoxItemTemplate { get { throw new NotImplementedException (); } }

	}
}
