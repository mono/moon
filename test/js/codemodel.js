var DependencyObject = {
	name: "DependencyObject",

	properties: ["Name"],

	methods: ["FindName", "GetHost", "GetValue", "SetValue", "Equals"],

	events: []
};

var Accessibility = {
	name: "Accessibility",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.accessibility;
	},

	properties: ["ActionDescription", "Description", "Title"],

	events: ["PerformAction"]
};

var Downloader = {
	name: "Downloader",

	parent: DependencyObject,

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

var UIElement = {
	name: "UIElement",

	parent: DependencyObject,

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

var Canvas = {
	name: "Canvas",

	parent: UIElement,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Canvas />");
	},

	properties: [
		"Background",
		"Children"
	]
};

var Collection = {
	name: "Collection",

	parent: DependencyObject,

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

var ColorKeyFrameCollection = {
	name: "ColorKeyFrameCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><SplineColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames;
	}
};

var DoubleKeyFrameCollection = {
	name: "DoubleKeyFrameCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleKeyFrameCollection />");
	}
};

var GeometryCollection = {
	name: "GeometryCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GeometryCollection />");
	}
};

var GradientStopCollection = {
	name: "GradientStopCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GradientStopCollection />");
	}
};

var MediaAttributeCollection = {
	name: "MediaAttributeCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MediaElement />").attributes;
	},

	methods: [
		"GetItem",
		"GetItemByName"
	]
};

var PathFigureCollection = {
	name: "PathFigureCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathFigureCollection />");
	}
};

var PathSegmentCollection = {
	name: "PathSegmentCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathSegmentCollection />");
	}
};

var PointKeyFrameCollection = {
	name: "PointKeyFrameCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointKeyFrameCollection />");
	}
};

var ResourceDictionary = {
	name: "ResourceDictionary",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ResourceDictionary />");
	}
};

var StrokeCollection = {
	name: "StrokeCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StrokeCollection />");
	},

	methods: [
		"GetBounds",
		"HitTest",
	]
};

var StylusPointCollection = {
	name: "StylusPointCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StylusPointCollection />");
	},

	methods: [
		"AddStylusPoints"
		//"GetBounds",
		//"HitTest",
	]
};

var TimelineCollection = {
	name: "TimelineCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineCollection />");
	}
};

var TimelineMarkerCollection = {
	name: "TimelineMarkerCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineMarkerCollection />");
	}
};

var TransformCollection = {
	name: "TransformCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TransformCollection />");
	}
};

var TriggerActionCollection = {
	name: "TriggerActionCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerActionCollection />");
	}
};

var TriggerCollection = {
	name: "TriggerCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerCollection />");
	}
};

var UIElementCollection = {
	name: "UIElementCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Canvas />").children;
	}
};

var KeyFrame = {
	name: "KeyFrame",

	parent: DependencyObject,

	properties: ["KeyTime", "Value"]
};

var ColorKeyFrame = {
	name: "ColorKeyFrame",

	parent: KeyFrame
}

var DoubleKeyFrame = {
	name: "DoubleKeyFrame",

	parent: KeyFrame
}

var PointKeyFrame = {
	name: "PointKeyFrame",

	parent: KeyFrame
}

