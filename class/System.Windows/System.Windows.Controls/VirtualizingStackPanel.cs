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
		int firstVisibleIndex = -1;
		
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
		// Helper Methods
		//
		void RemoveUnusedContainers (int first, int count)
		{
			IItemContainerGenerator generator = ItemContainerGenerator;
			int last = first + count - 1;
			
			// FIXME: batch these...
			for (int i = Children.Count - 1; i >= 0; i--) {
				GeneratorPosition pos = new GeneratorPosition (i, 0);
				int itemIndex = generator.IndexFromGeneratorPosition (pos);
				
				if (itemIndex < first || itemIndex > last) {
					generator.Remove (pos, 1);
					RemoveInternalChildRange (i, 1);
				}
			}
		}
		
		
		//
		// Method Overrides
		//
		protected override Size MeasureOverride (Size availableSize)
		{
			ItemsControl owner = ItemsControl.GetItemsOwner (this);
			Size measured = new Size (0, 0);
			bool invalidate = false;
			int visible = 0;
			
			if (owner.Items.Count > 0) {
				IItemContainerGenerator generator = ItemContainerGenerator;
				GeneratorPosition start;
				Size childAvailable;
				int insertAt;
				
				// Calculate the child sizing constraints
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
				
				if (firstVisibleIndex == -1)
					firstVisibleIndex = 0;
				
				// Next, prepare and measure the extents of our viewable items...
				start = generator.GeneratorPositionFromIndex (firstVisibleIndex);
				insertAt = (start.Offset == 0) ? start.Index : start.Index + 1;
				
				using (generator.StartAt (start, GeneratorDirection.Forward, true)) {
					bool isNewlyRealized;
					
					for (int i = firstVisibleIndex; ; i++, insertAt++, visible++) {
						// Generate the child container
						UIElement child = generator.GenerateNext (out isNewlyRealized) as UIElement;
						if (isNewlyRealized) {
							// Add newly created children to the panel
							if (insertAt < Children.Count)
								InsertInternalChild (insertAt, child);
							else
								AddInternalChild (child);
							
							generator.PrepareItemContainer (child);
						}
						
						// Call Measure() on the child to both force layout and also so
						// that we can figure out when to stop adding children (e.g. when
						// we go beyond the viewable area)
						child.Measure (childAvailable);
						Size size = child.DesiredSize;
						
						if (Orientation == Orientation.Vertical) {
							measured.Width = Math.Max (measured.Width, size.Width);
							measured.Height += size.Height;
							
							if (measured.Height > availableSize.Height)
								break;
						} else {
							measured.Height = Math.Max (measured.Height, size.Height);
							measured.Width += size.Width;
							
							if (measured.Width > availableSize.Width)
								break;
						}
					}
				}
				
				// Using our measured viewable extents, guesstimate the full extents
				if (Orientation == Orientation.Vertical) {
					measured.Height = (measured.Height / visible) * owner.Items.Count;
				} else {
					measured.Width = (measured.Width / visible) * owner.Items.Count;
				}
			}
			
			RemoveUnusedContainers (firstVisibleIndex, visible);
			
			// Update our extents / viewport and invalidate our ScrollInfo if either have changed.
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
			
			return availableSize;
		}
		
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
		
		// FIXME: anything else this should do?
		protected override void OnClearChildren ()
		{
			base.OnClearChildren ();
			
			extents = new Size (0, 0);
			firstVisibleIndex = -1;
			HorizontalOffset = 0;
			VerticalOffset = 0;
			
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
