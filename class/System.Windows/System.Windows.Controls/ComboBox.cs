//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (c) 2008 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

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
		
		static ComboBox ()
		{
			PropertyMetadata metadata;
			metadata = new PropertyMetadata (null, delegate (DependencyObject sender, DependencyPropertyChangedEventArgs e) {
				((ComboBox) sender).IsDropDownOpenChanged (sender, e);
			});
			IsDropDownOpenProperty = DependencyProperty.Register ("IsDropDownOpen", typeof (bool), typeof (ComboBox), metadata);
			
			IsSelectionActiveProperty = DependencyProperty.Register ("IsSelectionActive", typeof (bool), typeof (ComboBox), null);
			
			ItemContainerStyleProperty = DependencyProperty.Register ("ItemContainerStyle", typeof (Style), typeof (ComboBox), null);
			
			metadata = new PropertyMetadata (double.PositiveInfinity, null);
			MaxDropDownHeightProperty = DependencyProperty.Register ("MaxDropDownHeight", typeof (double), typeof (ComboBox), metadata);
		}

		public event EventHandler DropDownClosed;
		public event EventHandler DropDownOpened;

		public bool IsDropDownOpen {
			get { return (bool) GetValue (IsDropDownOpenProperty); }
			set { SetValue (IsDropDownOpenProperty, value); }
		}
		
		public bool IsEditable {
			get; set;
		}
		
		public bool IsSelectionBoxHighlighted {
			get; private set;
		}
		
		public Style ItemContainerStyle {
			get { return (Style) GetValue (ItemContainerStyleProperty); }
		}
		
		public double MaxDropDownHeight {
			get { return (double) GetValue (MaxDropDownHeightProperty); }
			set { SetValue (MaxDropDownHeightProperty, value); }
		}
		
		public object SelectionBoxItem {
			get; private set;
		}
		
		public DataTemplate SelectionBoxItemTemplate {
			get; private set;
		}
		
		public ComboBox ()
		{
			
		}

		#region Property Changed Handlers
		
		void IsDropDownOpenChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{
			if ((bool) e.NewValue) {
				OnDropDownOpened (EventArgs.Empty);
				EventHandler h = DropDownOpened;
				if (h != null)
					h (sender, EventArgs.Empty);
			}
			else {
				OnDropDownClosed (EventArgs.Empty);
				EventHandler h = DropDownClosed;
				if (h != null)
					h (sender, EventArgs.Empty);
			}
		}
		
		void IsSelectionActiveChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{

		}
		
		void ItemContainerStyleChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{

		}
		
		void MaxDropDownHeightChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{

		}

		#endregion
		

		protected virtual void OnDropDownClosed (EventArgs e)
		{
			
		}
		
		protected virtual void OnDropDownOpened (EventArgs e)
		{
			
		}

//
//		protected override Size ArrangeOverride (Size arrangeBounds)
//		{
//			throw new NotImplementedException ();
//		}
//
//		protected override DependencyObject GetContainerForItemOverride ()
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override bool IsItemItsOwnContainerOverride (object item)
//		{
//			throw new NotImplementedException ();
//		}
//		
//		public override void OnApplyTemplate ()
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override AutomationPeer OnCreateAutomationPeer ()
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override void OnGotFocus (RoutedEventArgs e)
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override void OnKeyDown (KeyEventArgs e)
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override void OnLostFocus (RoutedEventArgs e)
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override void OnMouseEnter (MouseEventArgs e)
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override void OnMouseLeave (MouseEventArgs e)
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override void OnMouseLeftButtonDown (MouseButtonEventArgs e)
//		{
//			throw new NotImplementedException ();
//		}
//		
//		protected override void PrepareContainerForItemOverride (DependencyObject element, object item)
//		{
//			throw new NotImplementedException ();
//		}
	}
}
