DependencyObject = {
	name: "DependencyObject",

	isAbstract: true,

	properties: ["Name"],

	methods: [
		"FindName", "GetHost", "GetValue", "SetValue", "Equals"
	],

	events: []
};

Accessibility = {
	name: "Accessibility",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.accessibility;
	},

	properties: ["ActionDescription", "Description", "Title"],

	methods: [],

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

	isAbstract: true,

	parent: DependencyObject,

	properties: [],
	methods: [],
	events: []
};

Canvas = {
	name: "Canvas",

	parent: UIElement,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Canvas />");
	},

	properties: [
		"Background",
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Children",
		"Clip",
		"Cursor",
		"Height",
		"IsHitTestVisible",
		"Name",
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
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
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

Collection = {
	name: "Collection",

	isAbstract: true,

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
	},

	properties: ["Count", "Name"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

DoubleKeyFrameCollection = {
	name: "DoubleKeyFrameCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleKeyFrameCollection />");
	},

	properties: ["Count", "Name"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

GeometryCollection = {
	name: "GeometryCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GeometryCollection />");
	},

	properties: ["Count", "Name"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

GradientStopCollection = {
	name: "GradientStopCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GradientStopCollection />");
	},

	properties: ["Count", "Name"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

MediaAttributeCollection = {
	name: "MediaAttributeCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MediaElement />").attributes;
	},

	properties: ["Count"],

	methods: [
		"GetItem",
		"GetItemByName"
	],

	events: []
};

