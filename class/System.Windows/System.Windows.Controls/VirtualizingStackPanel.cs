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
using System.Collections.Specialized;

namespace System.Windows.Controls {
	public class VirtualizingStackPanel : VirtualizingPanel, IScrollInfo
	{
		static readonly double LineDelta = 14.7;
		static readonly double Wheelitude = 3;
		
		Size viewport = new Size (0, 0);
		Size extents = new Size (0, 0);
		
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
			get { return (Orientation) GetValue (VirtualizingStackPanel.OrientationProperty); }
			set { SetValue (VirtualizingStackPanel.OrientationProperty, value); }
		}
		
		
		//
		// Constructors
		//
		public VirtualizingStackPanel ()
		{
		}
		
		
		//
		// Helper Methods
		//
		void RemoveUnusedContainers (int first, int count)
		{
			IItemContainerGenerator generator = ItemContainerGenerator;
			ItemsControl owner = ItemsControl.GetItemsOwner (this);
			CleanUpVirtualizedItemEventArgs args;
			int last = first + count - 1;
			
			for (int i = Children.Count - 1; i >= 0; i--) {
				GeneratorPosition pos = new GeneratorPosition (i, 0);
				int item = generator.IndexFromGeneratorPosition (pos);
				
				if (item < first || item > last) {
					args = new CleanUpVirtualizedItemEventArgs (Children[i], owner.Items[item]);
					OnCleanUpVirtualizedItem (args);
					RemoveInternalChildRange (i, 1);
					
					if (!args.Cancel)
						generator.Remove (pos, 1);
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
			Size viewable = availableSize;
			bool invalidate = false;
			int nvisible = 0;
			int beyond = 0;
			int index;
			
			if (Orientation == Orientation.Horizontal)
				index = (int) HorizontalOffset;
			else
				index = (int) VerticalOffset;
			
			if (owner.Items.Count > 0) {
				IItemContainerGenerator generator = ItemContainerGenerator;
				GeneratorPosition start;
				Size childAvailable;
				int insertAt;
				
				// Calculate the child sizing constraints
				childAvailable = new Size (double.PositiveInfinity, double.PositiveInfinity);
				
				if (Orientation == Orientation.Vertical) {
					// Vertical layout
					if (!Double.IsNaN (this.Width))
						childAvailable.Width = this.Width;
					
					childAvailable.Width = Math.Min (childAvailable.Width, this.MaxWidth);
					childAvailable.Width = Math.Max (childAvailable.Width, this.MinWidth);
				} else {
					// Horizontal layout
					if (!Double.IsNaN (this.Height))
						childAvailable.Height = this.Height;
					
					childAvailable.Height = Math.Min (childAvailable.Height, this.MaxHeight);
					childAvailable.Height = Math.Max (childAvailable.Height, this.MinHeight);
				}
				
				// Next, prepare and measure the extents of our viewable items...
				start = generator.GeneratorPositionFromIndex (index);
				insertAt = (start.Offset == 0) ? start.Index : start.Index + 1;
				
				using (generator.StartAt (start, GeneratorDirection.Forward, true)) {
					bool isNewlyRealized;
					
					for (int i = index; i < owner.Items.Count && beyond < 2; i++, insertAt++) {
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
						
						nvisible++;
						
						if (Orientation == Orientation.Vertical) {
							measured.Width = Math.Max (measured.Width, size.Width);
							measured.Height += size.Height;
							
							if (measured.Height > availableSize.Height)
								beyond++;
						} else {
							measured.Height = Math.Max (measured.Height, size.Height);
							measured.Width += size.Width;
							
							if (measured.Width > availableSize.Width)
								beyond++;
						}
					}
				}
			}
			
			RemoveUnusedContainers (index, nvisible);
			
			// 'Fix' the extents/viewport values
			if (Orientation == Orientation.Vertical) {
				measured.Height = owner.Items.Count;
				viewable.Height = nvisible - beyond;
			} else {
				measured.Width = owner.Items.Count;
				viewable.Width = nvisible - beyond;
			}
			
			// Update our extents / viewport and invalidate our ScrollInfo if either have changed.
			if (extents != measured) {
				extents = measured;
				invalidate = true;
			}
			
			if (viewport != viewable) {
				viewport = viewable;
				invalidate = true;
			}
			
			if (invalidate && ScrollOwner != null)
				ScrollOwner.InvalidateScrollInfo ();
			
			return availableSize;
		}
		
		protected override Size ArrangeOverride (Size finalSize)
		{
			ItemsControl owner = ItemsControl.GetItemsOwner (this);
			Size arranged = finalSize;
			
			if (Orientation == Orientation.Vertical)
				arranged.Height = 0;
			else
				arranged.Width = 0;
			
			// Arrange our children
			foreach (UIElement child in Children) {
				Size size = child.DesiredSize;
				
				if (Orientation == Orientation.Vertical) {
					Rect childFinal = new Rect (-HorizontalOffset, arranged.Height, size.Width, size.Height);
					
					if (childFinal.IsEmpty)
						child.Arrange (new Rect ());
					else
						child.Arrange (childFinal);
					
					arranged.Width = Math.Max (arranged.Width, size.Width);
					arranged.Height += size.Height;
				} else {
					Rect childFinal = new Rect (arranged.Width, -VerticalOffset, size.Width, size.Height);
					
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
			
			return arranged;
		}
		
		protected override void OnClearChildren ()
		{
			base.OnClearChildren ();
			
			viewport = new Size (0, 0);
			extents = new Size (0, 0);
			HorizontalOffset = 0;
			VerticalOffset = 0;
			
			if (ScrollOwner != null)
				ScrollOwner.InvalidateScrollInfo ();
		}
		
 		protected override void OnItemsChanged (object sender, ItemsChangedEventArgs args)
 		{
			switch (args.Action) {
			case NotifyCollectionChangedAction.Remove:
			case NotifyCollectionChangedAction.Replace:
				RemoveInternalChildRange (args.Position.Index, args.ItemUICount);
				break;
			}
 		}
		
		
		//
		// Methods
		//
		protected virtual void OnCleanUpVirtualizedItem (CleanUpVirtualizedItemEventArgs e)
		{
			CleanUpVirtualizedItemEventHandler handler = CleanUpVirtualizedItemEvent;
			
			if (handler != null)
				handler (this, e);
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
		// Silverlight will perform logical scrolling. That
		// is, to say, it will scroll by items and not pixels.
		//
		
		public virtual void LineDown ()
		{
			if (Orientation == Orientation.Horizontal)
				SetVerticalOffset (VerticalOffset + LineDelta);
			else
				SetVerticalOffset (VerticalOffset + 1);
		}
		
		public virtual void LineLeft ()
		{
			if (Orientation == Orientation.Vertical)
				SetHorizontalOffset (HorizontalOffset - LineDelta);
			else
				SetHorizontalOffset (HorizontalOffset - 1);
		}
		
		public virtual void LineRight ()
		{
			if (Orientation == Orientation.Vertical)
				SetHorizontalOffset (HorizontalOffset + LineDelta);
			else
				SetHorizontalOffset (HorizontalOffset + 1);
		}
		
		public virtual void LineUp ()
		{
			if (Orientation == Orientation.Horizontal)
				SetVerticalOffset (VerticalOffset - LineDelta);
			else
				SetVerticalOffset (VerticalOffset - 1);
		}
		
		public virtual Rect MakeVisible (UIElement visual, Rect rectangle)
		{
			Rect exposed = new Rect (0, 0, 0, 0);
			
			foreach (UIElement child in Children) {
				if (child == visual) {
					if (Orientation == Orientation.Vertical) {
						if (rectangle.X != HorizontalOffset)
							SetHorizontalOffset (rectangle.X);
						
						exposed.Width = Math.Min (child.RenderSize.Width, ViewportWidth);
						exposed.Height = child.RenderSize.Height;
						exposed.X = HorizontalOffset;
					} else {
						if (rectangle.Y != VerticalOffset)
							SetVerticalOffset (rectangle.Y);
						
						exposed.Height = Math.Min (child.RenderSize.Height, ViewportHeight);
						exposed.Width = child.RenderSize.Width;
						exposed.Y = VerticalOffset;
					}
					
					return exposed;
				}
				
				if (Orientation == Orientation.Vertical)
					exposed.Y += child.RenderSize.Height;
				else
					exposed.X += child.RenderSize.Width;
			}
			
			throw new ArgumentException ("Visual is not a child of this Panel");
		}
		
		public virtual void MouseWheelDown ()
		{
			if (Orientation == Orientation.Horizontal)
				SetVerticalOffset (VerticalOffset + LineDelta * Wheelitude);
			else
				SetVerticalOffset (VerticalOffset + Wheelitude);
		}
		
		public virtual void MouseWheelLeft ()
		{
			if (Orientation == Orientation.Vertical)
				SetHorizontalOffset (HorizontalOffset - LineDelta * Wheelitude);
			else
				SetHorizontalOffset (HorizontalOffset - Wheelitude);
		}
		
		public virtual void MouseWheelRight ()
		{
			if (Orientation == Orientation.Vertical)
				SetHorizontalOffset (HorizontalOffset + LineDelta * Wheelitude);
			else
				SetHorizontalOffset (HorizontalOffset + Wheelitude);
		}
		
		public virtual void MouseWheelUp ()
		{
			if (Orientation == Orientation.Horizontal)
				SetVerticalOffset (VerticalOffset - LineDelta * Wheelitude);
			else
				SetVerticalOffset (VerticalOffset - Wheelitude);
		}
		
		public virtual void PageDown ()
		{
			SetVerticalOffset (VerticalOffset + ViewportHeight);
		}
		
		public virtual void PageLeft ()
		{
			SetHorizontalOffset (HorizontalOffset - ViewportWidth);
		}
		
		public virtual void PageRight ()
		{
			SetHorizontalOffset (HorizontalOffset + ViewportWidth);
		}
		
		public virtual void PageUp ()
		{
			SetVerticalOffset (VerticalOffset - ViewportHeight);
		}
		
		public void SetHorizontalOffset (double offset)
		{
			if (offset < 0 || ViewportWidth >= ExtentWidth)
				offset = 0;
			else if (offset + ViewportWidth >= ExtentWidth)
				offset = ExtentWidth - ViewportWidth;
			
			if (HorizontalOffset == offset)
				return;
			
			HorizontalOffset = offset;
			
			if (Orientation == Orientation.Horizontal)
				InvalidateMeasure ();
			else
				InvalidateArrange ();
			
			if (ScrollOwner != null)
				ScrollOwner.InvalidateScrollInfo ();
		}
		
		public void SetVerticalOffset (double offset)
		{
			if (offset < 0 || ViewportHeight >= ExtentHeight)
				offset = 0;
			else if (offset + ViewportHeight >= ExtentHeight)
				offset = ExtentHeight - ViewportHeight;
			
			if (VerticalOffset == offset)
				return;
			
			VerticalOffset = offset;
			
			if (Orientation == Orientation.Vertical)
				InvalidateMeasure ();
			else
				InvalidateArrange ();
			
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
