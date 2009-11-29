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
using System.Windows.Input;

namespace System.Windows.Controls
{
	[TemplateVisualStateAttribute(Name="Unselected", GroupName="SelectionStates")]
	[TemplateVisualStateAttribute(Name="Focused", GroupName="FocusStates")]
	[TemplateVisualStateAttribute(Name="Normal", GroupName="CommonStates")]
	[TemplateVisualStateAttribute(Name="MouseOver", GroupName="CommonStates")]
	[TemplateVisualStateAttribute(Name="Selected", GroupName="SelectionStates")]
	[TemplateVisualStateAttribute(Name="SelectedUnfocused", GroupName="SelectionStates")]
	[TemplateVisualStateAttribute(Name="Unfocused", GroupName="FocusStates")]
	public class ComboBoxItem : ListBoxItem
	{
		public ComboBoxItem ()
		{
			
		}
		
		protected override void OnMouseLeftButtonUp (MouseButtonEventArgs e)
		{
			base.OnMouseLeftButtonUp (e);
			ComboBox box = ParentSelector as ComboBox;
			if (box != null)
				box.IsDropDownOpen = false;
		}
	}
}
