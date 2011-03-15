//
// TemplateBindingExpression.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
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

using Mono;
using System.Windows.Data;
using System.Windows.Controls;

namespace System.Windows {
	public class TemplateBindingExpression : Expression, IListenPropertyChanged {

		WeakPropertyChangedListener Listener {
			get; set;
		}

		bool SetsParent {
			get; set;
		}

		internal DependencyProperty SourceProperty {
			get; set;
		}

		internal string SourcePropertyName {
			get; set;
		}

		FrameworkElement Target {
			get; set;
		}

		internal DependencyProperty TargetProperty {
			get; set;
		}

		internal TemplateBindingExpression ()
		{
		}

		void IListenPropertyChanged.OnPropertyChanged (IntPtr dependency_object, IntPtr propertyChangeArgs, ref MoonError error, IntPtr unused)
		{
			try {
				// Type converting doesn't happen for TemplateBindings
				Updating = true;
				try {
					Target.SetValue (TargetProperty, GetValue (null));
				} catch {
					Target.SetValue (TargetProperty, TargetProperty.GetDefaultValue (Target));
				}
			}
			catch (Exception ex) {
				error = new MoonError (ex);
			} finally {
				Updating = false;
			}
		}

		internal override object GetValue (DependencyProperty dp)
		{
			var source = Target.TemplateOwner;
			object value = null;
			if (source != null)
				value = source.GetValue (SourceProperty);
			return MoonlightTypeConverter.ConvertObject (TargetProperty, value, Target.GetType (), false);
		}

		internal override void OnAttached (DependencyObject element)
		{
			base.OnAttached (element);

			Target = (FrameworkElement) element;
			if (Listener != null) {
				Listener.Detach ();
				Listener = null;
			}

			ContentControl c = Target as ContentControl;
			if (TargetProperty == ContentControl.ContentProperty && c != null) {
				SetsParent = c.ContentSetsParent;
				c.ContentSetsParent = false;
			}

			// Note that Target.TemplateOwner is a weak reference - it can be GC'ed at any time
			var source = Target.TemplateOwner;
			if (source != null)
				Listener = new WeakPropertyChangedListener (source, SourceProperty, this);
		}

		internal override void OnDetached (DependencyObject element)
		{
			base.OnDetached (element);

			if (Listener == null)
				return;

			ContentControl c = Target as ContentControl;
			if (c != null)
				c.ContentSetsParent = SetsParent;

			Listener.Detach ();
			Listener = null;
			Target = null;
		}
	}
}
