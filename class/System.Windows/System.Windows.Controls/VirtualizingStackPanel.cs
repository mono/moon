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
using System.Windows.Media;
using System.Windows.Controls.Primitives;

namespace System.Windows.Controls {

	public class VirtualizingStackPanel : VirtualizingPanel, IScrollInfo
	{
		TranslateTransform translate = new TranslateTransform ();
		Size viewport = new Size (0, 0);
		Size extents = new Size (0, 0);
		bool recalculate = true;
		int firstVisibleIndex;
		int visibleCount;
		
		//
		// DependencyProperties
		//
		public static readonly DependencyProperty IsVirtualizingProperty =
			DependencyProperty.RegisterAttached ("IsVirtualizing",
							     typeof (bool),
							     typeof (VirtualizingStackPanel),
							     new PropertyMetadata (false));
		
		public static readonly DependencyProperty OrientationProperty =
			DependencyProperty.Register ("Orientation",
						     typeof (Orientation),
						     typeof (VirtualizingStackPanel),
						     new PropertyMetadata (Orientation.Vertical));
		
		public static readonly DependencyProperty VirtualizationModeProperty =
			DependencyProperty.RegisterAttached ("VirtualizationMode",
							     typeof (VirtualizationMode),
							     typeof (VirtualizingStackPanel),
							     new PropertyMetadata (VirtualizationMode.Recycling));
		
		
		//
		// Attached Property Accessor Methods
		//
		public static bool GetIsVirtualizing (DependencyObject o)
		{
			if (o == null)
				throw new ArgumentNullException ("o");
			
			return (bool) o.GetValue (VirtualizingStackPanel.IsVirtualizingProperty);
		}
		
		public static VirtualizationMode GetVirtualizationMode (DependencyObject element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			
			return (VirtualizationMode) element.GetValue (VirtualizingStackPanel.VirtualizationModeProperty);
		}
		
		public static void SetVirtualizationMode (DependencyObject element, VirtualizationMode mode)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			
			element.SetValue (VirtualizingStackPanel.VirtualizationModeProperty, mode);
		}
		
		
		//
		// Property Accessors
		//
		public Orientation Orientation {
			get { return (Orientation)GetValue (VirtualizingStackPanel.OrientationProperty); }
			set { SetValue (VirtualizingStackPanel.OrientationProperty, value); }
		}
		
		
		//
		// Constructors
		//
		public VirtualizingStackPanel ()
		{
			// FIXME: need to check if Silverlight does it this way or some other way...
			RenderTransform = translate;
		}
		
		
		//
		// Method Overrides
		//
		protected override Size ArrangeOverride (Size finalSize)
		{
			Size arranged = finalSize;
			
			if (Orientation == Orientation.Vertical)
				arranged.Height = 0;
			else
				arranged.Width = 0;
			
			// Arrange our children
			foreach (UIElement child in Children) {
				Size size = child.DesiredSize;
				
				if (Orientation == Orientation.Vertical) {
					size.Width = finalSize.Width;
					
					Rect childFinal = new Rect (0, arranged.Height, size.Width, size.Height);
					
					if (childFinal.IsEmpty)
						child.Arrange (new Rect ());
					else
						child.Arrange (childFinal);
					
					arranged.Width = Math.Max (arranged.Width, size.Width);
					arranged.Height += size.Height;
				} else {
					size.Height = finalSize.Height;
					
					Rect childFinal = new Rect (arranged.Width, 0, size.Width, size.Height);
					
					if (childFinal.IsEmpty)
						child.Arrange (new Rect ());
					else
						child.Arrange (childFinal);
					
					arranged.Width += size.Width;
					arranged.Height = Math.Max (arranged.Height, size.Height);
				}
			}
			
			if (Orientation == Orientation.Vertical)
				arranged.Height = Math.Max (arranged.Height, finalSize.Height);
			else
				arranged.Width = Math.Max (arranged.Width, finalSize.Width);
			
			if (extents != arranged) {
				extents = arranged;
				
				if (ScrollOwner != null)
					ScrollOwner.InvalidateScrollInfo ();
			}
			
			if (viewport != finalSize) {
				viewport = finalSize;
				
				if (ScrollOwner != null)
					ScrollOwner.InvalidateScrollInfo ();
			}
			
			return arranged;
		}
		
