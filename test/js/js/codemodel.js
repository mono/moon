var CodeModel = {};

CodeModel.DependencyObject = {
	name: "DependencyObject",

	properties: ["Name"],

	methods: ["FindName", "GetHost", "GetValue", "SetValue", "Equals"],

	events: []
};

CodeModel.Accessibility = {
	name: "AccessibilityObject",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.accessibility;
	},

	properties: ["ActionDescription", "Description", "Title"],

	events: ["PerformAction"]
};

CodeModel.Downloader = {
	name: "Downloader",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.createObject ("Downloader");
	},

	properties: [
		"DownloadProgress",
		"GetResponseText",
		"Status",
		"StatusText",
		"URI"
	],

	methods: [
		"Abort",
		"GetResponseText",
		"Open",
		"Send"
	],

	events: [
		"Completed",
		"DownloadProgressChanged"
	]
};

CodeModel.UIElement = {
	name: "UIElement",

	parent: CodeModel.DependencyObject,

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Height",
		"IsHitTestVisible",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Tag",
		"Triggers",
		"Visibility",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"GetParent",
		"ReleaseMouseCapture",
		"RemoveEventListener"
	],

	events: [
		"GotFocus",
		"KeyDown",
		"KeyUp",
		"Loaded",
		"LostFocus",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
	]
};

CodeModel.Canvas = {
	name: "Canvas",

	parent: CodeModel.UIElement,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Canvas />");
	},

	properties: [
		"Background",
		"Children"
	]
};

CodeModel.Collection = {
	name: "Collection",

	parent: CodeModel.DependencyObject,

	properties: ["Count"],

	methods: [
		"Add",
		"Clear",
		"Equals",
		"FindName",
		"GetHost",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

CodeModel.ColorKeyFrameCollection = {
	name: "ColorKeyFrameCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><SplineColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames;
	}
};

CodeModel.DoubleKeyFrameCollection = {
	name: "DoubleKeyFrameCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleKeyFrameCollection />");
	}
};

CodeModel.GeometryCollection = {
	name: "GeometryCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GeometryCollection />");
	}
};

CodeModel.GradientStopCollection = {
	name: "GradientStopCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GradientStopCollection />");
	}
};

CodeModel.MediaAttributeCollection = {
	name: "MediaAttributeCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MediaElement />").attributes;
	},

	methods: [
		"GetItem",
		"GetItemByName"
	]
};

CodeModel.PathFigureCollection = {
	name: "PathFigureCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathFigureCollection />");
	}
};

CodeModel.PathSegmentCollection = {
	name: "PathSegmentCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathSegmentCollection />");
	}
};

CodeModel.PointKeyFrameCollection = {
	name: "PointKeyFrameCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointKeyFrameCollection />");
	}
};

CodeModel.ResourceDictionary = {
	name: "ResourceDictionary",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ResourceDictionary />");
	}
};

CodeModel.StrokeCollection = {
	name: "StrokeCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StrokeCollection />");
	},

	methods: [
		"GetBounds",
		"HitTest",
	]
};

CodeModel.StylusPointCollection = {
	name: "StylusPointCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StylusPointCollection />");
	},

	methods: [
		"AddStylusPoints"
		//"GetBounds",
		//"HitTest",
	]
};

CodeModel.TimelineCollection = {
	name: "TimelineCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineCollection />");
	}
};

CodeModel.TimelineMarkerCollection = {
	name: "TimelineMarkerCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineMarkerCollection />");
	}
};

CodeModel.TransformCollection = {
	name: "TransformCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TransformCollection />");
	}
};

CodeModel.TriggerActionCollection = {
	name: "TriggerActionCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerActionCollection />");
	}
};

CodeModel.TriggerCollection = {
	name: "TriggerCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerCollection />");
	}
};

CodeModel.UIElementCollection = {
	name: "VisualCollection",

	parent: CodeModel.Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Canvas />").children;
	}
};

CodeModel.KeyFrame = {
	name: "KeyFrame",

	parent: CodeModel.DependencyObject,

	properties: ["KeyTime", "Value"]
};

CodeModel.ColorKeyFrame = {
	name: "ColorKeyFrame",

	parent: CodeModel.KeyFrame
}

CodeModel.DoubleKeyFrame = {
	name: "DoubleKeyFrame",

	parent: CodeModel.KeyFrame
}

CodeModel.PointKeyFrame = {
	name: "PointKeyFrame",

	parent: CodeModel.KeyFrame
}

