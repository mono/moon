/*
 * plugin-spinner.h: the xaml for the fullscreen message.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2009 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#define PLUGIN_SPINNER \
"<Grid xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" " \
      "xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\">" \
  "<Grid.Resources>" \
    "<Style x:Key=\"rectProp\" TargetType=\"Rectangle\">" \
      "<Setter Property=\"Width\" Value=\"18\"/>" \
      "<Setter Property=\"Height\" Value=\"18\"/>" \
      "<Setter Property=\"Fill\" Value=\"#f0f8f8ff\"/>" \
      "<Setter Property=\"Stroke\" Value=\"DarkGray\"/>" \
      "<Setter Property=\"StrokeThickness\" Value=\"2\"/>" \
      "<Setter Property=\"RadiusX\" Value=\"8\"/>" \
      "<Setter Property=\"RadiusY\" Value=\"8\"/>" \
    "</Style>" \
    "<ElasticEase x:Key=\"throbEase\"/>" \
    "<QuarticEase x:Key=\"throbOut\"/>" \
  "</Grid.Resources>" \
  "<Grid.Triggers>" \
    "<EventTrigger RoutedEvent=\"Grid.Loaded\">" \
      "<BeginStoryboard>" \
	"<Storyboard x:Name=\"Throb\" Duration=\"00:00:04\" RepeatBehavior=\"Forever\">" \
	  "<DoubleAnimationUsingKeyFrames Storyboard.TargetName=\"One\" Storyboard.TargetProperty=\"Height\" BeginTime=\"00:00:00\">" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:01\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:02\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	  "</DoubleAnimationUsingKeyFrames>" \
	  "<DoubleAnimationUsingKeyFrames Storyboard.TargetName=\"Two\" Storyboard.TargetProperty=\"Height\" BeginTime=\"00:00:00.5\">" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:01\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:02\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	  "</DoubleAnimationUsingKeyFrames>" \
	  "<DoubleAnimationUsingKeyFrames Storyboard.TargetName=\"Three\" Storyboard.TargetProperty=\"Height\" BeginTime=\"00:00:01\">" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:01\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:02\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:04\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:05\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame keytime=\"00:00:06\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	  "</DoubleAnimationUsingKeyFrames>" \
	  "<DoubleAnimationUsingKeyFrames Storyboard.TargetName=\"Four\" Storyboard.TargetProperty=\"Height\" BeginTime=\"00:00:01.5\">" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:01\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:02\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:04\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:05\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame keytime=\"00:00:06\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	  "</DoubleAnimationUsingKeyFrames>" \
	  "<DoubleAnimationUsingKeyFrames Storyboard.TargetName=\"Five\" Storyboard.TargetProperty=\"Height\" BeginTime=\"-00:00:02\">" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:01\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:02\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:04\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:05\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame keytime=\"00:00:06\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	  "</DoubleAnimationUsingKeyFrames>" \
	  "<DoubleAnimationUsingKeyFrames Storyboard.TargetName=\"Six\" Storyboard.TargetProperty=\"Height\" BeginTime=\"-00:00:01.5\">" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:01\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:02\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:04\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:05\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame keytime=\"00:00:06\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	  "</DoubleAnimationUsingKeyFrames>" \
	  "<DoubleAnimationUsingKeyFrames Storyboard.TargetName=\"Seven\" Storyboard.TargetProperty=\"Height\" BeginTime=\"-00:00:01\">" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:01\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:02\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:04\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:05\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame keytime=\"00:00:06\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	  "</DoubleAnimationUsingKeyFrames>" \
	  "<DoubleAnimationUsingKeyFrames Storyboard.TargetName=\"Eight\" Storyboard.TargetProperty=\"Height\" BeginTime=\"-00:00:00.5\">" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:01\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:02\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:04\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	    "<EasingDoubleKeyFrame KeyTime=\"00:00:05\" Value=\"22\" EasingFunction=\"{StaticResource throbEase}\"/>" \
	    "<EasingDoubleKeyFrame keytime=\"00:00:06\" Value=\"18\" EasingFunction=\"{StaticResource throbOut}\"/>" \
	  "</DoubleAnimationUsingKeyFrames>" \
	"</Storyboard>" \
      "</BeginStoryboard>" \
    "</EventTrigger>" \
  "</Grid.Triggers>" \
  "<Grid>" \
  "<Canvas Width=\"10\" Height=\"10\">" \
    "<Canvas.RenderTransform>" \
      "<TransformGroup>" \
      "<TranslateTransform X=\"5\" Y=\"5\"/>" \
      "<ScaleTransform ScaleX=\".66\" ScaleY=\".66\"/>" \
      "</TransformGroup>" \
    "</Canvas.RenderTransform>" \
    "<Rectangle x:Name=\"One\" Style=\"{StaticResource rectProp}\">" \
      "<Rectangle.RenderTransform>" \
	"<TransformGroup>" \
	  "<TranslateTransform Y=\"33\" X=\"-9\"/>" \
	"</TransformGroup>" \
      "</Rectangle.RenderTransform>" \
    "</Rectangle>" \
    "<Rectangle x:Name=\"Two\" Style=\"{StaticResource rectProp}\">" \
      "<Rectangle.RenderTransform>" \
	"<TransformGroup>" \
	  "<TranslateTransform Y=\"33\" X=\"-9\"/>" \
	  "<RotateTransform Angle=\"45\"/>" \
	"</TransformGroup>" \
      "</Rectangle.RenderTransform>" \
    "</Rectangle>" \
    "<Rectangle x:Name=\"Three\" Style=\"{StaticResource rectProp}\">" \
      "<Rectangle.RenderTransform>" \
	"<TransformGroup>" \
	  "<TranslateTransform Y=\"33\" X=\"-9\"/>" \
	  "<RotateTransform Angle=\"90\"/>" \
	"</TransformGroup>" \
      "</Rectangle.RenderTransform>" \
    "</Rectangle>" \
    "<Rectangle x:Name=\"Four\" Style=\"{StaticResource rectProp}\">" \
      "<Rectangle.RenderTransform>" \
	"<TransformGroup>" \
	  "<TranslateTransform Y=\"33\" X=\"-9\"/>" \
	  "<RotateTransform Angle=\"135\"/>" \
	"</TransformGroup>" \
      "</Rectangle.RenderTransform>" \
    "</Rectangle>" \
    "<Rectangle x:Name=\"Five\" Style=\"{StaticResource rectProp}\">" \
      "<Rectangle.RenderTransform>" \
	"<TransformGroup>" \
	  "<TranslateTransform Y=\"33\" X=\"-9\"/>" \
	  "<RotateTransform Angle=\"180\"/>" \
	"</TransformGroup>" \
      "</Rectangle.RenderTransform>" \
    "</Rectangle>" \
    "<Rectangle x:Name=\"Six\" Style=\"{StaticResource rectProp}\">" \
      "<Rectangle.RenderTransform>" \
	"<TransformGroup>" \
	  "<TranslateTransform Y=\"33\" X=\"-9\"/>" \
	  "<RotateTransform Angle=\"225\"/>" \
	"</TransformGroup>" \
      "</Rectangle.RenderTransform>" \
    "</Rectangle>" \
    "<Rectangle x:Name=\"Seven\" Style=\"{StaticResource rectProp}\">" \
      "<Rectangle.RenderTransform>" \
	"<TransformGroup>" \
	  "<TranslateTransform Y=\"33\" X=\"-9\"/>" \
	  "<RotateTransform Angle=\"270\"/>" \
	"</TransformGroup>" \
      "</Rectangle.RenderTransform>" \
    "</Rectangle>" \
    "<Rectangle x:Name=\"Eight\" Style=\"{StaticResource rectProp}\">" \
      "<Rectangle.RenderTransform>" \
	"<TransformGroup>" \
	  "<TranslateTransform Y=\"33\" X=\"-9\"/>" \
	  "<RotateTransform Angle=\"315\"/>" \
	"</TransformGroup>" \
      "</Rectangle.RenderTransform>" \
    "</Rectangle>" \
  "</Canvas>" \
  "</Grid>" \
"</Grid>" \
""

