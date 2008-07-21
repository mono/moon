var SJ = null;
var master = null;
var carousel = null;

function onLoad (sender)
{
	SJ = document.getElementById ("slControl").content;
	master = sender.findName("masterCanvas");
	console.log ("Loading...");
	createCarousel ();
	addImage ("icon-1.png", 32, 32, "0");
	resizeAnimation (272, 92);
	fire ();
}

function fire ()
{
	var sb = carousel.findName (generateName ("storyboard", "0"));
	sb.Begin ();
}

function createCarousel ()
{    
	console.log ("Creating carousel...");
    var xaml = 
        '<Canvas \
            xmlns="http://schemas.microsoft.com/client/2007" \
            xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" \
            x:Name="carousel" Left="100" Top="100"> \
	    <Ellipse Width="272" Height="92" Fill="Blue" /> \
            <Canvas.Resources> \
                <Storyboard x:Name="%fadeIn%"> \
                    <DoubleAnimation \
                         Storyboard.TargetName="carousel" \
                         Storyboard.TargetProperty="Opacity" \
                         From="0" To="1" Duration="0:0:1" /> \
                </Storyboard> \
            </Canvas.Resources> \
        </Canvas>';

	carousel = SJ.createFromXaml(xaml);
	master.Children.Add(carousel);
}

function resizeAnimation (width, height) 
{
	// IMAGE LENGTH
    for (var i = 0; i < 1; i++) {
        var animateX = carousel.findName (generateName ("animateX", i));
        if (this.direction == 0) {
            animateX.KeyFrames.GetItem(0).Value = width / 2;
            animateX.KeyFrames.GetItem(1).Value = 20;
            animateX.KeyFrames.GetItem(2).Value = width / 2 - 16;
            animateX.KeyFrames.GetItem(3).Value = width / 2 + 16;
            animateX.KeyFrames.GetItem(4).Value = width - 16;
            animateX.KeyFrames.GetItem(5).Value = width / 2;
        }
        else {
            animateX.KeyFrames.GetItem(0).Value = width / 2;
            animateX.KeyFrames.GetItem(1).Value = width - 16;
            animateX.KeyFrames.GetItem(2).Value = width / 2 + 16;
            animateX.KeyFrames.GetItem(3).Value = width / 2 - 16;
            animateX.KeyFrames.GetItem(4).Value = 20;
            animateX.KeyFrames.GetItem(5).Value = width / 2;
        }
        
        var animateY = carousel.findName (generateName ("animateY", i));
        animateY.KeyFrames.GetItem(0).Value = height - 10;
        animateY.KeyFrames.GetItem(1).Value = (height / 2) - 10;
        animateY.KeyFrames.GetItem(2).Value = 15;
        animateY.KeyFrames.GetItem(3).Value = 15;
        animateY.KeyFrames.GetItem(4).Value = (height / 2) - 10;
        animateY.KeyFrames.GetItem(5).Value = height - 10;
    }        
}

function generateName (keyword, number)
{
	return "image_" + keyword + "_" + number;
}


function addImage (imagePath, imageWidth, imageHeight, imageName) 
{
	console.log ("Loading %s as %s (%d x %d)", imagePath, imageName, imageWidth, imageHeight);	
   var xaml =
      '<Image x:Name="' + generateName ("image", imageName) + '" \
              Source="' + imagePath + '" \
              xmlns="http://schemas.microsoft.com/client/2007" \
              xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" \
              Canvas.Left="-26" Canvas.Top="-52"> \
        <Image.RenderTransform> \
            <TransformGroup> \
                <ScaleTransform Name="' + generateName ("scale", imageName) + '" ScaleX="1" ScaleY="1" /> \
                <TranslateTransform Name="' + generateName ("translate", imageName) + '" X="0" Y="0" /> \
            </TransformGroup> \
        </Image.RenderTransform> \
        <Image.Resources> \
            <Storyboard x:Name="' + generateName ("storyboard", imageName) + '"> \
                <DoubleAnimationUsingKeyFrames \
                  x:Name="' + generateName ("animateX", imageName) + '" \
                  Storyboard.TargetName="' + generateName ("translate", imageName) + '" \
                  Storyboard.TargetProperty="X" \
                  RepeatBehavior="Forever" \
                  > \
                  <SplineDoubleKeyFrame KeyTime="0:0:0.00" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:0.33" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:0.68" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:0.83" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:1.17" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:1.50" KeySpline="0.5,0.25 0.5,0.25" /> \
                </DoubleAnimationUsingKeyFrames> \
                <DoubleAnimationUsingKeyFrames \
                  x:Name="' + generateName ("animateY", imageName) + '" \
                  Storyboard.TargetName="' + generateName ("translate", imageName) + '" \
                  Storyboard.TargetProperty="Y" \
                  RepeatBehavior="Forever" \
                  > \
                  <SplineDoubleKeyFrame KeyTime="0:0:0.00" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:0.33" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:0.68" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:0.83" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:1.17" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame KeyTime="0:0:1.50" KeySpline="0.5,0.75 0.5,0.75" /> \
                </DoubleAnimationUsingKeyFrames> \
                <DoubleAnimationUsingKeyFrames \
                  x:Name="' + generateName ("animateScaleX", imageName) + '" \
                  Storyboard.TargetProperty="ScaleX" \
                  RepeatBehavior="Forever" \
                  > \
                  <SplineDoubleKeyFrame Value="1"    KeyTime="0:0:0.00" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame Value="0.95" KeyTime="0:0:0.33" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame Value="0.90" KeyTime="0:0:0.68" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame Value="0.90" KeyTime="0:0:0.83" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame Value="0.95" KeyTime="0:0:1.17" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame Value="1"    KeyTime="0:0:1.50" KeySpline="0.5,0.75 0.5,0.75" /> \
                </DoubleAnimationUsingKeyFrames> \
                <DoubleAnimationUsingKeyFrames \
                  x:Name="' + generateName ("animateScaleY", imageName) + '" \
                  Storyboard.TargetName="' + generateName ("scale", imageName) + '" \
                  Storyboard.TargetProperty="ScaleY" \
                  RepeatBehavior="Forever" \
                  > \
                  <SplineDoubleKeyFrame Value="1"    KeyTime="0:0:0.00" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame Value="0.95" KeyTime="0:0:0.33" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame Value="0.90" KeyTime="0:0:0.68" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame Value="0.90" KeyTime="0:0:0.83" KeySpline="0.5,0.75 0.5,0.75" /> \
                  <SplineDoubleKeyFrame Value="0.95" KeyTime="0:0:1.17" KeySpline="0.5,0.25 0.5,0.25" /> \
                  <SplineDoubleKeyFrame Value="1"    KeyTime="0:0:1.50" KeySpline="0.5,0.75 0.5,0.75" /> \
                </DoubleAnimationUsingKeyFrames> \
            </Storyboard> \
        </Image.Resources> \
      </Image>';

	var element = SJ.createFromXaml(xaml);
	carousel.Children.Add(element);

    	//SJ.findElement(names["scale"]).CenterX = imageWidth / 2;
    	//SJ.findElement(names["scale"]).CenterY = imageHeight / 2;



}