var DiscreteColorKeyFrame = {
	name: "DiscreteColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><DiscreteColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

var DiscreteDoubleKeyFrame = {
	name: "DiscreteDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><DiscreteDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

var DiscretePointKeyFrame = {
	name: "DiscretePointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><DiscretePointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

var LinearColorKeyFrame = {
	name: "LinearColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><LinearColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

var LinearDoubleKeyFrame = {
	name: "LinearDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><LinearDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

var LinearPointKeyFrame = {
	name: "LinearPointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><LinearPointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

var SplineColorKeyFrame = {
	name: "SplineColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><SplineColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

var SplineDoubleKeyFrame = {
	name: "SplineDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><SplineDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

var SplinePointKeyFrame = {
	name: "SplinePointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><SplinePointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

var Timeline = {
	name: "TimeLine",

	parent: DependencyObject,

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

var TimelineGroup = {
	name: "TimeLineGroup",

	parent: Timeline,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineGroup />");
	},

	properties: ["Children"]
};

var Storyboard = {
	name: "Storyboard",

	parent: Timeline,

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

var Animation = {
	name: "Animation",

	parent: Timeline,

	properties: ["By", "From", "To"]
};

var ColorAnimation = {
	name: "ColorAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimation />");
	}
};

var ColorAnimationUsingKeyFrames = {
	name: "ColorAnimationUsingKeyFrames",

	parent: ColorAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

var DoubleAnimation = {
	name: "DoubleAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimation />");
	}
};

var DoubleAnimationUsingKeyFrames = {
	name: "DoubleAnimationUsingKeyFrames",

	parent: DoubleAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

var PointAnimation = {
	name: "PointAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimation />");
	}
};

var PointAnimationUsingKeyFrames = {
	name: "PointAnimationUsingKeyFrames",

	parent: PointAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

var Brush = {
	name: "Brush",

	parent: DependencyObject,

	properties: [
		"Opacity",
		"RelativeTransform",
		"Transform"
	]
};

var ImageBrush = {
	name: "ImageBrush",

	parent: Brush,

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

var SolidColorBrush = {
	name: "SolidColorBrush",

	parent: Brush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<SolidColorBrush />");
	},

	properties: ["Color"]
};

var VideoBrush = {
	name: "VideoBrush",

	parent: Brush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<VideoBrush />");
	},

	properties: [
		"SourceName",
		"Stretch"
	]
};

var GradientBrush = {
	name: "GradientBrush",

	parent: Brush,

	properties: [
		"ColorInterpolationMode",
		"GradientStops",
		"MappingMode",
		"SpreadMethod"
	]
};

var LinearGradientBrush = {
	name: "LinearGradientBrush",

	parent: GradientBrush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LinearGradientBrush />");
	},

	properties: [
		"StartPoint",
		"EndPoint"
	]
};

var RadialGradientBrush = {
	name: "RadialGradientBrush",

	parent: GradientBrush,

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

var PathSegment = {
	name: "PathSegment",

	parent: DependencyObject
};

var ArcSegment = {
	name: "ArcSegment",

	parent: PathSegment,

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

var BezierSegment = {
	name: "BezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<BezierSegment />");
	},

	properties: [
		"Point1",
		"Point2",
		"Point3"
	]
};

var LineSegment = {
	name: "LineSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LineSegment />");
	},

	properties: ["Point"]
};

var PolyBezierSegment = {
	name: "PolyBezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyBezierSegment />");
	},

	properties: [
		"Points"
	]
};

var PolyLineSegment = {
	name: "PolyLineSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyLineSegment />");
	},

	properties: ["Points"]
};

var PolyQuadraticBezierSegment = {
	name: "PolyQuadraticBezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyQuadraticBezierSegment />");
	},

	properties: ["Points"]
};

var QuadraticBezierSegment = {
	name: "QuadraticBezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<QuadraticBezierSegment />");
	},

	properties: [
		"Point1",
		"Point2"
	]
};

var TriggerAction = {
	name: "TriggerAction",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerAction />");
	}
};

var BeginStoryboard = {
	name: "BeginStoryboard",

	parent: TriggerAction,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<BeginStoryboard />");
	},

	properties: ["Storyboard"]
};

var DrawingAttributes = {
	name: "DrawingAttributes",

	parent: DependencyObject,

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

var Shape = {
	name: "Shape",

	parent: UIElement,

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

var Ellipse = {
	name: "Ellipse",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Ellipse />");
	}
};

var Path = {
	name: "Path",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Path />");
	},

	properties: ["Data"]
};

var Polygon = {
	name: "Polygon",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Polygon />");
	},

	properties: ["FillRule"]
};

var Polyline = {
	name: "Polyline",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Polyline />");
	},

	properties: ["FillRule"]
};

var Rectangle = {
	name: "Rectangle",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Rectangle />");
	}
};

var Geometry = {
	name: "Geometry",

	parent: DependencyObject,

	properties: ["Transform"]
};

var GeometryGroup = {
	name: "GeometryGroup",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GeometryGroup />");
	},

	properties: ["Children"]
};

var EllipseGeometry = {
	name: "EllipseGeometry",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<EllipseGeometry />");
	},

	properties: [
		"Center",
		"RadiusX",
		"RadiusY"
	]
};

var LineGeometry = {
	name: "LineGeometry",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LineGeometry />");
	},

	properties: [
		"StartPoint",
		"EndPoint"
	]
};

var PathGeometry = {
	name: "PathGeometry",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathGeometry />");
	},

	properties: [
		"Figures",
		"FillRule"
	]
};

var RectangleGeometry = {
	name: "RectangleGeometry",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<RectangleGeometry />");
	},

	properties: [
		"RadiusX",
		"RadiusY",
		"Rect"
	]
};

var EventTrigger = {
	name: "EventTrigger",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<EventTrigger />");
	},

	properties: [
		"Actions",
		"RoutedEvent"
	]
};