CodeModel.DiscreteColorKeyFrame = {
	name: "DiscreteColorKeyFrame",

	parent: CodeModel.ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><DiscreteColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

CodeModel.DiscreteDoubleKeyFrame = {
	name: "DiscreteDoubleKeyFrame",

	parent: CodeModel.DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><DiscreteDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

CodeModel.DiscretePointKeyFrame = {
	name: "DiscretePointKeyFrame",

	parent: CodeModel.PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><DiscretePointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

CodeModel.LinearColorKeyFrame = {
	name: "LinearColorKeyFrame",

	parent: CodeModel.ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><LinearColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

CodeModel.LinearDoubleKeyFrame = {
	name: "LinearDoubleKeyFrame",

	parent: CodeModel.DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><LinearDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

CodeModel.LinearPointKeyFrame = {
	name: "LinearPointKeyFrame",

	parent: CodeModel.PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><LinearPointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

CodeModel.SplineColorKeyFrame = {
	name: "SplineColorKeyFrame",

	parent: CodeModel.ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><SplineColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

CodeModel.SplineDoubleKeyFrame = {
	name: "SplineDoubleKeyFrame",

	parent: CodeModel.DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><SplineDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

CodeModel.SplinePointKeyFrame = {
	name: "SplinePointKeyFrame",

	parent: CodeModel.PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><SplinePointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

CodeModel.Timeline = {
	name: "Timeline",

	parent: CodeModel.DependencyObject,

	properties: [
		"AutoReverse",
		"BeginTime",
		"Duration",
		"FillBehavior",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty"
	]
};

CodeModel.TimelineGroup = {
	name: "TimeLineGroup",

	parent: CodeModel.Timeline,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineGroup />");
	},

	properties: ["Children"]
};

CodeModel.Storyboard = {
	name: "Storyboard",

	parent: CodeModel.Timeline,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Storyboard />");
	},

	properties: [
		"Children"
	],

	methods: [
		"Begin",
		"Pause",
		"Resume",
		"Seek",
		"Stop"
	],

	events: ["Completed"]
};

CodeModel.Animation = {
	name: "Animation",

	parent: CodeModel.Timeline,

	properties: ["By", "From", "To"]
};

CodeModel.ColorAnimation = {
	name: "ColorAnimation",

	parent: CodeModel.Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimation />");
	}
};

CodeModel.ColorAnimationUsingKeyFrames = {
	name: "ColorAnimationUsingKeyFrames",

	parent: CodeModel.ColorAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

CodeModel.DoubleAnimation = {
	name: "DoubleAnimation",

	parent: CodeModel.Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimation />");
	}
};

CodeModel.DoubleAnimationUsingKeyFrames = {
	name: "DoubleAnimationUsingKeyFrames",

	parent: CodeModel.DoubleAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

CodeModel.PointAnimation = {
	name: "PointAnimation",

	parent: CodeModel.Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimation />");
	}
};

CodeModel.PointAnimationUsingKeyFrames = {
	name: "PointAnimationUsingKeyFrames",

	parent: CodeModel.PointAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

CodeModel.Brush = {
	name: "Brush",

	parent: CodeModel.DependencyObject,

	properties: [
		"Opacity",
		"RelativeTransform",
		"Transform"
	]
};

CodeModel.ImageBrush = {
	name: "ImageBrush",

	parent: CodeModel.Brush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ImageBrush />");
	},

	properties: [
		"AlignmentX",
		"AlignmentY",
		"DownloadProgress",
		"ImageSource",
		"Stretch"
	],

	methods: ["SetSource"],

	events: ["DownloadProgressChanged"]
};

CodeModel.SolidColorBrush = {
	name: "SolidColorBrush",

	parent: CodeModel.Brush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<SolidColorBrush />");
	},

	properties: ["Color"]
};

CodeModel.VideoBrush = {
	name: "VideoBrush",

	parent: CodeModel.Brush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<VideoBrush />");
	},

	properties: [
		"SourceName",
		"Stretch"
	]
};

CodeModel.GradientBrush = {
	name: "GradientBrush",

	parent: CodeModel.Brush,

	properties: [
		"ColorInterpolationMode",
		"GradientStops",
		"MappingMode",
		"SpreadMethod"
	]
};

CodeModel.LinearGradientBrush = {
	name: "LinearGradientBrush",

	parent: CodeModel.GradientBrush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LinearGradientBrush />");
	},

	properties: [
		"StartPoint",
		"EndPoint"
	]
};

CodeModel.RadialGradientBrush = {
	name: "RadialGradientBrush",

	parent: CodeModel.GradientBrush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<RadialGradientBrush />");
	},

	properties: [
		"Center",
		"GradientOrigin",
		"RadiusX",
		"RadiusY"
	]
};

CodeModel.PathSegment = {
	name: "PathSegment",

	parent: CodeModel.DependencyObject
};

CodeModel.ArcSegment = {
	name: "ArcSegment",

	parent: CodeModel.PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ArcSegment />");
	},

	properties: [
		"IsLargeArc",
		"Point",
		"RotationAngle",
		"Size",
		"SweepDirection"
	]
};

CodeModel.BezierSegment = {
	name: "BezierSegment",

	parent: CodeModel.PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<BezierSegment />");
	},

	properties: [
		"Point1",
		"Point2",
		"Point3"
	]
};

