DependencyObject = {
	name: "DependencyObject",

	properties: ["Name"],

	methods: ["FindName", "GetHost", "GetValue", "SetValue", "Equals"],

	events: []
};

Accessibility = {
	name: "Accessibility",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.accessibility;
	},

	properties: ["ActionDescription", "Description", "Title"],

	events: ["PerformAction"]
};

Downloader = {
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

UIElement = {
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

Canvas = {
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

Collection = {
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

ColorKeyFrameCollection = {
	name: "ColorKeyFrameCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><SplineColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames;
	}
};

DoubleKeyFrameCollection = {
	name: "DoubleKeyFrameCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleKeyFrameCollection />");
	}
};

GeometryCollection = {
	name: "GeometryCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GeometryCollection />");
	}
};

GradientStopCollection = {
	name: "GradientStopCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GradientStopCollection />");
	}
};

MediaAttributeCollection = {
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

PathFigureCollection = {
	name: "PathFigureCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathFigureCollection />");
	}
};

PathSegmentCollection = {
	name: "PathSegmentCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathSegmentCollection />");
	}
};

PointKeyFrameCollection = {
	name: "PointKeyFrameCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointKeyFrameCollection />");
	}
};

ResourceDictionary = {
	name: "ResourceDictionary",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ResourceDictionary />");
	}
};

StrokeCollection = {
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

StylusPointCollection = {
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

TimelineCollection = {
	name: "TimelineCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineCollection />");
	}
};

TimelineMarkerCollection = {
	name: "TimelineMarkerCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineMarkerCollection />");
	}
};

TransformCollection = {
	name: "TransformCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TransformCollection />");
	}
};

TriggerActionCollection = {
	name: "TriggerActionCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerActionCollection />");
	}
};

TriggerCollection = {
	name: "TriggerCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerCollection />");
	}
};

UIElementCollection = {
	name: "UIElementCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Canvas />").children;
	}
};

KeyFrame = {
	name: "KeyFrame",

	parent: DependencyObject,

	properties: ["KeyTime", "Value"]
};

ColorKeyFrame = {
	name: "ColorKeyFrame",

	parent: KeyFrame
}

DoubleKeyFrame = {
	name: "DoubleKeyFrame",

	parent: KeyFrame
}

PointKeyFrame = {
	name: "PointKeyFrame",

	parent: KeyFrame
}

DiscreteColorKeyFrame = {
	name: "DiscreteColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><DiscreteColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

DiscreteDoubleKeyFrame = {
	name: "DiscreteDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><DiscreteDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

DiscretePointKeyFrame = {
	name: "DiscretePointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><DiscretePointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

LinearColorKeyFrame = {
	name: "LinearColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><LinearColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

LinearDoubleKeyFrame = {
	name: "LinearDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><LinearDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

LinearPointKeyFrame = {
	name: "LinearPointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><LinearPointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	}
}

SplineColorKeyFrame = {
	name: "SplineColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><SplineColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

SplineDoubleKeyFrame = {
	name: "SplineDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><SplineDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

SplinePointKeyFrame = {
	name: "SplinePointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><SplinePointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: ["KeySpline"]
}

Timeline = {
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

TimelineGroup = {
	name: "TimeLineGroup",

	parent: Timeline,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineGroup />");
	},

	properties: ["Children"]
};

Storyboard = {
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

Animation = {
	name: "Animation",

	parent: Timeline,

	properties: ["By", "From", "To"]
};

ColorAnimation = {
	name: "ColorAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimation />");
	}
};

ColorAnimationUsingKeyFrames = {
	name: "ColorAnimationUsingKeyFrames",

	parent: ColorAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

DoubleAnimation = {
	name: "DoubleAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimation />");
	}
};

DoubleAnimationUsingKeyFrames = {
	name: "DoubleAnimationUsingKeyFrames",

	parent: DoubleAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

PointAnimation = {
	name: "PointAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimation />");
	}
};

PointAnimationUsingKeyFrames = {
	name: "PointAnimationUsingKeyFrames",

	parent: PointAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames />");
	},

	properties: ["KeyFrames"]
};

Brush = {
	name: "Brush",

	parent: DependencyObject,

	properties: [
		"Opacity",
		"RelativeTransform",
		"Transform"
	]
};

ImageBrush = {
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

SolidColorBrush = {
	name: "SolidColorBrush",

	parent: Brush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<SolidColorBrush />");
	},

	properties: ["Color"]
};

VideoBrush = {
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

GradientBrush = {
	name: "GradientBrush",

	parent: Brush,

	properties: [
		"ColorInterpolationMode",
		"GradientStops",
		"MappingMode",
		"SpreadMethod"
	]
};

LinearGradientBrush = {
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

RadialGradientBrush = {
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

PathSegment = {
	name: "PathSegment",

	parent: DependencyObject,
};

ArcSegment = {
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

BezierSegment = {
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

LineSegment = {
	name: "LineSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LineSegment />");
	},

	properties: ["Point"]
};

PolyBezierSegment = {
	name: "PolyBezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyBezierSegment />");
	},

	properties: [
		"Points"
	]
};

PolyLineSegment = {
	name: "PolyLineSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyLineSegment />");
	},

	properties: ["Points"]
};

PolyQuadraticBezierSegment = {
	name: "PolyQuadraticBezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyQuadraticBezierSegment />");
	},

	properties: ["Points"]
};

