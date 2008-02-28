var Properties = {};

/*
Properties ["DependencyObject.Name"] = {
	parent: CodeModel.DependencyObject,
	name: "Name",
	type: "string",
	valdef: ""
};

*/

function regprop (model, pname, ptype, pval) {
	Properties [model.name + "." + pname] = {
		parent: model,
		name: pname,
		type: ptype,
		valdef: pval
	};
}

//

regprop (CodeModel.DependencyObject, "Name", "string", "");

regprop (CodeModel.Accessibility, "ActionDescription", "object", null);
regprop (CodeModel.Accessibility, "Description", "object", null);
regprop (CodeModel.Accessibility, "Title", "string", "Silverlight Content");

regprop (CodeModel.Downloader, "DownloadProgress", "number", 0);
regprop (CodeModel.Downloader, "Status", "number", 0);
regprop (CodeModel.Downloader, "StatusText", "string", "");

regprop (CodeModel.UIElement, "Canvas.Left", "number", 0);
regprop (CodeModel.UIElement, "Canvas.Top", "number", 0);
regprop (CodeModel.UIElement, "Canvas.ZIndex", "number", 0);
regprop (CodeModel.UIElement, "Clip", "object", null);
regprop (CodeModel.UIElement, "Cursor", "string", "Default");
regprop (CodeModel.UIElement, "Height", "number", 0);
regprop (CodeModel.UIElement, "IsHitTestVisible", "boolean", true);
regprop (CodeModel.UIElement, "Opacity", "number", 1);
regprop (CodeModel.UIElement, "OpacityMask", "object", null);
regprop (CodeModel.UIElement, "RenderTransform", "object", null);
regprop (CodeModel.UIElement, "RenderTransformOrigin", "exception");
regprop (CodeModel.UIElement, "Resources", "object", "ResourceDictionary");
regprop (CodeModel.UIElement, "Tag", "string", "");
regprop (CodeModel.UIElement, "Triggers", "object", "TriggerCollection");
regprop (CodeModel.UIElement, "Visibility", "string", "Visible");
regprop (CodeModel.UIElement, "Width", "number", 0);

regprop (CodeModel.Canvas, "Background", "object", null);
regprop (CodeModel.Canvas, "Children", "object", "VisualCollection");

regprop (CodeModel.KeyFrame, "KeyTime", "object", null);
regprop (CodeModel.KeyFrame, "Value", "object", null);

regprop (CodeModel.SplineColorKeyFrame, "KeySpline", "object", null);

regprop (CodeModel.SplineDoubleKeyFrame, "KeySpline", "object", null);

regprop (CodeModel.SplinePointKeyFrame, "KeySpline", "object", null);

regprop (CodeModel.Storyboard, "Children", "exception");

regprop (CodeModel.Timeline, "AutoReverse", "boolean", false);
regprop (CodeModel.Timeline, "BeginTime", "object", null);
regprop (CodeModel.Timeline, "Duration", "object", null);
regprop (CodeModel.Timeline, "FillBehavior", "string", "HoldEnd");
regprop (CodeModel.Timeline, "RepeatBehavior", "object", null);
regprop (CodeModel.Timeline, "SpeedRatio", "number", 1);
regprop (CodeModel.Timeline, "StoryBoard.TargetName", "string", "");
regprop (CodeModel.Timeline, "StoryBoard.TargetProperty", "string", "");

regprop (CodeModel.ColorAnimationUsingKeyFrames, "KeyFrames", "object", null);

regprop (CodeModel.Animation, "By", "object", null);
regprop (CodeModel.Animation, "From", "object", null);
regprop (CodeModel.Animation, "To", "object", null);

regprop (CodeModel.DoubleAnimationUsingKeyFrames, "KeyFrames", "object", "DoubleKeyFrameCollection");

regprop (CodeModel.PointAnimationUsingKeyFrames, "KeyFrames", "object", "PointKeyFrameCollection");

regprop (CodeModel.Brush, "Opacity", "number", 1);
regprop (CodeModel.Brush, "RelativeTransform", "object", null);
regprop (CodeModel.Brush, "Transform", "object", null);

regprop (CodeModel.ImageBrush, "AlignmentX", "string", "Center");
regprop (CodeModel.ImageBrush, "AlignmentY", "string", "Center");
regprop (CodeModel.ImageBrush, "DownloadProgress", "number", 0);
regprop (CodeModel.ImageBrush, "ImageSource", "string", "");
regprop (CodeModel.ImageBrush, "Stretch", "string", "Fill");

regprop (CodeModel.SolidColorBrush, "Color", "number", 0);

