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
	public partial class ContentPresenter : FrameworkElement {
		static readonly UnmanagedEventHandler clear_root = Events.SafeDispatcher (ClearRoot);
		internal UIElement _contentRoot;
		UIElement _fallbackRoot;
		
		UIElement FallbackRoot {
			get {
				if (_fallbackRoot == null)
					_fallbackRoot = (UIElement) ContentControl.FallbackTemplate.GetVisualTree (null);
				return _fallbackRoot;
			}
		}
		
		private new void Initialize ()
		{
			Events.AddHandler (this, EventIds.ContentPresenter_ContentPresenterClearRootEvent, clear_root);
		}

		void ClearRoot ()
		{
			if (_contentRoot != null)
				Mono.NativeMethods.uielement_element_removed (native, _contentRoot.native);
			_contentRoot = null;
		}
		
		static void ClearRoot (IntPtr target, IntPtr calldata, IntPtr closure)
		{
			ContentPresenter presenter = (ContentPresenter) NativeDependencyObjectHelper.FromIntPtr (closure);
			
			presenter.ClearRoot ();
		}
		
		internal override void InvokeLoaded ()
		{
			if (Content is UIElement)
				ClearValue (ContentPresenter.DataContextProperty);
			else
				DataContext = Content;
			base.InvokeLoaded ();
		}

		internal override UIElement GetDefaultTemplate ()
		{
			ContentControl templateOwner = TemplateOwner as ContentControl;
			if (templateOwner != null) {
				if (DependencyProperty.UnsetValue == ReadLocalValue (ContentPresenter.ContentProperty)) {
					SetTemplateBinding (ContentPresenter.ContentProperty,
							       new TemplateBindingExpression {
								       SourceProperty = ContentControl.ContentProperty,
								       TargetProperty = ContentPresenter.ContentProperty
							       });
				}

				if (DependencyProperty.UnsetValue == ReadLocalValue (ContentPresenter.ContentTemplateProperty)) {
					SetTemplateBinding (ContentPresenter.ContentTemplateProperty,
							       new TemplateBindingExpression {
								       SourceProperty = ContentControl.ContentTemplateProperty,
								       TargetProperty = ContentPresenter.ContentTemplateProperty
							       });
				}
			}

			// Expand the ContentTemplate if it exists
			DataTemplate template = ContentTemplate;
			if (template != null) {
				_contentRoot = template.LoadContent () as UIElement;
			} else {
				var content = Content;
				_contentRoot = content as UIElement;
				if (_contentRoot == null && content != null)
					_contentRoot = FallbackRoot;
			}
			return _contentRoot;
		}
	}
} 