var Glyphs = {
	name: "Glyphs",

	parent: UIElement,

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

var GradientStop = {
	name: "GradientStop",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GradientStop />");
	},

	properties: [
		"Color",
		"Offset"
	]
};

var Image = {
	name: "Image",

	parent: UIElement,

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

var InkPresenter = {
	name: "InkPresenter",

	parent: Canvas,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<InkPresenter />");
	},

	properties: [
		"Background",
		"Children",
		"Strokes"
	]
};

var KeySpline = {
	name: "KeySpline",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<KeySpline />");
	}
};

var Line = {
	name: "Line",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Line />");
	},

	properties: [
		"X1",
		"X2",
		"Y1",
		"Y2"
	]
};

var LineBreak = {
	name: "LineBreak",

	parent: DependencyObject,

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

var Run = {
	name: "Run",

	parent: DependencyObject,

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

var Matrix = {
	name: "Matrix",

	parent: DependencyObject,

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

var Transform = {
	name: "Transform",

	parent: DependencyObject
};

var MatrixTransform = {
	name: "MatrixTransform",

	parent: Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MatrixTransform />");
	},

	properties: ["Matrix"]
};

var RotateTransform = {
	name: "RotateTransform",

	parent: Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<RotateTransform />");
	},

	properties: [
		"Angle",
		"CenterX",
		"CenterY",
	]
};

var ScaleTransform = {
	name: "ScaleTransform",

	parent: Transform,

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

var SkewTransform = {
	name: "SkewTransform",

	parent: Transform,

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

var TransformGroup = {
	name: "TransformGroup",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TransformGroup />");
	},

	properties: ["Children"]
};

var TranslateTransform = {
	name: "TranslateTransform",

	parent: Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TranslateTransform />");
	},

	properties: [
		"X",
		"Y"
	]
};

var MediaAttribute = {
	name: "MediaAttribute",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MediaAttribute />");
	},

	properties: ["Value"]
};

var MediaElement = {
	name: "MediaElement",

	parent: UIElement,

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

var PathFigure = {
	name: "PathFigure",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathFigure />");
	},

	properties: [
		"IsClosed",
		"Segments",
		"StartPoint"
	]
};

var SilverlightPlugin = {
	name: "SilverlightPlugin",

	create: function (plugin) {
		return plugin;
	},

	properties: [
		"Accessibility",
		"ActualHeight",
		"ActualWidth",
		"Background",
		"EnableFramerateCounter",
		"EnableHtmlAccess",
		"EnableRedrawRegions",
		"FullScreen",
		"InitParams",
		"IsLoaded",
		"MaxFrameRate",
		"Root",
		"Source",
		"Windowless"
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

var SilverlightPluginContent = {
	name: "SilverlightPluginContent",

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

var Stroke = {
	name: "Stroke",

	parent: DependencyObject,

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

var StylusInfo = {
	name: "StylusInfo",

	parent: DependencyObject,

	properties: [
		"IsInverted",
		"DeviceType"
	]
};

var StylusPoint = {
	name: "StylusPoint",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StylusPoint />");
	},

	properties: [
		"PressureFactor",
		"X",
		"Y"
	]
};

var TextBlock = {
	name: "TextBlock",

	parent: UIElement,

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

var TimelineMarker = {
	name: "TimelineMarker",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineMarker />");
	},

	properties: [
		"Text",
		"Time",
		"Type"
	]
};

var EventArgs = {
	name: "EventArgs",

	parent: DependencyObject
};

var KeyboardEventArgs  = {
	name: "KeyboardEventArgs ",

	parent: EventArgs,

	properties: [
		"Ctrl",
		"Key",
		"PlatformKeyCode",
		"Shift"
	]
};

var MouseEventArgs  = {
	name: "MouseEventArgs ",

	parent: EventArgs,

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

var ErrorEventArgs  = {
	name: "ErrorEventArgs ",

	parent: EventArgs,

	properties: [
		"ErrorCode",
		"ErrorMessage",
		"ErrorType"
	]
};

var ParserErrorEventArgs  = {
	name: "ParserErrorEventArgs ",

	parent: ErrorEventArgs,

	properties: [
		"CharPosition",
		"LineNumber",
		"XamlFile",
		"XmlAttribute",
		"XmlElement"
	]
};

var RuntimeErrorEventArgs  = {
	name: "RuntimeErrorEventArgs ",

	parent: ErrorEventArgs,

	properties: [
		"CharPosition",
		"LineNumber",
		"MethodName"
	]
};
