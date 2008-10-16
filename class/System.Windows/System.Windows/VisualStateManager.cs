//
// VisualStateManager.cs
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

using System.Collections;
using System.Collections.ObjectModel;
using System.Windows.Controls;
using System.Windows.Markup;

namespace System.Windows {

	public class VisualStateManager : DependencyObject
	{
		private static DependencyProperty VisualStateGroupsProperty = DependencyProperty.RegisterAttached ("VisualStateGroups",
														   typeof (Collection<VisualStateGroup>),
														   typeof (VisualStateManager), null);

		public static DependencyProperty CustomVisualStateManagerProperty = DependencyProperty.RegisterAttached ("CustomVisualStateManager",
															 typeof (VisualStateManager),
															 typeof (VisualStateManager), null);

		public VisualStateManager()
		{
		}

		public static VisualStateManager GetCustomVisualStateManager (FrameworkElement obj)
		{
			if (obj == null)
				throw new ArgumentNullException ("obj");

			return (VisualStateManager)obj.GetValue (CustomVisualStateManagerProperty);
		}

		public static void SetCustomVisualStateManager (FrameworkElement obj,
								VisualStateManager value)
		{
			if (obj == null)
				throw new ArgumentNullException ("obj");

			obj.SetValue (CustomVisualStateManagerProperty, value);
		}

		public static IList GetVisualStateGroups (FrameworkElement obj)
		{
			Collection<VisualStateGroup> col = (Collection<VisualStateGroup>)obj.GetValue (VisualStateGroupsProperty);
			if (col == null) {
				col = new Collection<VisualStateGroup>();
				obj.SetValue (VisualStateGroupsProperty, col);
			}
			return col;
		}

		public static bool GoToState (Control control,
					      string stateName,
					      bool useTransitions)
		{
			Console.Error.WriteLine ("GoToState not implemented");
			return true;
		}

		protected virtual bool GoToStateCore (Control control,
						      FrameworkElement templateRoot,
						      string stateName,
						      VisualStateGroup group,
						      VisualState state,
						      bool useTransitions)
		{
			Console.Error.WriteLine ("GoToStateCore not implemented");
			return true;
		}
	}

}