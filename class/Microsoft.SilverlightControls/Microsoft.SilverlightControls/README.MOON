DIRECTORIES

* Files under Controls are the old beta1 controls. Many of them were
modified to be used, temporarily, inside Moonlight. Some are still used
since the "final" controls source code don't include them anymore.

* Files under RuntimeControls and SDKControls are the "final" controls.


SOURCE CODE DIFFERENCES

The final control source code, as published, was edited to be compilable 
outside System.Windows.dll (which makes sense for everyone but Moonlight).
This can introduce a few incompatibilities with SL2 controls.

e.g. RoutedEventArgs.OriginalSource needs to be set, but that's not 
possible outside System.Windows.dll (internal setter) so the published
controls don't do it.

    Final source code
	handler(this, new RoutedEventArgs());

    Beta1 source code
	handler(this, new RoutedEventArgs { OriginalSource = this });

Unit tests shows that the "right" (SL2 final) behaviour is identical
to beta1 (i.e. OriginalSource == this).


READ-ONLY DEPENDENCY PROPERTIES

They do not "officially" exists in Silverlight 2 (like they do in WPF).
However we need to deal with a few of them.

<quote>There are some read-only dependency properties that are part
of the Silverlight 2 API, but these rely on internal support.</quote>
Ref: http://msdn.microsoft.com/en-us/library/cc903923(VS.95).aspx

So far the following properties were found
	* ButtonBase.IsFocusedProperty
	* ButtonBase.IsMouseOverProperty
	* ButtonBase.IsPressedProperty
	* ScrollViewer.ComputedHorizontalScrollBarVisibilityProperty
	* ScrollViewer.ComputedVerticalScrollBarVisibilityProperty
	* ScrollViewer.ExtentHeightProperty
	* ScrollViewer.ExtentWidthProperty
	* ScrollViewer.HorizontalOffsetProperty
	* ScrollViewer.ScrollableHeightProperty
	* ScrollViewer.ScrollableWidthProperty
	* ScrollViewer.VerticalOffsetProperty
	* ScrollViewer.ViewportHeightProperty
	* ScrollViewer.ViewportWidthProperty
	* Slider.IsFocusedProperty
	* Thumb.IsDraggingProperty
	* Thumb.IsFocusedProperty
which all throws InvalidOperationException when SetValue is used from
use code (but obviously the runtime can change those values).

notes: 
* MediaElement has a bunch of getter only, DP-backed, properties
* Multiscale[Sub]Image.AspectRatio only have getters
