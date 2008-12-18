DIRECTORIES

* Files under Controls are the old beta1 controls. Many of them were
modified to be used, temporarily, inside Moonlight.

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
