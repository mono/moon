//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2009 Novell, Inc.
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

using System;
using System.Windows;
using System.Windows.Controls.Primitives;

namespace System.Windows.Controls {

	public abstract class VirtualizingPanel : Panel {
		protected void AddInternalChild (UIElement child)
		{
			Console.WriteLine ("not implemented: VirtualizingPanel.AddInternalChild");
		}



		protected void InsertInternalChild (int index,
						    UIElement child)
		{
			Console.WriteLine ("not implemented: VirtualizingPanel.InsertInternalChild");
		}

		protected void RemoveInternalChildRange (int index,
							 int range)
		{
			Console.WriteLine ("not implemented: VirtualizingPanel.RemoveInternalChildRange");
		}

		protected virtual void BringIndexIntoView (int index)
		{
			Console.WriteLine ("not implemented: VirtualizingPanel.BringIndexIntoView");
		}


		protected virtual void OnClearChildren ()
		{
		}

 		protected virtual void OnItemsChanged (object sender,
 						       ItemsChangedEventArgs args)
 		{
 		}

		public IItemContainerGenerator ItemContainerGenerator { get; internal set; }
	}

}