//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2009 Novell, Inc (http://www.novell.com)
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//


﻿// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using Mono;
using System;
using System.ComponentModel; 
using System.Diagnostics; 
using System.Windows.Documents;
using System.Windows.Markup; 
using System.Windows.Media;
using System.Windows.Controls;

namespace System.Windows.Controls
{
	/// <summary>
	/// Displays the content of a ContentControl. 
	/// </summary> 
	/// <remarks>
	/// Typically, you the ContentPresenter directly within the ControlTemplate 
	/// of a ContentControl to mark where the content should be displayed.
	/// Every type derived from ContentControl should have a ContentPrenter in
	/// its ControlTemplate (although it may not necessarily be a TemplatePart). 
	/// The ContentPresenter in the ControlTemplate should use a TemplateBinding
	/// to associate ContentControl.Content with ContentPresenter.Content (and
	/// an other relevant properties like FontSize, VeriticalAlignment, etc.). 
	/// </remarks> 
	[ContentProperty ("Content")]
	public class ContentPresenter : FrameworkElement
	{ 
		/// <summary> 
		/// Root element of the panel that will contain the visual content.
		/// </summary>
		/// <remarks> 
		/// Since a Control cannot change its content (i.e. it may only call 
		/// InitializeFromXaml once to set its root), every ContentPresetner
		/// will create a container as its root element regardless of the 
		/// content that will be stuffed into it.  It's also worth noting that
		/// there is no TemplatePartAttribute because the Template is write-once
		/// and cannot be changed by users.  This field is marked internal for 
		/// unit testing.
		/// </remarks>
		internal Grid _elementRoot; 

		/// <summary> 
		/// Visual representation of the Content currently being displayed.
		/// </summary>
		/// <remarks>This field is marked internal for unit testing.</remarks> 
		internal UIElement _elementContent;

		/// <summary> 
		/// Visual representation of any text Content currently being displayed. 
		/// </summary>
		/// <remarks> 
		/// The _elementText property is necessary in addition to the
		/// _elementContent property because of all of all the font related
		/// properties whose values need to be propogated into it by the 
		/// ContentPresenter.  This field is marked internal for unit testing.
		/// </remarks>
		internal TextBlock _elementText; 

#region Content
		/// <summary> 
		/// Gets or sets the data used to generate the contentPresenter elements of a 
		/// ContentPresenter.
		/// </summary> 
		public object Content
		{
			get { return GetValue(ContentProperty); } 
			set { SetValue(ContentProperty, value); }
		}
 
		/// <summary> 
		/// Identifies the Content dependency property.
		/// </summary> 
		public static readonly DependencyProperty ContentProperty =
			DependencyProperty.Register(
						    "Content", 
						    typeof(object),
						    typeof(ContentPresenter),
						    new PropertyMetadata(OnContentPropertyChanged)); 
 
		/// <summary>
		/// ContentProperty property changed handler. 
		/// </summary>
		/// <param name="d">ContentPresenter that changed its Content.</param>
		/// <param name="e">DependencyPropertyChangedEventArgs.</param> 
		private static void OnContentPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			ContentPresenter source = d as ContentPresenter; 
			Debug.Assert(source != null, 
				     "The source is not an instance of ContentPresenter!");
 
			// Use the Content as the DataContext to enable bindings in
			// ContentTemplate
			if (source.ContentTemplate != null) {
				source.DataContext = e.NewValue;
			} 
 
			// Display the Content
			source.PrepareContentPresenter(); 
		}
#endregion Content
 
#region ContentTemplate
		/// <summary>
		/// Gets or sets the data template used to display the content of the 
		/// ContentPresenter. 
		/// </summary>
		public DataTemplate ContentTemplate 
		{
			get { return GetValue(ContentTemplateProperty) as DataTemplate; }
			set { SetValue(ContentTemplateProperty, value); } 
		}

		/// <summary> 
		/// Identifies the ContentTemplate dependency property. 
		/// </summary>
		public static readonly DependencyProperty ContentTemplateProperty = 
			DependencyProperty.Register(
						    "ContentTemplate",
						    typeof(DataTemplate), 
						    typeof(ContentPresenter),
						    new PropertyMetadata(OnContentTemplatePropertyChanged));
 
		/// <summary> 
		/// ContentTemplateProperty property changed handler.
		/// </summary> 
		/// <param name="d">ContentPresenter that changed its ContentTemplate.</param>
		/// <param name="e">DependencyPropertyChangedEventArgs.</param>
		private static void OnContentTemplatePropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e) 
		{
			ContentPresenter source = d as ContentPresenter;
			Debug.Assert(source != null, 
				     "The source is not an instance of ContentPresenter!"); 

			// Use the Content as the DataContext to enable bindings in 
			// ContentTemplate or clear it if we removed our template (NOTE:
			// this should use ClearValue instead when it's available).
			source.DataContext = e.NewValue != null ? source.Content : null; 

			// Display the Content
			source.PrepareContentPresenter(); 
		} 
#endregion ContentTemplate

		/// <summary>
		/// Initializes a new instance of the ContentPresenter class.
		/// </summary> 
		public ContentPresenter() 
		{
		}

		internal override void InvokeLoaded ()
		{
			PrepareContentPresenter ();

			base.InvokeLoaded ();
		}

		/// <summary> 
		/// Update the ContentPresenter's logical tree with the appropriate
		/// visual elements when its Content is changed.
		/// </summary> 
		private void PrepareContentPresenter() 
		{
			if (_elementRoot == null) {
				_elementRoot = new Grid ();
				_elementText = new TextBlock();
				_elementRoot.Children.Add (_elementText);

				NativeMethods.uielement_element_added (native, _elementRoot.native);
				NativeMethods.uielement_set_subtree_object (native, _elementRoot.native);
			}

			// Remove the old content 
			if (_elementContent != null) {
				_elementRoot.Children.Remove(_elementContent); 
				_elementContent = null; 
			}
			else {
				_elementText.Text = null;
			} 
			
			// Expand the ContentTemplate if it exists
			DataTemplate template = ContentTemplate; 
			object content = (template != null) ? 
				template.LoadContent() :
				Content; 

			// Add the new content
			UIElement element = content as UIElement; 
			if (element != null) {
				_elementContent = element; 
				_elementRoot.Children.Add(_elementContent); 
				_elementText.Visibility = Visibility.Collapsed;
			} 
			else if (content != null) {
				_elementText.Visibility = Visibility.Visible; 
				//
				_elementText.Text = content.ToString();
			} 
			else  {
				_elementText.Visibility = Visibility.Collapsed; 
			}
		}
#if false
		/// <summary>
		/// Returns the specified contentPresenter element.
		/// </summary> 
		/// <param name="index">The index of the desired contentPresenter.</param> 
		/// <returns>The contentPresenter element at the specified index value.</returns>
		protected virtual DependencyObject GetVisualChild(int index) 
		{
			if (index != 0) {
				throw new ArgumentOutOfRangeException("index");
			}
 
			object content = Content; 
			return (content == null) ?
				null : 
				((content is UIElement) ?
				 _elementContent :
				 _elementText); 
		}
#endif
	}
} 