CodeModel.LineSegment = {
	name: "LineSegment",

	parent: CodeModel.PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LineSegment />");
	},

	properties: ["Point"]
};

CodeModel.PolyBezierSegment = {
	name: "PolyBezierSegment",

	parent: CodeModel.PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyBezierSegment />");
	},

	properties: [
		"Points"
	]
};

CodeModel.PolyLineSegment = {
	name: "PolyLineSegment",

	parent: CodeModel.PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyLineSegment />");
	},

	properties: ["Points"]
};

CodeModel.PolyQuadraticBezierSegment = {
	name: "PolyQuadraticBezierSegment",

	parent: CodeModel.PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyQuadraticBezierSegment />");
	},

	properties: ["Points"]
};

CodeModel.QuadraticBezierSegment = {
	name: "QuadraticBezierSegment",

	parent: CodeModel.PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<QuadraticBezierSegment />");
	},

	properties: [
		"Point1",
		"Point2"
	]
};

CodeModel.TriggerAction = {
	name: "TriggerAction",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerAction />");
	}
};

CodeModel.BeginStoryboard = {
	name: "BeginStoryboard",

	parent: CodeModel.TriggerAction,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<BeginStoryboard />");
	},

	properties: ["Storyboard"]
};

CodeModel.DrawingAttributes = {
	name: "DrawingAttributes",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DrawingAttributes />");
	},

	properties: [
		"Color",
		"Height",
		"OutlineColor",
		"Width"
	]
};

CodeModel.Shape = {
	name: "Shape",

	parent: CodeModel.UIElement,

	properties: [
		"Fill",
		"Stretch",
		"Stroke",
		"StrokeDashArray",
		"StrokeDashCap",
		"StrokeDashOffset",
		"StrokeEndLineCap",
		"StrokeLineJoin",
		"StrokeMiterLimit",
		"StrokeStartLineCap",
		"StrokeThickness"
	]
};

CodeModel.Ellipse = {
	name: "Ellipse",

	parent: CodeModel.Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Ellipse />");
	},

	properties: ["Stretch"]
};

CodeModel.Path = {
	name: "Path",

	parent: CodeModel.Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Path />");
	},

	properties: ["Data", "Stretch"]
};

CodeModel.Polygon = {
	name: "Polygon",

	parent: CodeModel.Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Polygon />");
	},

	properties: ["FillRule", "Points", "Stretch"]
};

CodeModel.Polyline = {
	name: "Polyline",

	parent: CodeModel.Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Polyline />");
	},

	properties: ["FillRule", "Points", "Stretch"]
};

CodeModel.Rectangle = {
	name: "Rectangle",

	parent: CodeModel.Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Rectangle />");
	},

	properties: [ "Stretch" ]
};

CodeModel.Geometry = {
	name: "Geometry",

	parent: CodeModel.DependencyObject,

	properties: ["Transform"]
};

CodeModel.GeometryGroup = {
	name: "GeometryGroup",

	parent: CodeModel.Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GeometryGroup />");
	},

	properties: ["Children"]
};

CodeModel.EllipseGeometry = {
	name: "EllipseGeometry",

	parent: CodeModel.Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<EllipseGeometry />");
	},

	properties: [
		"Center",
		"RadiusX",
		"RadiusY"
	]
};

CodeModel.LineGeometry = {
	name: "LineGeometry",

	parent: CodeModel.Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LineGeometry />");
	},

	properties: [
		"StartPoint",
		"EndPoint"
	]
};