regprop (CodeModel.VideoBrush, "SourceName", "string", "");
regprop (CodeModel.VideoBrush, "Stretch", "string", "Fill");

regprop (CodeModel.GradientBrush, "ColorInterpolationMode", "string", "SRgbLinearInterpolation");
regprop (CodeModel.GradientBrush, "GradientStops", "object", "GradientStopCollection");
regprop (CodeModel.GradientBrush, "MappingMode", "string", "RelativeToBoundingBox");
regprop (CodeModel.GradientBrush, "SpreadMethod", "string", "Pad");

regprop (CodeModel.LinearGradientBrush, "StartPoint", "exception");
regprop (CodeModel.LinearGradientBrush, "EndPoint", "exception");

regprop (CodeModel.RadialGradientBrush, "RadiusX", "number", 0.5);
regprop (CodeModel.RadialGradientBrush, "RadiusY", "number", 0.5);
regprop (CodeModel.RadialGradientBrush, "Center", "exception");
regprop (CodeModel.RadialGradientBrush, "GradientOrigin", "exception");

regprop (CodeModel.ArcSegment, "IsLargeArc", "boolean", false);
regprop (CodeModel.ArcSegment, "RotationAngle", "number", 0);
regprop (CodeModel.ArcSegment, "SweepDirection", "string", "Counterclockwise");
regprop (CodeModel.ArcSegment, "Point", "exception");
regprop (CodeModel.ArcSegment, "Size", "exception");

regprop (CodeModel.BezierSegment, "Point1", "exception");
regprop (CodeModel.BezierSegment, "Point2", "exception");
regprop (CodeModel.BezierSegment, "Point3", "exception");

regprop (CodeModel.LineSegment, "Point", "exception");

regprop (CodeModel.PolyBezierSegment, "Points", "exception");

regprop (CodeModel.PolyLineSegment, "Points", "exception");

regprop (CodeModel.PolyQuadraticBezierSegment, "Points", "exception");

regprop (CodeModel.QuadraticBezierSegment, "Point1", "exception");
regprop (CodeModel.QuadraticBezierSegment, "Point2", "exception");

regprop (CodeModel.BeginStoryboard, "Storyboard", "object", null);

regprop (CodeModel.DrawingAttributes, "Color", "number", -16777216);
regprop (CodeModel.DrawingAttributes, "Height", "number", 3);
regprop (CodeModel.DrawingAttributes, "OutlineColor", "number", 0);
regprop (CodeModel.DrawingAttributes, "Width", "number", 3);

regprop (CodeModel.Shape, "Fill", "object", null);
regprop (CodeModel.Shape, "Stroke", "object", null);
regprop (CodeModel.Shape, "StrokeDashArray", "exception");
regprop (CodeModel.Shape, "StrokeDashCap", "string", "Flat");
regprop (CodeModel.Shape, "StrokeDashOffset", "number", 0);
regprop (CodeModel.Shape, "StrokeEndLineCap", "string", "Flat");
regprop (CodeModel.Shape, "StrokeLineJoin", "string", "Miter");
regprop (CodeModel.Shape, "StrokeMiterLimit", "number", 10);
regprop (CodeModel.Shape, "StrokeStartLineCap", "string", "Flat");
regprop (CodeModel.Shape, "StrokeThickness", "number", 1);

regprop (CodeModel.Path, "Data", "object", null);
regprop (CodeModel.Path, "Stretch", "string", "None");

regprop (CodeModel.Polygon, "FillRule", "string", "EvenOdd");
regprop (CodeModel.Polygon, "Points", "exception");
regprop (CodeModel.Polygon, "Stretch", "string", "None");

regprop (CodeModel.Polyline, "FillRule", "string", "EvenOdd");
regprop (CodeModel.Polyline, "Points", "exception");
regprop (CodeModel.Polyline, "Stretch", "string", "None");

regprop (CodeModel.GeometryGroup, "Children", "object", "GeometryCollection");

regprop (CodeModel.Geometry, "Transform", "object", null);

regprop (CodeModel.Ellipse, "Stretch", "string", "Fill");

regprop (CodeModel.EllipseGeometry, "RadiusX", "number", 0);
regprop (CodeModel.EllipseGeometry, "RadiusY", "number", 0);
regprop (CodeModel.EllipseGeometry, "Center", "exception");

regprop (CodeModel.PathGeometry, "Figures", "object", null);
regprop (CodeModel.PathGeometry, "FillRule", "string", "EvenOdd");

regprop (CodeModel.Rectangle, "Stretch", "string", "Fill");

regprop (CodeModel.RectangleGeometry, "RadiusX", "number", 0);
regprop (CodeModel.RectangleGeometry, "RadiusY", "number", 0);
regprop (CodeModel.RectangleGeometry, "Rect", "exception");

