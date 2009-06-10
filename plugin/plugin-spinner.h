/*
 * plugin-spinner.h: the xaml for the loading spinner.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */
#define PLUGIN_SPINNER \
"<Grid xmlns=\"http://schemas.microsoft.com/winfx/2006/xaml/presentation\" " \
 "xmlns:x=\"http://schemas.microsoft.com/winfx/2006/xaml\" > " \
"<Canvas Width=\"400\" Height=\"400\" > " \
"	<Canvas Canvas.Top=\"200\" Canvas.Left=\"200\"> "\
"	  <Canvas.Triggers> " \
"	    <EventTrigger  RoutedEvent=\"Canvas.Loaded\" > "\
"	      <BeginStoryboard> "\
"		<Storyboard x:Name=\"sb_spin\" " \
"			    BeginTime=\"0:0:0\"" \
"			    RepeatBehavior=\"Forever\">" \
"		  <DoubleAnimation x:Name=\"spin\" " \
"				   Storyboard.TargetName=\"xform\"" \
"				   Storyboard.TargetProperty=\"Angle\"" \
"				   To=\"360\"" \
"				   Duration=\"00:00:01\"" \
"				   />" \
"		</Storyboard>" \
"	      </BeginStoryboard>" \
"	    </EventTrigger>" \
"	  </Canvas.Triggers>" \
"	  <Canvas.RenderTransform>" \
"	    <TransformGroup>" \
"	      <RotateTransform x:Name=\"xform\" Angle=\"0\" />" \
"	    </TransformGroup>" \
"	  </Canvas.RenderTransform>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".15\" ScaleY=\".15\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".2\" ScaleY=\".2\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"30\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".25\" ScaleY=\".25\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"60\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".3\" ScaleY=\".3\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"90\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".35\" ScaleY=\".35\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"120\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".4\" ScaleY=\".4\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"150\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".45\" ScaleY=\".45\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"180\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".5\" ScaleY=\".5\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"210\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".55\" ScaleY=\".55\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"240\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".6\" ScaleY=\".6\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"270\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".65\" ScaleY=\".65\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"300\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	  <Ellipse Fill=\"White\" Stroke=\"Gray\" Width=\"25\" Height=\"25\">" \
"	    <Ellipse.RenderTransform>" \
"	      <TransformGroup>" \
"		<ScaleTransform ScaleX=\".65\" ScaleY=\".65\" CenterX=\"12.5\" CenterY=\"12.5\" />" \
"		<RotateTransform Angle=\"330\" />" \
"	      </TransformGroup>" \
"	    </Ellipse.RenderTransform>" \
"	  </Ellipse>" \
"	</Canvas>" \
"</Canvas>" \
"</Grid>"