		Size MeasureExtents (Size availableSize)
		{
			IItemContainerGenerator generator = ItemContainerGenerator;
			ItemsControl owner = ItemsControl.GetItemsOwner (this);
			Size measured = new Size (0, 0);
			GeneratorPosition start;
			Size childAvailable;
			
			if (owner.Items.Count == 0)
				return measured;
			
			childAvailable = new Size (double.PositiveInfinity, double.PositiveInfinity);
			
			if (Orientation == Orientation.Vertical) {
				// Vertical layout
				childAvailable.Width = availableSize.Width;
				if (!Double.IsNaN (this.Width))
					childAvailable.Width = this.Width;
				
				childAvailable.Width = Math.Min (childAvailable.Width, this.MaxWidth);
				childAvailable.Width = Math.Max (childAvailable.Width, this.MinWidth);
			} else {
				// Horizontal layout
				childAvailable.Height = availableSize.Height;
				if (!Double.IsNaN (this.Height))
					childAvailable.Height = this.Height;
				
				childAvailable.Height = Math.Min (childAvailable.Height, this.MaxHeight);
				childAvailable.Height = Math.Max (childAvailable.Height, this.MinHeight);
			}
			
			start = generator.GeneratorPositionFromIndex (0);
			
			// Iterate over all items to calculate our actual extents
			using (generator.StartAt (start, GeneratorDirection.Forward, false)) {
				for (int i = 0; i < owner.Items.Count; i++) {
					bool isNewlyRealized;
					UIElement child;
					Size size;
					
					child = generator.GenerateNext (out isNewlyRealized) as UIElement;
					child.Measure (childAvailable);
					size = child.DesiredSize;
					
					if (Orientation == Orientation.Vertical) {
						measured.Height += size.Height;
						measured.Width = Math.Max (measured.Width, size.Width);
					} else {
						measured.Width += size.Width;
						measured.Height = Math.Max (measured.Height, size.Height);
					}
					
					// FIXME: presumably we need to unrealize some items?
				}
			}
			
			return measured;
		}
		
		void UpdateScrollInfo (Size availableSize)
		{
			if (recalculate || availableSize != viewport) {
				Size measured = MeasureExtents (availableSize);
				bool invalidate = false;
				
				recalculate = false;
				
				if (extents != measured) {
					extents = measured;
					invalidate = true;
				}
				
				if (viewport != availableSize) {
					viewport = availableSize;
					invalidate = true;
				}
				
				if (ScrollOwner != null && invalidate)
					ScrollOwner.InvalidateScrollInfo ();
			}
		}
		
		protected override Size MeasureOverride (Size availableSize)
		{
			Size childAvailable = new Size (double.PositiveInfinity, double.PositiveInfinity);
			Size measured = new Size (0, 0);
			
			if (Orientation == Orientation.Vertical) {
				// Vertical layout
				childAvailable.Width = availableSize.Width;
				if (!Double.IsNaN (this.Width))
					childAvailable.Width = this.Width;
				
				childAvailable.Width = Math.Min (childAvailable.Width, this.MaxWidth);
				childAvailable.Width = Math.Max (childAvailable.Width, this.MinWidth);
			} else {
				// Horizontal layout
				childAvailable.Height = availableSize.Height;
				if (!Double.IsNaN (this.Height))
					childAvailable.Height = this.Height;
				
				childAvailable.Height = Math.Min (childAvailable.Height, this.MaxHeight);
				childAvailable.Height = Math.Max (childAvailable.Height, this.MinHeight);
			}
			
			// Measure our children to get our extents
			foreach (UIElement child in Children) {
				child.Measure (childAvailable);
				Size size = child.DesiredSize;
				
				if (Orientation == Orientation.Vertical) {
					measured.Height += size.Height;
					measured.Width = Math.Max (measured.Width, size.Width);
				} else {
					measured.Width += size.Width;
					measured.Height = Math.Max (measured.Height, size.Height);
				}
			}
			
			if (extents != measured) {
				extents = measured;
				
				if (ScrollOwner != null)
					ScrollOwner.InvalidateScrollInfo ();
			}
			
			if (viewport != availableSize) {
				viewport = availableSize;
				
				if (ScrollOwner != null)
					ScrollOwner.InvalidateScrollInfo ();
			}
			
			return availableSize;
		}
		