regprop (CodeModel.LineGeometry, "StartPoint", "exception");
regprop (CodeModel.LineGeometry, "EndPoint", "exception");

regprop (CodeModel.PathFigure, "StartPoint", "exception");

regprop (CodeModel.EventTrigger, "Actions", "object", "TriggerActionCollection");
regprop (CodeModel.EventTrigger, "RoutedEvent", "string", "");

regprop (CodeModel.Glyphs, "Fill", "object", null);
regprop (CodeModel.Glyphs, "FontRenderingEmSize", "number", 0);
regprop (CodeModel.Glyphs, "FontUri", "string", "");
regprop (CodeModel.Glyphs, "Indices", "string", "");
regprop (CodeModel.Glyphs, "StyleSimulations", "string", "None");
regprop (CodeModel.Glyphs, "UnicodeString", "string", "");

regprop (CodeModel.GradientStop, "Color", "number", 0);
regprop (CodeModel.GradientStop, "Offset", "number", 0);

regprop (CodeModel.Image, "DownloadProgress", "number", 0);
regprop (CodeModel.Image, "Source", "string", "");
regprop (CodeModel.Image, "Stretch", "string", "Uniform");

regprop (CodeModel.InkPresenter, "Background", "object", null);
regprop (CodeModel.InkPresenter, "Children", "object", "VisualCollection");
regprop (CodeModel.InkPresenter, "Strokes", "object", "StrokeCollection");

regprop (CodeModel.Line, "Stretch", "string", "None");
regprop (CodeModel.Line, "X1", "number", 0);
regprop (CodeModel.Line, "X2", "number", 0);
regprop (CodeModel.Line, "Y1", "number", 0);
regprop (CodeModel.Line, "Y2", "number", 0);

regprop (CodeModel.LineBreak, "FontFamily", "string", "Portable User Interface");
regprop (CodeModel.LineBreak, "FontSize", "number", 14.666666984558105);
regprop (CodeModel.LineBreak, "FontStretch", "string", "Normal");
regprop (CodeModel.LineBreak, "FontStyle", "string", "Normal");
regprop (CodeModel.LineBreak, "FontWeight", "string", "Normal");
regprop (CodeModel.LineBreak, "Foreground", "object", "SolidColorBrush");
regprop (CodeModel.LineBreak, "TextDecorations", "string", "None");

regprop (CodeModel.Run, "FontFamily", "string", "Portable User Interface");
regprop (CodeModel.Run, "FontSize", "number", 14.666666984558105);
regprop (CodeModel.Run, "FontStretch", "string", "Normal");
regprop (CodeModel.Run, "FontStyle", "string", "Normal");
regprop (CodeModel.Run, "FontWeight", "string", "Normal");
regprop (CodeModel.Run, "Foreground", "object", "SolidColorBrush");
regprop (CodeModel.Run, "TextDecorations", "string", "None");

regprop (CodeModel.Matrix, "M11", "number", 1);
regprop (CodeModel.Matrix, "M12", "number", 0);
regprop (CodeModel.Matrix, "M21", "number", 0);
regprop (CodeModel.Matrix, "M22", "number", 1);
regprop (CodeModel.Matrix, "OffsetX", "number", 0);
regprop (CodeModel.Matrix, "OffsetY", "number", 0);

regprop (CodeModel.MatrixTransform, "Matrix", "object", null);

regprop (CodeModel.RotateTransform, "Angle", "number", 0);
regprop (CodeModel.RotateTransform, "CenterX", "number", 0);
regprop (CodeModel.RotateTransform, "CenterY", "number", 0);

regprop (CodeModel.ScaleTransform, "CenterX", "number", 0);
regprop (CodeModel.ScaleTransform, "CenterY", "number", 0);
regprop (CodeModel.ScaleTransform, "ScaleX", "number", 1);
regprop (CodeModel.ScaleTransform, "ScaleY", "number", 1);

regprop (CodeModel.SkewTransform, "AngleX", "number", 0);
regprop (CodeModel.SkewTransform, "AngleY", "number", 0);
regprop (CodeModel.SkewTransform, "CenterX", "number", 0);
regprop (CodeModel.SkewTransform, "CenterY", "number", 0);

regprop (CodeModel.TransformGroup, "Children", "object", "TransformCollection");

regprop (CodeModel.TranslateTransform, "X", "number", 0);
regprop (CodeModel.TranslateTransform, "Y", "number", 0);