CodeModel.PathGeometry = {
	name: "PathGeometry",

	parent: CodeModel.Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathGeometry />");
	},

	properties: [
		"Figures",
		"FillRule"
	]
};

CodeModel.RectangleGeometry = {
	name: "RectangleGeometry",

	parent: CodeModel.Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<RectangleGeometry />");
	},

	properties: [
		"RadiusX",
		"RadiusY",
		"Rect"
	]
};

CodeModel.EventTrigger = {
	name: "EventTrigger",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<EventTrigger />");
	},

	properties: [
		"Actions",
		"RoutedEvent"
	]
};

CodeModel.Glyphs = {
	name: "Glyphs",

	parent: CodeModel.UIElement,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Glyphs />");
	},

	properties: [
		"Fill",
		"FontRenderingEmSize",
		"FontUri",
		"Indices",
		"StyleSimulations",
		"UnicodeString"
	]
};

CodeModel.GradientStop = {
	name: "GradientStop",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GradientStop />");
	},

	properties: [
		"Color",
		"Offset"
	]
};

CodeModel.Image = {
	name: "Image",

	parent: CodeModel.UIElement,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Image />");
	},

	properties: [
		"DownloadProgress",
		"Source",
		"Stretch"
	],

	events: [
		"DownloadProgressChanged",
		"ImageFailed"
	]
};

CodeModel.InkPresenter = {
	name: "InkPresenter",

	parent: CodeModel.Canvas,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<InkPresenter />");
	},

	properties: [
		"Background",
		"Children",
		"Strokes"
	]
};

CodeModel.KeySpline = {
	name: "KeySpline",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<KeySpline />");
	}
};

CodeModel.Line = {
	name: "Line",

	parent: CodeModel.Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Line />");
	},

	properties: [
		"Stretch",
		"X1",
		"X2",
		"Y1",
		"Y2"
	]
};

CodeModel.LineBreak = {
	name: "LineBreak",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LineBreak />");
	},

	properties: [
		"FontFamily",
		"FontSize",
		"FontStretch",
		"FontStyle",
		"FontWeight",
		"Foreground",
		"TextDecorations"
	]
};

CodeModel.Run = {
	name: "Run",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Run />");
	},

	properties: [
		"FontFamily",
		"FontSize",
		"FontStretch",
		"FontStyle",
		"FontWeight",
		"Foreground",
		"TextDecorations"
	]
};

CodeModel.Matrix = {
	name: "Matrix",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Matrix />");
	},

	properties: [
		"M11",
		"M12",
		"M21",
		"M22",
		"OffsetX",
		"OffsetY"
	]
};

CodeModel.Transform = {
	name: "Transform",

	parent: CodeModel.DependencyObject
};

CodeModel.MatrixTransform = {
	name: "MatrixTransform",

	parent: CodeModel.Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MatrixTransform />");
	},

	properties: ["Matrix"]
};

CodeModel.RotateTransform = {
	name: "RotateTransform",

	parent: CodeModel.Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<RotateTransform />");
	},

	properties: [
		"Angle",
		"CenterX",
		"CenterY",
	]
};

CodeModel.ScaleTransform = {
	name: "ScaleTransform",

	parent: CodeModel.Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ScaleTransform />");
	},

	properties: [
		"CenterX",
		"CenterY",
		"ScaleX",
		"ScaleY"
	]
};

CodeModel.SkewTransform = {
	name: "SkewTransform",

	parent: CodeModel.Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<SkewTransform />");
	},

	properties: [
		"AngleX",
		"AngleY",
		"CenterX",
		"CenterY",
	]
};

CodeModel.TransformGroup = {
	name: "TransformGroup",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TransformGroup />");
	},

	properties: ["Children"]
};

CodeModel.TranslateTransform = {
	name: "TranslateTransform",

	parent: CodeModel.Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TranslateTransform />");
	},

	properties: [
		"X",
		"Y"
	]
};

CodeModel.MediaAttribute = {
	name: "MediaAttribute",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MediaAttribute />");
	},

	properties: ["Value"]
};

CodeModel.MediaElement = {
	name: "MediaElement",

	parent: CodeModel.UIElement,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MediaElement />");
	},

	properties: [
		"Attributes",
		"AutoPlay",
		"AudioStreamCount",
		"AudioStreamIndex",
		"Balance",
		"BufferingProgress",
		"BufferingTime",
		"CanPause",
		"CurrentState",
		"DownloadProgress",
		"IsMuted",
		"Markers",
		"NaturalDuration",
		"NaturalVideoHeight",
		"NaturalVideoWidth",
		"Position",
		"Source",
		"Stretch",
		"Volume"
	],

	methods: [
		"Pause",
		"Play",
		"SetSource",
		"Stop"
	],

	events: [
		"BufferingProgressChanged",
		"CurrentStateChanged",
		"DownloadProgressChanged",
		"MarkerReached",
		"MediaEnded",
		"MediaFailed",
		"MediaOpened"
	]
};

