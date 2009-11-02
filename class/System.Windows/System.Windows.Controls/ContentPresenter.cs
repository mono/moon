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


// Copyright © Microsoft Corporation. 
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
		bool hasContent;
		internal UIElement _contentRoot;
		UIElement _fallbackRoot;

		UIElement FallbackRoot {
			get {
				if (_fallbackRoot == null)
					_fallbackRoot = ContentControl.CreateFallbackRoot ();
				return _fallbackRoot;
			}
		}
		
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
			DependencyProperty.RegisterCore(
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

			object newValue = e.NewValue;
			if (newValue is UIElement)
				source.ClearValue (ContentPresenter.DataContextProperty);
			else
				source.DataContext = newValue;

			// If the content is a UIElement, we have to clear the Template and wait for a re-render
			// Otherwise we directly update the text in our textbox.
			if (e.OldValue is UIElement || newValue is UIElement) {
				source.InvalidateMeasure ();
				source.hasContent = false;
				source.SetContentRoot (null);
			} else {
				source.PrepareContentPresenter ();
			}
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
			DependencyProperty.RegisterCore(
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

			source.InvalidateMeasure ();
			source.hasContent = false;
			source.SetContentRoot (null);
		} 
#endregion ContentTemplate

		/// <summary>
		/// Initializes a new instance of the ContentPresenter class.
		/// </summary> 
		public ContentPresenter() 
		{
		}

		internal override void InvokeLoaded (RoutedEventArgs e)
		{
			if (Content is UIElement)
				ClearValue (ContentPresenter.DataContextProperty);
			else
				DataContext = Content;
			base.InvokeLoaded (e);
		}

		internal override void ApplyTemplateHook ()
		{
			if (!hasContent)
				PrepareContentPresenter ();
			hasContent = true;
			base.ApplyTemplateHook ();
		}

		/// <summary> 
		/// Update the ContentPresenter's logical tree with the appropriate
		/// visual elements when its Content is changed.
		/// </summary> 
		private void PrepareContentPresenter() 
		{
			// Expand the ContentTemplate if it exists
			DataTemplate template = ContentTemplate; 
			object content = Content;
			if (template != null)
				content = template.LoadContent () ?? content;

			// Add the new content
			UIElement element = content as UIElement; 
			if (element == null && content != null) {
				element = FallbackRoot;
				// FIXME: Loaded event handlers are only invoked the first
				// time a UIElement is loaded. They need to be re-invoked every
				// time the element is removed from the live tree and added back
				// in. This will cause the Bindings on FrameworkElements to be
				// refreshed and remove the need for this hack. Normally Text is
				// populated using a one-way binding.
				((TextBlock) ((Grid)FallbackRoot).Children [0]).Text = content.ToString ();
			}
			
			SetContentRoot (element);
		}
		
		void SetContentRoot (UIElement newContentRoot)
		{
			if (newContentRoot == _contentRoot)
				return;
			
			if (_contentRoot != null) {
				// clear the old content
				NativeMethods.uielement_element_removed (native, _contentRoot.native);
				NativeMethods.uielement_set_subtree_object (native, IntPtr.Zero);
			}

			_contentRoot = newContentRoot;

			if (_contentRoot != null) {
				// set the new content
				NativeMethods.uielement_element_added (native, _contentRoot.native);
				NativeMethods.uielement_set_subtree_object (native, _contentRoot.native);
			}
		}
	}
} 