regprop (CodeModel.MediaElement, "Attributes", "object", "MediaAttributeCollection");
regprop (CodeModel.MediaElement, "AutoPlay", "boolean", true);
regprop (CodeModel.MediaElement, "AudioStreamCount", "number", 0);
regprop (CodeModel.MediaElement, "AudioStreamIndex", "object", null);
regprop (CodeModel.MediaElement, "Balance", "number", 0);
regprop (CodeModel.MediaElement, "BufferingProgress", "number", 0);
regprop (CodeModel.MediaElement, "BufferingTime", "object", "TimeSpan");
regprop (CodeModel.MediaElement, "CanPause", "boolean", false);
regprop (CodeModel.MediaElement, "CurrentState", "string", "Closed");
regprop (CodeModel.MediaElement, "DownloadProgress", "number", 0);
regprop (CodeModel.MediaElement, "IsMuted", "boolean", false);
regprop (CodeModel.MediaElement, "Markers", "object", "TimelineMarkerCollection");
regprop (CodeModel.MediaElement, "NaturalDuration", "object", "Duration");
regprop (CodeModel.MediaElement, "NaturalVideoHeight", "number", 0);
regprop (CodeModel.MediaElement, "NaturalVideoWidth", "number", 0);
regprop (CodeModel.MediaElement, "Position", "object", "TimeSpan");
regprop (CodeModel.MediaElement, "Source", "string", "");
regprop (CodeModel.MediaElement, "Stretch", "string", "Uniform");
regprop (CodeModel.MediaElement, "Volume", "number", 0.5);

regprop (CodeModel.PathFigure, "IsClosed", "boolean", false);
regprop (CodeModel.PathFigure, "Segments", "object", null);

regprop (CodeModel.SilverlightPlugin, "InitParams", "string", "");
regprop (CodeModel.SilverlightPlugin, "IsLoaded", "boolean", true);
regprop (CodeModel.SilverlightPlugin, "MaxFrameRate", "undefined", undefined);
regprop (CodeModel.SilverlightPlugin, "Source", "string", "xaml/void.xaml");
regprop (CodeModel.SilverlightPlugin, "Settings", "object", "Settings");
regprop (CodeModel.SilverlightPlugin, "Content", "object", "Content");

regprop (CodeModel.SilverlightPluginContent, "Accessibility", "object", "AccessibilityObject");
regprop (CodeModel.SilverlightPluginContent, "ActualHeight", "number", 1);
regprop (CodeModel.SilverlightPluginContent, "ActualWidth", "number", 1);
regprop (CodeModel.SilverlightPluginContent, "FullScreen", "boolean", false);
regprop (CodeModel.SilverlightPluginContent, "Root", "object", "Canvas");

regprop (CodeModel.SilverlightPluginSettings, "Background", "string", "#ffffff");
regprop (CodeModel.SilverlightPluginSettings, "EnableFramerateCounter", "boolean", false);
regprop (CodeModel.SilverlightPluginSettings, "EnableRedrawRegions", "boolean", false);
regprop (CodeModel.SilverlightPluginSettings, "EnableHtmlAccess", "boolean", true);
regprop (CodeModel.SilverlightPluginSettings, "MaxFramerate", "number", 60);
regprop (CodeModel.SilverlightPluginSettings, "Windowless", "boolean", false);

regprop (CodeModel.Stroke, "DrawingAttributes", "object", "DrawingAttributes");
regprop (CodeModel.Stroke, "StylusPoints", "object", "StylusPointCollection");

regprop (CodeModel.StylusPoint, "PressureFactor", "number", 0.5);
regprop (CodeModel.StylusPoint, "X", "number", 0);
regprop (CodeModel.StylusPoint, "Y", "number", 0);

regprop (CodeModel.TextBlock, "ActualHeight", "number", 0);
regprop (CodeModel.TextBlock, "ActualWidth", "number", 0);
regprop (CodeModel.TextBlock, "FontFamily", "string", "Portable User Interface");
regprop (CodeModel.TextBlock, "FontSize", "number", 14.666666984558105);
regprop (CodeModel.TextBlock, "FontStretch", "string", "Normal");
regprop (CodeModel.TextBlock, "FontStyle", "string", "Normal");
regprop (CodeModel.TextBlock, "FontWeight", "string", "Normal");
regprop (CodeModel.TextBlock, "Foreground", "object", "SolidColorBrush");
regprop (CodeModel.TextBlock, "Text", "string", "");
regprop (CodeModel.TextBlock, "TextDecorations", "string", "None");
regprop (CodeModel.TextBlock, "TextWrapping", "string", "NoWrap");

regprop (CodeModel.TimelineMarker, "Text", "string", "");
regprop (CodeModel.TimelineMarker, "Time", "object", null);
regprop (CodeModel.TimelineMarker, "Type", "string", "");