PathFigureCollection = {
	name: "PathFigureCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathFigureCollection />");
	},

	properties: ["Count", "Name"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

PathSegmentCollection = {
	name: "PathSegmentCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathSegmentCollection />");
	},

	properties: ["Count", "Name"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

PointKeyFrameCollection = {
	name: "PointKeyFrameCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointKeyFrameCollection />");
	},

	properties: ["Count", "Name"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

ResourceDictionary = {
	name: "ResourceDictionary",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ResourceDictionary />");
	},

	properties: ["Count"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

StrokeCollection = {
	name: "StrokeCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StrokeCollection />");
	},

	properties: ["Count"],

	methods: [
		"Add",
		"Clear",
		"GetBounds",
		"GetItem",
		"GetValue",
		"HitTest",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

StylusPointCollection = {
	name: "StylusPointCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StylusPointCollection />");
	},

	properties: ["Count"],

	methods: [
		"Add",
		"AddStylusPoints",
		"Clear",
		//"GetBounds",
		"GetItem",
		"GetValue",
		//"HitTest",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

TimelineCollection = {
	name: "TimelineCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineCollection />");
	},

	properties: ["Count"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

TimelineMarkerCollection = {
	name: "TimelineMarkerCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineMarkerCollection />");
	},

	properties: ["Count"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

TransformCollection = {
	name: "TransformCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TransformCollection />");
	},

	properties: ["Count"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

TriggerActionCollection = {
	name: "TriggerActionCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerActionCollection />");
	},

	properties: ["Count"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

TriggerCollection = {
	name: "TriggerCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerCollection />");
	},

	properties: ["Count"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

UIElementCollection = {
	name: "UIElementCollection",

	parent: Collection,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Canvas />").children;
	},

	properties: ["Count", "Name"],

	methods: [
		"Add",
		"Clear",
		"GetItem",
		"GetValue",
		"Insert",
		"Remove",
		"RemoveAt",
		"SetValue"
	],

	events: []
};

KeyFrame = {
	name: "KeyFrame",

	isAbstract: true,

	parent: DependencyObject,

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

ColorKeyFrame = {
	name: "ColorKeyFrame",

	isAbstract: true,

	parent: KeyFrame,

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

DoubleKeyFrame = {
	name: "DoubleKeyFrame",

	isAbstract: true,

	parent: KeyFrame,

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

PointKeyFrame = {
	name: "PointKeyFrame",

	isAbstract: true,

	parent: KeyFrame,

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

DiscreteColorKeyFrame = {
	name: "DiscreteColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><DiscreteColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

DiscreteDoubleKeyFrame = {
	name: "DiscreteDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><DiscreteDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

DiscretePointKeyFrame = {
	name: "DiscretePointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><DiscretePointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

LinearColorKeyFrame = {
	name: "LinearColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><LinearColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

LinearDoubleKeyFrame = {
	name: "LinearDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><LinearDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

LinearPointKeyFrame = {
	name: "LinearPointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><LinearPointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

SplineColorKeyFrame = {
	name: "SplineColorKeyFrame",

	parent: ColorKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames><SplineColorKeyFrame /></ColorAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeySpline",
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

SplineDoubleKeyFrame = {
	name: "SplineDoubleKeyFrame",

	parent: DoubleKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames><SplineDoubleKeyFrame /></DoubleAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeySpline",
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

SplinePointKeyFrame = {
	name: "SplinePointKeyFrame",

	parent: PointKeyFrame,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames><SplinePointKeyFrame /></PointAnimationUsingKeyFrames>").keyFrames.getItem (0);
	},

	properties: [
		"KeySpline",
		"KeyTime",
		"Name",
		"Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
}

Timeline = {
	name: "TimeLine",

	isAbstract: true,

	parent: DependencyObject,

	properties: [
		"AutoReverse",
		"BeginTime",
		"Duration",
		"FillBehavior",
		"Name",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

TimelineGroup = {
	name: "TimeLineGroup",

	parent: Timeline,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TimelineGroup />");
	},

	properties: [
		"Children",
		"Name"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

Storyboard = {
	name: "Storyboard",

	parent: Timeline,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Storyboard />");
	},

	properties: [
		"AutoReverse",
		"BeginTime",
		"Children",
		"Duration",
		"FillBehavior",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty"
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

	isAbstract: true,

	parent: Timeline,

	properties: [
		"AutoReverse",
		"BeginTime",
		"By",
		"Duration",
		"FillBehavior",
		"From",
		"Name",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty",
		"To"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

ColorAnimation = {
	name: "ColorAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimation />");
	},

	properties: [
		"AutoReverse",
		"BeginTime",
		"By",
		"Duration",
		"FillBehavior",
		"From",
		"Name",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty",
		"To"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

ColorAnimationUsingKeyFrames = {
	name: "ColorAnimationUsingKeyFrames",

	parent: ColorAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ColorAnimationUsingKeyFrames />");
	},

	properties: [
		"AutoReverse",
		"BeginTime",
		"Duration",
		"FillBehavior",
		"KeyFrames",
		"Name",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty",
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

DoubleAnimation = {
	name: "DoubleAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimation />");
	},

	properties: [
		"AutoReverse",
		"BeginTime",
		"By",
		"Duration",
		"FillBehavior",
		"From",
		"Name",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty",
		"To"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

DoubleAnimationUsingKeyFrames = {
	name: "DoubleAnimationUsingKeyFrames",

	parent: DoubleAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<DoubleAnimationUsingKeyFrames />");
	},

	properties: [
		"AutoReverse",
		"BeginTime",
		"Duration",
		"FillBehavior",
		"KeyFrames",
		"Name",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty",
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

PointAnimation = {
	name: "PointAnimation",

	parent: Animation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimation />");
	},

	properties: [
		"AutoReverse",
		"BeginTime",
		"By",
		"Duration",
		"FillBehavior",
		"From",
		"Name",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty",
		"To"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

PointAnimationUsingKeyFrames = {
	name: "PointAnimationUsingKeyFrames",

	parent: PointAnimation,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PointAnimationUsingKeyFrames />");
	},

	properties: [
		"AutoReverse",
		"BeginTime",
		"Duration",
		"FillBehavior",
		"KeyFrames",
		"Name",
		"RepeatBehavior",
		"SpeedRatio",
		"StoryBoard.TargetName",
		"StoryBoard.TargetProperty",
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

Brush = {
	name: "Brush",

	isAbstract: true,

	parent: DependencyObject,

	properties: [
		"Name",
		"Opacity",
		"RelativeTransform",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
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
		"Name",
		"Opacity",
		"RelativeTransform",
		"Stretch",
		"Transform"
	],

	methods: ["GetValue", "SetSource", "SetValue"],

	events: ["DownloadProgressChanged"]
};

SolidColorBrush = {
	name: "SolidColorBrush",

	parent: Brush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<SolidColorBrush />");
	},

	properties: [
		"Color",
		"Name",
		"Opacity",
		"RelativeTransform",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

VideoBrush = {
	name: "VideoBrush",

	parent: Brush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<VideoBrush />");
	},

	properties: [
		"Name",
		"Opacity",
		"RelativeTransform",
		"SourceName",
		"Stretch",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

GradientBrush = {
	name: "GradientBrush",

	isAbstract: true,

	parent: Brush,

	properties: [
		"ColorInterpolationMode",
		"GradientStops",
		"MappingMode",
		"Name",
		"Opacity",
		"RelativeTransform",
		"SpreadMethod",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

LinearGradientBrush = {
	name: "LinearGradientBrush",

	parent: GradientBrush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LinearGradientBrush />");
	},

	properties: [
		"ColorInterpolationMode",
		"EndPoint",
		"GradientStops",
		"MappingMode",
		"Name",
		"Opacity",
		"RelativeTransform",
		"SpreadMethod",
		"StartPoint",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

RadialGradientBrush = {
	name: "RadialGradientBrush",

	parent: GradientBrush,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<RadialGradientBrush />");
	},

	properties: [
		"Center",
		"ColorInterpolationMode",
		"GradientOrigin",
		"GradientStops",
		"MappingMode",
		"Name",
		"Opacity",
		"RadiusX",
		"RadiusY",
		"RelativeTransform",
		"SpreadMethod",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

PathSegment = {
	name: "PathSegment",

	isAbstract: true,

	parent: DependencyObject,

	properties: ["Name"],

	methods: ["GetValue", "SetValue"],

	events: []
};

ArcSegment = {
	name: "ArcSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<ArcSegment />");
	},

	properties: [
		"IsLargeArc",
		"Name",
		"Point",
		"RotationAngle",
		"Size",
		"SweepDirection"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

BezierSegment = {
	name: "BezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<BezierSegment />");
	},

	properties: [
		"Name",
		"Point1",
		"Point2",
		"Point3"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

LineSegment = {
	name: "LineSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LineSegment />");
	},

	properties: [
		"Name",
		"Point"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

PolyBezierSegment = {
	name: "PolyBezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyBezierSegment />");
	},

	properties: [
		"Name",
		"Points"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

PolyLineSegment = {
	name: "PolyLineSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyLineSegment />");
	},

	properties: [
		"Name",
		"Points"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

PolyQuadraticBezierSegment = {
	name: "PolyQuadraticBezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PolyQuadraticBezierSegment />");
	},

	properties: [
		"Name",
		"Points"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

QuadraticBezierSegment = {
	name: "QuadraticBezierSegment",

	parent: PathSegment,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<QuadraticBezierSegment />");
	},

	properties: [
		"Name",
		"Point1",
		"Point2"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

TriggerAction = {
	name: "TriggerAction",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TriggerAction />");
	},

	properties: [
		//"Count", // sdk doc says there's a count property, but apparently not
		"Name"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

BeginStoryboard = {
	name: "BeginStoryboard",

	parent: TriggerAction,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<BeginStoryboard />");
	},

	properties: ["Name", "Storyboard"],

	methods: ["GetValue", "SetValue"],

	events: []
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
		"Name",
		"OutlineColor",
		"Width"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

Shape = {
	name: "Shape",

	isAbstract: true,

	parent: DependencyObject,

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Fill",
		"Height",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Stretch",
		"Stroke",
		"StrokeDashArray",
		"StrokeDashCap",
		"StrokeDashOffset",
		"StrokeEndLineCap",
		"StrokeLineJoin",
		"StrokeMiterLimit",
		"StrokeStartLineCap",
		"StrokeThickness",
		"Tag",
		"Triggers",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
	]
};

Ellipse = {
	name: "Ellipse",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Ellipse />");
	},

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Fill",
		"Height",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Stretch",
		"Stroke",
		"StrokeDashArray",
		"StrokeDashCap",
		"StrokeDashOffset",
		"StrokeEndLineCap",
		"StrokeLineJoin",
		"StrokeMiterLimit",
		"StrokeStartLineCap",
		"StrokeThickness",
		"Tag",
		"Triggers",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
	]
};

Path = {
	name: "Path",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Path />");
	},

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Data",
		"Fill",
		"Height",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Stretch",
		"Stroke",
		"StrokeDashArray",
		"StrokeDashCap",
		"StrokeDashOffset",
		"StrokeEndLineCap",
		"StrokeLineJoin",
		"StrokeMiterLimit",
		"StrokeStartLineCap",
		"StrokeThickness",
		"Tag",
		"Triggers",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
	]
};

Polygon = {
	name: "Polygon",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Polygon />");
	},

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Fill",
		"FillRule",
		"Height",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Stretch",
		"Stroke",
		"StrokeDashArray",
		"StrokeDashCap",
		"StrokeDashOffset",
		"StrokeEndLineCap",
		"StrokeLineJoin",
		"StrokeMiterLimit",
		"StrokeStartLineCap",
		"StrokeThickness",
		"Tag",
		"Triggers",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
	]
};

Polyline = {
	name: "Polyline",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Polyline />");
	},

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Fill",
		"FillRule",
		"Height",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Stretch",
		"Stroke",
		"StrokeDashArray",
		"StrokeDashCap",
		"StrokeDashOffset",
		"StrokeEndLineCap",
		"StrokeLineJoin",
		"StrokeMiterLimit",
		"StrokeStartLineCap",
		"StrokeThickness",
		"Tag",
		"Triggers",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
	]
};

Rectangle = {
	name: "Rectangle",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Rectangle />");
	},

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Fill",
		"Height",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Stretch",
		"Stroke",
		"StrokeDashArray",
		"StrokeDashCap",
		"StrokeDashOffset",
		"StrokeEndLineCap",
		"StrokeLineJoin",
		"StrokeMiterLimit",
		"StrokeStartLineCap",
		"StrokeThickness",
		"Tag",
		"Triggers",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
	]
};

Geometry = {
	name: "Geometry",

	isAbstract: true,

	parent: DependencyObject,

	properties: ["Name", "Transform"],

	methods: ["GetValue", "SetValue"],

	events: []
};

GeometryGroup = {
	name: "GeometryGroup",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<GeometryGroup />");
	},

	properties: [
		"Children",
		"Name",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

EllipseGeometry = {
	name: "EllipseGeometry",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<EllipseGeometry />");
	},

	properties: [
		"Center",
		"Name",
		"RadiusX",
		"RadiusY",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

LineGeometry = {
	name: "LineGeometry",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<LineGeometry />");
	},

	properties: [
		"EndPoint",
		"Name",
		"StartPoint",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

PathGeometry = {
	name: "PathGeometry",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<PathGeometry />");
	},

	properties: [
		"Figures",
		"FillRule",
		"Name",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

RectangleGeometry = {
	name: "RectangleGeometry",

	parent: Geometry,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<RectangleGeometry />");
	},

	properties: [
		"Name",
		"RadiusX",
		"RadiusY",
		"Rect",
		"Transform"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

EventTrigger = {
	name: "EventTrigger",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<EventTrigger />");
	},

	properties: [
		"Actions",
		"Name",
		"RoutedEvent"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

Glyphs = {
	name: "Glyphs",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Glyphs />");
	},

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Fill",
		"FontRenderingEmSize",
		"FontUri",
		"Height",
		"Indices",
		"Name",
		"Opacity",
		"OpacityMask",
		"OriginX",
		"OriginY",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"StyleSimulations",
		"Tag",
		"Triggers",
		"UnicodeString",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
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
		"Name",
		"Offset"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

Image = {
	name: "Image",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Image />");
	},

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"DownloadProgress",
		"Height",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Source",
		"Stretch",
		"Tag",
		"Triggers",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"DownloadProgressChanged",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove",
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
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Children",
		"Clip",
		"Cursor",
		"Height",
		"IsHitTestVisible",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Strokes",
		"Tag",
		"Triggers",
		"Visibility",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
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

KeySpline = {
	name: "KeySpline",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<KeySpline />");
	},

	properties: [
		// "KeyTime",
		"Name",
		// "Value"
	],

	methods: ["GetValue", "SetValue"],

	events: []
};

Line = {
	name: "Line",

	parent: Shape,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<Line />");
	},

	properties: [
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"Fill",
		"Height",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Stretch",
		"Stroke",
		"StrokeDashArray",
		"StrokeDashCap",
		"StrokeDashOffset",
		"StrokeEndLineCap",
		"StrokeLineJoin",
		"StrokeMiterLimit",
		"StrokeStartLineCap",
		"StrokeThickness",
		"Tag",
		"Triggers",
		"Width",
		"X1",
		"X2",
		"Y1",
		"Y2"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
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
		"Name",
		"TextDecorations"
	],

	methods: [
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"SetValue"
	],

	events: []
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
		"Name",
		"TextDecorations"
	],

	methods: [
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"SetValue"
	],

	events: []
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
	],

	methods: [],

	events: []
};

Transform = {
	name: "Transform",

	isAbstract: true,

	parent: DependencyObject,

	properties: ["Name"],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
};

MatrixTransform = {
	name: "MatrixTransform",

	parent: Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MatrixTransform />");
	},

	properties: [
		"Matrix",
		"Name",
	],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
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
		"Name",
	],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
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
		"Name",
		"ScaleX",
		"ScaleY"
	],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
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
		"Name"
	],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
};

TransformGroup = {
	name: "TransformGroup",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TransformGroup />");
	},

	properties: ["Children", "Name"],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
};

TranslateTransform = {
	name: "TranslateTransform",

	parent: Transform,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TranslateTransform />");
	},

	properties: [
		"Name",
		"X",
		"Y"
	],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
};

MediaAttribute = {
	name: "MediaAttribute",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<MediaAttribute />");
	},

	properties: [
		"Name",
		"Value"
	],

	methods: [],

	events: []
};

MediaElement = {
	name: "MediaElement",

	parent: DependencyObject,

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
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"CurrentState",
		"Cursor",
		"DownloadProgress",
		"Height",
		"IsMuted",
		"Markers",
		"NaturalDuration",
		"NaturalVideoHeight",
		"NaturalVideoWidth",
		"Opacity",
		"OpacityMask",
		"Position",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Source",
		"Stretch",
		"Tag",
		"Triggers",
		"Volume",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"Pause",
		"Play",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetSource",
		"SetValue",
		"Stop"
	],

	events: [
		"BufferingProgressChanged",
		"CurrentStateChanged",
		"DownloadProgressChanged",
		"Loaded",
		"MarkerReached",
		"MediaEnded",
		"MediaFailed",
		"MediaOpened",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
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
		"Name",
		"Segments",
		"StartPoint"
	],

	methods: ["GetValue", "SetValue"],

	events: []
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
		"Name",
		"StylusPoints"
	],

	methods: [
		"GetBounds",
		"GetValue",
		"HitTest",
		"SetValue"
	],

	events: []
};

StylusInfo = {
	name: "StylusInfo",

	parent: DependencyObject,

	properties: [
		"IsInverted",
		"Name",
		"DeviceType"
	],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
};

StylusPoint = {
	name: "StylusPoint",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<StylusPoint />");
	},

	properties: [
		"Name",
		"PressureFactor",
		"X",
		"Y"
	],

	methods: [
		"GetValue",
		"SetValue"
	],

	events: []
};

TextBlock = {
	name: "TextBlock",

	parent: DependencyObject,

	create: function (plugin) {
		return plugin.content.createFromXaml ("<TextBlock />");
	},

	properties: [
		"ActualHeight",
		"ActualWidth",
		"Canvas.Left",
		"Canvas.Top",
		"Canvas.ZIndex",
		"Clip",
		"Cursor",
		"FontFamily",
		"FontSize",
		"FontStretch",
		"FontStyle",
		"FontWeight",
		"Foreground",
		"Height",
		"Name",
		"Opacity",
		"OpacityMask",
		"RenderTransform",
		"RenderTransformOrigin",
		"Resources",
		"Text",
		"TextDecorations",
		"TextWrapping",
		"Tag",
		"Triggers",
		"Visibility",
		"Width"
	],

	methods: [
		"AddEventListener",
		"CaptureMouse",
		"Equals",
		"FindName",
		"GetHost",
		"GetParent",
		"GetValue",
		"ReleaseMouseCapture",
		"RemoveEventListener",
		"SetFontSource",
		"SetValue"
	],

	events: [
		"Loaded",
		"MouseEnter",
		"MouseLeave",
		"MouseLeftButtonDown",
		"MouseLeftButtonUp",
		"MouseMove"
	]
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
	],

	methods: [],

	events: []
};

EventArgs = {
	name: "EventArgs",

	isAbstract: true,

	parent: DependencyObject,

	properties: [],

	methods: [],

	events: []
};

KeyboardEventArgs  = {
	name: "KeyboardEventArgs ",

	parent: EventArgs,

	properties: [
		"Ctrl",
		"Key",
		"PlatformKeyCode",
		"Shift"
	],

	methods: [],

	events: []
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
	],

	events: []
};

ErrorEventArgs  = {
	name: "ErrorEventArgs ",

	isAbstract: true,

	parent: EventArgs,

	properties: [
		"ErrorCode",
		"ErrorMessage",
		"ErrorType"
	],

	methods: [],

	events: []
};

ParserErrorEventArgs  = {
	name: "ParserErrorEventArgs ",

	parent: ErrorEventArgs,

	properties: [
		"CharPosition",
		"ErrorCode",
		"ErrorMessage",
		"ErrorType",
		"LineNumber",
		"XamlFile",
		"XmlAttribute",
		"XmlElement"
	],

	methods: [],

	events: []
};

RuntimeErrorEventArgs  = {
	name: "RuntimeErrorEventArgs ",

	parent: ErrorEventArgs,

	properties: [
		"CharPosition",
		"ErrorCode",
		"ErrorMessage",
		"ErrorType",
		"LineNumber",
		"MethodName"
	],

	methods: [],

	events: []
};
