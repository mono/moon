/*
 * fullscreen.h: the xaml for the fullscreen message.
 *
 * Author:
 *  Rolf Bjarne Kvinge  (RKvinge@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#define FULLSCREEN_MESSAGE \
"<Canvas\n" \
"	xmlns=\"http://schemas.microsoft.com/client/2007\"\n" \
"	xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\"\n" \
"	Width=\"500\" Height=\"68\"\n" \
"	x:Name=\"FullScreenMessage\"\n" \
"	Visibility=\"Visible\"\n" \
"	Opacity=\"1\"\n" \
"	>\n" \
"	<Canvas.Triggers>\n" \
"		<EventTrigger RoutedEvent=\"Canvas.Loaded\">\n" \
"			<BeginStoryboard>\n" \
"				<Storyboard x:Name=\"FadeOut\">\n" \
"					<DoubleAnimationUsingKeyFrames BeginTime=\"00:00:00\" Storyboard.TargetName=\"FullScreenMessage\" Storyboard.TargetProperty=\"(UIElement.Opacity)\">\n" \
"						<SplineDoubleKeyFrame KeyTime=\"00:00:04.6000000\" Value=\"1\"/>\n" \
"						<SplineDoubleKeyFrame KeyTime=\"00:00:05\" Value=\"0\"/>\n" \
"					</DoubleAnimationUsingKeyFrames>\n" \
"				</Storyboard>\n" \
"			</BeginStoryboard>\n" \
"		</EventTrigger>\n" \
"	</Canvas.Triggers>\n" \
"	<Canvas.RenderTransform>\n" \
"		<TranslateTransform X=\"0\" Y=\"0\"/>\n" \
"	</Canvas.RenderTransform>\n" \
"	<Rectangle Width=\"500\" Height=\"68\" Stroke=\"#FF000000\" StrokeThickness=\"0\" RadiusX=\"8\" RadiusY=\"8\">\n" \
"		<Rectangle.Fill>\n" \
"			<LinearGradientBrush EndPoint=\"0,1\" StartPoint=\"0,0\">\n" \
"				<GradientStop Color=\"#FF242323\" Offset=\"0\"/>\n" \
"				<GradientStop Color=\"#FF515151\" Offset=\"1\"/>\n" \
"			</LinearGradientBrush>\n" \
"		</Rectangle.Fill>\n" \
"	</Rectangle>\n" \
"	<TextBlock Width=\"458\" Height=\"27\" Canvas.Left=\"124\" Canvas.Top=\"14\" TextWrapping=\"Wrap\" FontWeight=\"Normal\" Foreground=\"#FFFFFFFF\" FontSize=\"14\" Text=\"Press &quot;Esc&quot; to exit full-screen mode.\" x:Name=\"message\"/>\n" \
"	<TextBlock Width=\"458\" Height=\"20\" Canvas.Left=\"193\" Canvas.Top=\"34\" Text=\"&lt;url: file://&gt;\" TextWrapping=\"Wrap\" x:Name=\"url\" Foreground=\"#FFC8C4C4\" FontSize=\"13\" FontStretch=\"Normal\"/>\n" \
"</Canvas>\n" \
""