CodeModel.PathFigure = {
	name: "PathFigure",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathFigure />");
	},

	properties: [
		"IsClosed",
		"Segments",
		"StartPoint"
	]
};

CodeModel.SilverlightPlugin = {
	name: "SilverlightPlugin",

	create: function (plugin) {
		return plugin;
	},

	properties: [
		"Content",
		"InitParams",
		"IsLoaded",
		"Source",
		"Settings"
	],

	methods: [
		"createObject",
		"IsVersionSupported"
	],

	events: [
		"OnError",
		"OnLoad",
	]
};

CodeModel.SilverlightPluginSettings = {
	name: "Settings",

	create: function (plugin) {
		return plugin.settings;
	},

	properties: [
		"Background",
		"EnableFramerateCounter",
		"EnableRedrawRegions",
		"EnableHtmlAccess",
		"MaxFramerate",
		//"Version",
		"Windowless"
	]
};

CodeModel.SilverlightPluginContent = {
	name: "Content",

	create: function (plugin) {
		return plugin.content;
	},

	properties: [
		"Accessibility",
		"ActualHeight",
		"ActualWidth",
		"FullScreen",
		"Root",
	],

	methods: [
		"CreateFromXaml",
		"CreateFromXamlDownloader",
		"FindName",
	],

	events: [
		"OnFullScreenChange",
		"OnResize"
	]
};

CodeModel.Stroke = {
	name: "Stroke",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Stroke />");
	},

	properties: [
		"DrawingAttributes",
		"StylusPoints"
	],

	methods: [
		"GetBounds",
		"HitTest",
	]
};

CodeModel.StylusInfo = {
	name: "StylusInfo",

	parent: CodeModel.DependencyObject,

	properties: [
		"IsInverted",
		"DeviceType"
	]
};

CodeModel.StylusPoint = {
	name: "StylusPoint",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StylusPoint />");
	},

	properties: [
		"PressureFactor",
		"X",
		"Y"
	]
};

CodeModel.TextBlock = {
	name: "TextBlock",

	parent: CodeModel.UIElement,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TextBlock />");
	},

	properties: [
		"ActualHeight",
		"ActualWidth",
		"FontFamily",
		"FontSize",
		"FontStretch",
		"FontStyle",
		"FontWeight",
		"Foreground",
		"Text",
		"TextDecorations",
		"TextWrapping"
	],

	methods: ["SetFontSource"]
};

CodeModel.TimelineMarker = {
	name: "TimelineMarker",

	parent: CodeModel.DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineMarker />");
	},

	properties: [
		"Text",
		"Time",
		"Type"
	]
};

CodeModel.EventArgs = {
	name: "EventArgs",

	parent: CodeModel.DependencyObject
};

CodeModel.KeyboardEventArgs  = {
	name: "KeyboardEventArgs",

	parent: CodeModel.EventArgs,

	properties: [
		"Ctrl",
		"Key",
		"PlatformKeyCode",
		"Shift"
	]
};

CodeModel.MouseEventArgs  = {
	name: "MouseEventArgs",

	parent: CodeModel.EventArgs,

	properties: [
		"Ctrl",
		"Shift"
	],

	methods: [
		"GetPosition",
		"GetStylusInfo",
		"GetStylusPoints"
	]
};

CodeModel.ErrorEventArgs  = {
	name: "ErrorEventArgs ",

	parent: CodeModel.EventArgs,

	properties: [
		"ErrorCode",
		"ErrorMessage",
		"ErrorType"
	]
};

CodeModel.ParserErrorEventArgs  = {
	name: "ParserErrorEventArgs ",

	parent: CodeModel.ErrorEventArgs,

	properties: [
		"CharPosition",
		"LineNumber",
		"XamlFile",
		"XmlAttribute",
		"XmlElement"
	]
};

CodeModel.RuntimeErrorEventArgs  = {
	name: "RuntimeErrorEventArgs ",

	parent: CodeModel.ErrorEventArgs,

	properties: [
		"CharPosition",
		"LineNumber",
		"MethodName"
	]
};
