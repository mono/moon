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
	[TemplateVisualState (Name="FocusedDropDown", GroupName="FocusStates")]
	[TemplateVisualState (Name="Normal", GroupName="CommonStates")]
	[TemplateVisualState (Name="MouseOver", GroupName="CommonStates")]
	[TemplateVisualState (Name="Disabled", GroupName="CommonStates")]
	[TemplateVisualState (Name="Unfocused", GroupName="FocusStates")]
	[TemplateVisualState (Name="Focused", GroupName="FocuStates")]
	[TemplatePart (Name="ContentPresenter", Type=typeof(ContentPresenter))]
	[TemplatePart (Name="Popup", Type=typeof(Popup))]
	[TemplatePart (Name="ContentPresenterBorder", Type=typeof(FrameworkElement))]
	[TemplatePart (Name="DropDownToggle", Type=typeof(ToggleButton))]
	public class ComboBox : Selector
	{
		public static readonly DependencyProperty IsDropDownOpenProperty = 
			DependencyProperty.Register ("IsDropDownOpen", typeof (bool), typeof (ComboBox),
						     new PropertyMetadata (null, delegate (DependencyObject sender, DependencyPropertyChangedEventArgs e) {
								     ((ComboBox) sender).IsDropDownOpenChanged (sender, e);
							     }));

		public static readonly DependencyProperty IsSelectionActiveProperty =
			DependencyProperty.Register ("IsSelectionActive", typeof (bool), typeof (ComboBox), null);

		public static readonly DependencyProperty ItemContainerStyleProperty =
			DependencyProperty.Register ("ItemContainerStyle", typeof (Style), typeof (ComboBox), null);

		public static readonly DependencyProperty MaxDropDownHeightProperty =
			DependencyProperty.Register ("MaxDropDownHeight", typeof (double), typeof (ComboBox),
						     new PropertyMetadata (double.PositiveInfinity, null));
		

		public event EventHandler DropDownClosed;
		public event EventHandler DropDownOpened;

		public bool IsDropDownOpen {
			get { return (bool) GetValue (IsDropDownOpenProperty); }
			set { SetValue (IsDropDownOpenProperty, value); }
		}
		
		public bool IsEditable {
			get;
			internal set;
		}
		
		public bool IsSelectionBoxHighlighted {
			get; private set;
		}
		
		public Style ItemContainerStyle {
			get { return (Style) GetValue (ItemContainerStyleProperty); }
			set { SetValue (ItemContainerStyleProperty, value); }
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
			DefaultStyleKey = typeof (ComboBox);

			Loaded += delegate { UpdateVisualState (false); };
		}

		#region Property Changed Handlers
		
		void IsDropDownOpenChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{
			if ((bool) e.NewValue)
				OnDropDownOpened (EventArgs.Empty);
			else
				OnDropDownClosed (EventArgs.Empty);

			UpdateVisualState (true);
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
		

		void UpdateVisualState (bool useTransitions)
		{
			if (!IsEnabled) {
				VisualStateManager.GoToState (this, "Disabled", useTransitions);
			}
			else if (isMouseOver) {
				VisualStateManager.GoToState (this, "MouseOver", useTransitions);
			}
			else {
				VisualStateManager.GoToState (this, "Normal", useTransitions);
			}

			if (IsDropDownOpen && IsEnabled) {
				VisualStateManager.GoToState (this, "FocusedDropDown", useTransitions);
			}
			else if (isFocused && IsEnabled) {
				VisualStateManager.GoToState (this, "Focused", useTransitions);
			}
			else {
				VisualStateManager.GoToState (this, "Unfocused", useTransitions);
			}
		}

		protected virtual void OnDropDownClosed (EventArgs e)
		{
			if (_popup != null)
				_popup.IsOpen = false;
			Focus ();
			
			EventHandler h = DropDownClosed;
			if (h != null)
				h (this, e);
		}
		
		protected virtual void OnDropDownOpened (EventArgs e)
		{
			if (_popup != null)
				_popup.IsOpen = true;
			ComboBoxItem t = SelectedItem as ComboBoxItem;
			if (t == null && Items.Count > 0)
				t = Items [0] as ComboBoxItem;
			
			if (t != null)
				t.Focus ();
			else 
				Console.WriteLine ("Nothing to focus");
			EventHandler h = DropDownOpened;
			if (h != null)
				h (this, e);
		}


		protected override Size ArrangeOverride (Size arrangeBounds)
		{
			return base.ArrangeOverride (arrangeBounds);
		}

		protected override DependencyObject GetContainerForItemOverride ()
		{
			ComboBoxItem cbItem = new ComboBoxItem ();
			if (null != ItemContainerStyle) {
				cbItem.Style = ItemContainerStyle;
			}
			return cbItem;
		}
		
		protected override bool IsItemItsOwnContainerOverride (object item)
		{
			return item is ComboBoxItem;
		}

		
		protected override void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
		}

		
		public override void OnApplyTemplate ()
		{
			base.OnApplyTemplate ();
			_contentPresenter = GetTemplateChild ("ContentPresenter") as ContentPresenter;
			_popup = GetTemplateChild ("Popup") as Popup;
			_contentPresenterBorder = GetTemplateChild ("ContentPresenterBorder") as FrameworkElement;
			_dropDownToggle = GetTemplateChild ("DropDownToggle") as ToggleButton;
		}
		
		protected override AutomationPeer OnCreateAutomationPeer ()
		{
			throw new NotImplementedException ();
		}
		
		protected override void OnGotFocus (RoutedEventArgs e)
		{
			base.OnGotFocus (e);
			isFocused = true;
			UpdateVisualState (true);
		}
		
		protected override void OnLostFocus (RoutedEventArgs e)
		{
			base.OnLostFocus (e);
			isFocused = false;
			IsSelectionActive = _popup.IsOpen;
			UpdateVisualState (true);
		}
		
		protected override void OnMouseEnter (MouseEventArgs e)
		{
			base.OnMouseEnter(e);
			isMouseOver = true;
			UpdateVisualState (true);
		}
		
		protected override void OnMouseLeave (MouseEventArgs e)
		{
			base.OnMouseLeave(e);
			isMouseOver = false;
			UpdateVisualState (true);
		}
		
		protected override void OnMouseLeftButtonDown (MouseButtonEventArgs e)
		{
			base.OnMouseLeftButtonDown (e);
			Focus ();
			IsSelectionActive = true;
			IsDropDownOpen = true;
			UpdateVisualState (true);
		}

		protected override void OnKeyDown (KeyEventArgs e)
		{
			if (e.Key == Key.Enter ||
			    e.Key == Key.Space) {
				Console.WriteLine ("open the popup here");
				IsDropDownOpen = true;
				UpdateVisualState (true);
			}
			else
				base.OnKeyDown (e);
		}
		

		ContentPresenter _contentPresenter;
		Popup _popup;
		FrameworkElement _contentPresenterBorder;
		ToggleButton _dropDownToggle;

		bool isMouseOver;
		bool isFocused;
	}
}
