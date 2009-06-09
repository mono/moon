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
using System.Windows.Media;

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
			DependencyProperty.RegisterCore ("IsDropDownOpen", typeof (bool), typeof (ComboBox),
						     new PropertyMetadata (null, delegate (DependencyObject sender, DependencyPropertyChangedEventArgs e) {
								     ((ComboBox) sender).IsDropDownOpenChanged (sender, e);
							     }));

		public static readonly DependencyProperty IsSelectionActiveProperty =
			DependencyProperty.RegisterCore ("IsSelectionActive", typeof (bool), typeof (ComboBox), null);

		public static readonly DependencyProperty ItemContainerStyleProperty =
			DependencyProperty.RegisterCore ("ItemContainerStyle", typeof (Style), typeof (ComboBox), null);

		public static readonly DependencyProperty MaxDropDownHeightProperty =
			DependencyProperty.RegisterCore ("MaxDropDownHeight", typeof (double), typeof (ComboBox),
						     new PropertyMetadata (double.PositiveInfinity, null));
		

		public event EventHandler DropDownClosed;
		public event EventHandler DropDownOpened;

		ComboBoxItem DisplayedItem {
			get; set;
		}
		
		public bool IsDropDownOpen {
			get { return (bool) GetValue (IsDropDownOpenProperty); }
			set { SetValue (IsDropDownOpenProperty, value); }
		}
		
		public bool IsEditable {
			get;
			internal set;
		}
		
		public bool IsSelectionBoxHighlighted {
			get { return !IsDropDownOpen && FocusManager.GetFocusedElement () == this; }
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

			Loaded += delegate { UpdateVisualState (false); UpdateDisplayedItem (); };
			SelectionChanged += delegate { UpdateDisplayedItem (); };
		}

		#region Property Changed Handlers
		
		void IsDropDownOpenChanged (DependencyObject sender, DependencyPropertyChangedEventArgs e)
		{
			if ((bool) e.NewValue)
				OnDropDownOpened (EventArgs.Empty);
			else
				OnDropDownClosed (EventArgs.Empty);

			UpdateDisplayedItem ();
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
		
		protected override void ClearContainerForItemOverride (DependencyObject element, object item)
		{
			base.ClearContainerForItemOverride (element, item);
			ListBoxItem cb = (ListBoxItem) element;
			cb.ParentSelector = null;
		}

		protected override DependencyObject GetContainerForItemOverride ()
		{
			return new ComboBoxItem ();
		}
		
		protected override bool IsItemItsOwnContainerOverride (object item)
		{
			return item is ComboBoxItem;
		}

		internal override void NotifyListItemClicked (ListBoxItem listBoxItem)
		{
			Console.WriteLine ("List item clicked");
			base.NotifyListItemClicked (listBoxItem);
			IsDropDownOpen = false;
			SelectedItem = listBoxItem.Item ?? listBoxItem;
			UpdateDisplayedItem ();
		}
		
		protected override void PrepareContainerForItemOverride (DependencyObject element, object item)
		{
			base.PrepareContainerForItemOverride (element, item);
			ListBoxItem cb = (ListBoxItem) element;
			cb.Item = item;
			cb.Style = ItemContainerStyle;
			cb.ContentTemplate = ItemTemplate;
			if (element != item)
				cb.Content = item;
			cb.ParentSelector = this;
		}

		
		public override void OnApplyTemplate ()
		{
			base.OnApplyTemplate ();
			_contentPresenter = GetTemplateChild ("ContentPresenter") as ContentPresenter;
			_popup = GetTemplateChild ("Popup") as Popup;
			_contentPresenterBorder = GetTemplateChild ("ContentPresenterBorder") as FrameworkElement;
			_dropDownToggle = GetTemplateChild ("DropDownToggle") as ToggleButton;
			
			if (_popup != null) {
				_popup.CatchClickedOutside ();
				_popup.ClickedOutside += delegate { IsDropDownOpen = false; };
			}
			if (_dropDownToggle != null) {
				_dropDownToggle.MouseLeftButtonDown += (o, e) => {
					Console.WriteLine ("Toggle toggled");
					IsDropDownOpen = true;
				};
				_dropDownToggle.Click += delegate {
						Console.WriteLine ("Clicked");
						IsDropDownOpen = true;
				};
			}
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
			IsSelectionActive = _popup == null ? false : _popup.IsOpen;
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
				IsDropDownOpen = true;
				UpdateVisualState (true);
			}
			else
				base.OnKeyDown (e);
		}

		void UpdateDisplayedItem ()
		{
			object content = null;
			Console.WriteLine ("Updating...");

			// Can't do anything with no content presenter
			if (_contentPresenter == null) {
				Console.WriteLine ("Bailing out - no presenter");
				return;
			}
			
			// Clear out any existing displayed item
			if (DisplayedItem != null) {
				Console.WriteLine ("Putting: {0} back into {1}", ItemDebugString (_contentPresenter.Content), DisplayedItem.Name);
				content = _contentPresenter.Content;
				_contentPresenter.Content = null;
				DisplayedItem.Content = content;
				DisplayedItem = null;
			}

			// If nothing is selected or popup is open bail out
			if (SelectedItem == null || IsDropDownOpen) {
				Console.WriteLine ("Bailing out: {0}/{1}", ItemDebugString (SelectedItem), IsDropDownOpen);
				return;
			}
			_contentPresenter.Content = null;
			DisplayedItem = GetContainerItem (SelectedIndex) as ComboBoxItem;
			if (DisplayedItem == null) {
				Console.WriteLine ("** ERROR **: There should be one container for each item");
				return;
			}
			Console.WriteLine ("Displaying: {0} from {1}", ItemDebugString (DisplayedItem.Content), DisplayedItem.Name);
			content = DisplayedItem.Content;
			DisplayedItem.Content = null;
			_contentPresenter.Content = content;
		}

		private string ItemDebugString (object item)
		{
			if (item == null)
				return "<NULL>";
			FrameworkElement element = item as FrameworkElement;
			if (element != null)
				return element.Name;
			return item.ToString ();
		}
		
		ContentPresenter _contentPresenter;
		Popup _popup;
		FrameworkElement _contentPresenterBorder;
		ToggleButton _dropDownToggle;

		bool isMouseOver;
		bool isFocused;
	}
}
