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

	public class VirtualizingStackPanel : VirtualizingPanel, IScrollInfo
	{
		public static readonly DependencyProperty IsVirtualizingProperty =
			DependencyProperty.RegisterAttached ("IsVirtualizing",
							     typeof (bool),
							     typeof (VirtualizingStackPanel),
							     new PropertyMetadata (false));

		public static bool GetIsVirtualizing (DependencyObject o)
		{
			if (o == null)
				throw new ArgumentNullException ("o");

			return (bool)o.GetValue (VirtualizingStackPanel.IsVirtualizingProperty);
		}


		public static readonly DependencyProperty OrientationProperty =
			DependencyProperty.Register ("Orientation",
						     typeof (Orientation),
						     typeof (VirtualizingStackPanel),
						     new PropertyMetadata (Orientation.Vertical));

		public Orientation Orientation {
			get { return (Orientation)GetValue (VirtualizingStackPanel.OrientationProperty); }
			set { SetValue (VirtualizingStackPanel.OrientationProperty, value); }
		}


		public static readonly DependencyProperty VirtualizationModeProperty =
			DependencyProperty.RegisterAttached ("VirtualizationMode",
							     typeof (VirtualizationMode),
							     typeof (VirtualizingStackPanel),
							     new PropertyMetadata (VirtualizationMode.Recycling));

		public static VirtualizationMode GetVirtualizationMode (DependencyObject element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");

			return (VirtualizationMode)element.GetValue (VirtualizingStackPanel.VirtualizationModeProperty);
		}





#region "IScrollInfo"
		[MonoTODO]
		public virtual void LineDown ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void LineLeft ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void LineRight ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void LineUp ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual Rect MakeVisible (UIElement visual, Rect rectangle)
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void MouseWheelDown ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void MouseWheelLeft ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void MouseWheelRight ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void MouseWheelUp ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void PageDown ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void PageLeft ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void PageRight ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public virtual void PageUp ()
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public void SetHorizontalOffset (double offset)
		{
			throw new NotImplementedException ();
		}

		[MonoTODO]
		public void SetVerticalOffset (double offset)
		{
			throw new NotImplementedException ();
		}

		public bool CanHorizontallyScroll { get; set; }
		public bool CanVerticallyScroll { get; set; }

		public double ExtentWidth { get; private set; }
		public double ExtentHeight { get; private set; }

		public double HorizontalOffset { get; private set; }
		public double VerticalOffset { get; private set; }

		public double ViewportWidth { get; private set; }
		public double ViewportHeight { get; private set; }

		public ScrollViewer ScrollOwner { get; set; }
#endregion "IScrollInfo"
		

		protected override void OnClearChildren ()
		{
		}

 		protected override void OnItemsChanged (object sender,
 						       ItemsChangedEventArgs args)
 		{
 		}



		[MonoTODO]
		protected override Size ArrangeOverride (Size arrangeSize)
		{
			return base.ArrangeOverride (arrangeSize);
		}


		[MonoTODO]
		protected override Size MeasureOverride (Size constraint)
		{
			return base.MeasureOverride (constraint);
		}



		protected virtual void OnCleanUpVirtualizedItem (CleanUpVirtualizedItemEventArgs e)
		{
		}

		public event CleanUpVirtualizedItemEventHandler CleanUpVirtualizedItemEvent;
	}

}

