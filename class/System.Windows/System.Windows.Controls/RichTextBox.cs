//
// RichTextArea.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2010 Novell, Inc.
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
using System.Windows.Documents;

namespace System.Windows.Controls {
	[TemplatePart (Name = "ContentElement", Type = typeof (System.Windows.FrameworkElement))]
	[TemplateVisualState (Name = "MouseOver", GroupName = "CommonStates")]
	[TemplateVisualState (Name = "InvalidUnfocused", GroupName = "ValidationStates")]
	[TemplateVisualState (Name = "Focused", GroupName = "FocusStates")]
	[TemplateVisualState (Name = "Unfocused", GroupName = "FocusStates")]
	[TemplateVisualState (Name = "ReadOnly", GroupName= "CommonStates")]
	[TemplateVisualState (Name = "Valid", GroupName = "ValidationStates")]
	[TemplateVisualState (Name = "InvalidFocused", GroupName = "ValidationStates")]
	[TemplateVisualState (Name = "Disabled", GroupName = "CommonStates")]
	[TemplateVisualState (Name = "Normal", GroupName = "CommonStates")]
	public partial class RichTextBox : Control {
		protected override void OnGotFocus (RoutedEventArgs e)
		{
			base.OnGotFocus (e);
		}

		protected override void OnKeyDown (System.Windows.Input.KeyEventArgs e)
		{
			base.OnKeyDown (e);
		}

		protected override void OnKeyUp (System.Windows.Input.KeyEventArgs e)
		{
			base.OnKeyUp (e);
		}

		protected override void OnLostFocus (RoutedEventArgs e)
		{
			base.OnLostFocus (e);
		}

		protected override void OnMouseEnter (System.Windows.Input.MouseEventArgs e)
		{
			base.OnMouseEnter (e);
		}

		protected override void OnMouseLeave (System.Windows.Input.MouseEventArgs e)
		{
			base.OnMouseLeave (e);
		}

		protected override void OnMouseLeftButtonDown (System.Windows.Input.MouseButtonEventArgs e)
		{
			base.OnMouseLeftButtonDown (e);
		}

		protected override void OnMouseLeftButtonUp (System.Windows.Input.MouseButtonEventArgs e)
		{
			base.OnMouseLeftButtonUp (e);
		}

		protected override void OnMouseMove (System.Windows.Input.MouseEventArgs e)
		{
			base.OnMouseMove (e);
		}

		protected override void OnTextInput (System.Windows.Input.TextCompositionEventArgs e)
		{
			base.OnTextInput (e);
		}

		protected override void OnTextInputStart (System.Windows.Input.TextCompositionEventArgs e)
		{
			base.OnTextInputStart (e);
		}

		protected override void OnTextInputUpdate (System.Windows.Input.TextCompositionEventArgs e)
		{
			base.OnTextInputUpdate (e);
		}

		protected override void OnLostMouseCapture (System.Windows.Input.MouseEventArgs e)
		{
			base.OnLostMouseCapture (e);
		}

		protected override Automation.Peers.AutomationPeer OnCreateAutomationPeer ()
		{
			return base.OnCreateAutomationPeer ();
		}

		public void SelectAll ()
		{
			Console.WriteLine ("NIEX: System.Windows.Controls.RichTextBox:.SelectAll");
			throw new NotImplementedException ();
		}

		public TextPointer GetPositionFromPoint (Point point)
		{
			Console.WriteLine ("NIEX: System.Windows.Controls.RichTextBox:.GetPositionFromPoint");
			throw new NotImplementedException ();
		}

		public TextPointer ContentStart {
			get {
				Console.WriteLine ("NEIX: System.Windows.Controls.RichTextBox:.get_ContentStart");
				throw new NotImplementedException ();
			}
		}

		public TextPointer ContentEnd {
			get {
				Console.WriteLine ("NEIX: System.Windows.Controls.RichTextBox:.get_ContentEnd");
				throw new NotImplementedException ();
			}
		}
	}
}