QuadraticBezierSegment = {
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

TriggerAction = {
	name: "TriggerAction",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerAction />");
	}
};

BeginStoryboard = {
	name: "BeginStoryboard",

	parent: TriggerAction,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<BeginStoryboard />");
	},

	properties: ["Storyboard"]
};

DrawingAttributes = {
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

Shape = {
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

Ellipse = {
	name: "Ellipse",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Ellipse />");
	}
};

Path = {
	name: "Path",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Path />");
	},

	properties: ["Data"]
};

Polygon = {
	name: "Polygon",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Polygon />");
	},

	properties: ["FillRule"]
};

Polyline = {
	name: "Polyline",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Polyline />");
	},

	properties: ["FillRule"]
};

Rectangle = {
	name: "Rectangle",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Rectangle />");
	}
};

Geometry = {
	name: "Geometry",

	parent: DependencyObject,

	properties: ["Transform"]
};

GeometryGroup = {
	name: "GeometryGroup",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GeometryGroup />");
	},

	properties: ["Children"]
};

EllipseGeometry = {
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

LineGeometry = {
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

PathGeometry = {
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

RectangleGeometry = {
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

EventTrigger = {
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

Glyphs = {
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

GradientStop = {
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

Image = {
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

InkPresenter = {
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

KeySpline = {
	name: "KeySpline",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<KeySpline />");
	}
};

Line = {
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

LineBreak = {
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

Run = {
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

Matrix = {
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

Transform = {
	name: "Transform",

	parent: DependencyObject
};

MatrixTransform = {
	name: "MatrixTransform",

	parent: Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MatrixTransform />");
	},

	properties: ["Matrix"]
};

RotateTransform = {
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

ScaleTransform = {
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

SkewTransform = {
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

TransformGroup = {
	name: "TransformGroup",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TransformGroup />");
	},

	properties: ["Children"]
};

TranslateTransform = {
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

MediaAttribute = {
	name: "MediaAttribute",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MediaAttribute />");
	},

	properties: ["Value"]
};

MediaElement = {
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

PathFigure = {
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

SilverlightPlugin = {
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

SilverlightPluginContent = {
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

Stroke = {
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

StylusInfo = {
	name: "StylusInfo",

	parent: DependencyObject,

	properties: [
		"IsInverted",
		"DeviceType"
	]
};

StylusPoint = {
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

TextBlock = {
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

TimelineMarker = {
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

EventArgs = {
	name: "EventArgs",

	parent: DependencyObject
};

KeyboardEventArgs  = {
	name: "KeyboardEventArgs ",

	parent: EventArgs,

	properties: [
		"Ctrl",
		"Key",
		"PlatformKeyCode",
		"Shift"
	]
};

MouseEventArgs  = {
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

ErrorEventArgs  = {
	name: "ErrorEventArgs ",

	parent: EventArgs,

	properties: [
		"ErrorCode",
		"ErrorMessage",
		"ErrorType"
	]
};

ParserErrorEventArgs  = {
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

RuntimeErrorEventArgs  = {
	name: "RuntimeErrorEventArgs ",

	parent: ErrorEventArgs,

	properties: [
		"CharPosition",
		"LineNumber",
		"MethodName"
	]
};