		// FIXME: anything else this should do?
		protected override void OnClearChildren ()
		{
			base.OnClearChildren ();
			
			extents = new Size (0, 0);
			HorizontalOffset = 0;
			VerticalOffset = 0;
			
			recalculate = true;
			firstVisibleIndex = 0;
			visibleCount = 0;
			
			if (ScrollOwner != null)
				ScrollOwner.InvalidateScrollInfo ();
		}
		
		[MonoTODO]
 		protected override void OnItemsChanged (object sender, ItemsChangedEventArgs args)
 		{
 		}
		
		
		//
		// Methods
		//
		[MonoTODO]
		protected virtual void OnCleanUpVirtualizedItem (CleanUpVirtualizedItemEventArgs e)
		{
		}
		
#region "IScrollInfo"
		public bool CanHorizontallyScroll { get; set; }
		public bool CanVerticallyScroll { get; set; }
		
		public double ExtentWidth {
			get { return extents.Width; }
		}
		
		public double ExtentHeight {
			get { return extents.Height; }
		}
		
		public double HorizontalOffset {
			get; private set;
		}
		
		public double VerticalOffset {
			get; private set;
		}
		
		public double ViewportWidth {
			get { return viewport.Width; }
		}
		
		public double ViewportHeight {
			get { return viewport.Height; }
		}
		
		public ScrollViewer ScrollOwner { get; set; }
		
		//
		// Note: When scrolling along the stacking orientation,
		// supposedly Silverlight will perform logical scrolling. That
		// is, to say, it will scroll by items and not pixels.
		//
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void LineDown ()
		{
			SetVerticalOffset (VerticalOffset + 1);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void LineLeft ()
		{
			SetHorizontalOffset (HorizontalOffset - 1);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void LineRight ()
		{
			SetHorizontalOffset (HorizontalOffset + 1);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void LineUp ()
		{
			SetVerticalOffset (VerticalOffset - 1);
		}
		
		[MonoTODO]
		public virtual Rect MakeVisible (UIElement visual, Rect rectangle)
		{
			throw new NotImplementedException ();
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void MouseWheelDown ()
		{
			SetVerticalOffset (VerticalOffset + ViewportHeight / 3);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void MouseWheelLeft ()
		{
			SetHorizontalOffset (HorizontalOffset - ViewportWidth / 3);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void MouseWheelRight ()
		{
			SetHorizontalOffset (HorizontalOffset + ViewportWidth / 3);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void MouseWheelUp ()
		{
			SetVerticalOffset (VerticalOffset - ViewportHeight / 3);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void PageDown ()
		{
			SetVerticalOffset (VerticalOffset + ViewportHeight);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void PageLeft ()
		{
			SetHorizontalOffset (HorizontalOffset - ViewportWidth);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void PageRight ()
		{
			SetHorizontalOffset (HorizontalOffset + ViewportWidth);
		}
		
		[MonoTODO ("Make sure we scroll by the same amount as Silverlight")]
		public virtual void PageUp ()
		{
			SetVerticalOffset (VerticalOffset - ViewportHeight);
		}
		
		public void SetHorizontalOffset (double offset)
		{
			if (offset < 0 || viewport.Width >= extents.Width)
				offset = 0;
			else if (offset + viewport.Width >= extents.Width)
				offset = extents.Width - viewport.Width;
			
			HorizontalOffset = offset;
			translate.X = -offset;
			
			if (ScrollOwner != null)
				ScrollOwner.InvalidateScrollInfo ();
		}
		
		public void SetVerticalOffset (double offset)
		{
			if (offset < 0 || viewport.Height >= extents.Height)
				offset = 0;
			else if (offset + viewport.Height >= extents.Height)
				offset = extents.Height - viewport.Height;
			
			VerticalOffset = offset;
			translate.Y = -offset;
			
			if (ScrollOwner != null)
				ScrollOwner.InvalidateScrollInfo ();
		}
#endregion "IScrollInfo"
		
		
		//
		// Events
		//
		public event CleanUpVirtualizedItemEventHandler CleanUpVirtualizedItemEvent;
	}

}
