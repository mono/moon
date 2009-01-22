//
// System.Windows.Controls.TextBlock.cs
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

using System.Windows;
using Mono;

namespace System.Windows.Controls {
	public partial class ContentPresenter : FrameworkElement {
		internal const string ElementTextContentName = "TextElement";
		internal const string RootElementName = "RootElement";
		internal UIElement _elementContent;
		internal TextBlock _elementText;
		internal Grid _elementRoot;
		bool template_applied;
		
		void Initialize ()
		{
			// hook up the TemplateApplied callback so we
			// can notify controls when their template has
			// been instantiated as a visual tree.
			Events.AddHandler (this, "TemplateApplied", Events.template_applied);
		}
		
		// Since we don't inherit from Control on the managed side, we need to bind to this ourselves.
		DependencyObject GetTemplateChild (string childName)
		{
			if (childName == null)
				throw new ArgumentException ("childName");
			
			return NativeDependencyObjectHelper.FromIntPtr (NativeMethods.control_get_template_child (native, childName)) as DependencyObject;
		}
		
		public override void OnApplyTemplate ()
		{
			if (template_applied)
				throw new InvalidOperationException (Resource.ContentPresenter_OnApplyTemplate_WriteOnce);
			
			template_applied = true;
			
			// Get the content root and the text content visual
			_elementText = GetTemplateChild (ElementTextContentName) as TextBlock;
			_elementRoot = GetTemplateChild (RootElementName) as Grid;
		}
	}
}
