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
	public class TemplateBindingExpression : Expression {

		bool setsParent;
		internal Control Source;
		internal string SourcePropertyName;
		internal DependencyProperty SourceProperty;

		internal FrameworkElement Target;
		internal string TargetPropertyName;
		internal DependencyProperty TargetProperty;

		internal bool UpdatingTarget;

		internal TemplateBindingExpression ()
		{
		}

		private void PropertyChanged (IntPtr dependency_object, IntPtr propertyChangeArgs, ref MoonError error, IntPtr unused)
		{
			try {
				// Type converting doesn't happen for TemplateBindings
				UpdatingTarget = true;
				try {
					Target.SetValueImpl (TargetProperty, Value.ToObject (SourceProperty.PropertyType,
											     NativeMethods.property_changed_event_args_get_new_value (propertyChangeArgs)));
				} catch {
					Target.SetValue (TargetProperty, TargetProperty.DefaultValue);
				}
				UpdatingTarget = false;
			}
			catch (Exception ex) {
				error = new MoonError (ex);
			}
		}
		
		private UnmanagedPropertyChangeHandler change_handler;

		internal void AttachChangeHandler ()
		{
			if (change_handler == null)
				change_handler = new UnmanagedPropertyChangeHandler (PropertyChanged);

			ContentControl c = Target as ContentControl;
			if (TargetProperty == ContentControl.ContentProperty && c != null) {
				setsParent = c.ContentSetsParent;
				c.ContentSetsParent = false;
			}
			NativeMethods.dependency_object_add_property_change_handler (Source.native,
										     SourceProperty.Native,
										     change_handler,
										     IntPtr.Zero);
		}

		internal void DetachChangeHandler ()
		{
			if (change_handler == null)
				return;

			ContentControl c = Target as ContentControl;
			if (c != null)
				c.ContentSetsParent = setsParent;
			
			NativeMethods.dependency_object_remove_property_change_handler (Source.native,
											SourceProperty.Native,
											change_handler);
		}

		internal override object GetValue (DependencyProperty dp)
		{
			return MoonlightTypeConverter.ConvertObject (TargetProperty, Source.GetValue (SourceProperty), Target.GetType (), false);
		}

		internal override void Dispose ()
		{
			DetachChangeHandler ();
		}
	}
}
