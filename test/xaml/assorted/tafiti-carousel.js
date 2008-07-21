var SJ = null;
var master = null;
var carousel = null;
var imagesLength = 4;
var selection = 0;

function onLoad (sender)
{
	SJ = document.getElementById ("slControl").content;
	master = sender.findName("masterCanvas");
	console.log ("Loading...");
	createCarousel ();
	addImage ("icon-1.png", 55, 52, "0");
	addImage ("icon-2.png", 55, 52, "1");
	addImage ("icon-3.png", 55, 52, "2");
	addImage ("icon-4.png", 55, 52, "3");

	positionImages ();
	resizeAnimation (272, 92);

	fire ();
}

function positionImages ()
{
    // Images are positioned in a clockwise manner. Position 0 corresponds to 6 o'clock 
    // and increases clockwise to 1.5 as in:
    //  (back of carrousel)
    //        0.5
    //    0.25   0.75
    //         0 (or 1.5)
    // (front of carrousel)

    // positional offset between images
    var offset = 1.5 / imagesLength;
    
    // The currently "selected" image goes at position 0 (aka 1.5) and 
    // subsequent images are placed counter-clockwise if the direction is
    // clockwise (and clockwise if the direction is counterclockwise).
    if (this.direction == 0) {
        for (var i = 0; i < imagesLength; i++) {
            var n = (selection + i) % imagesLength;
            var pos = 1.5 - (i * offset);
            this.positionImage(n, pos);
        }
    } else {
        for (var i = 0; i < imagesLength; i++) {
            var n = (selection - i + imagesLength) % imagesLength;
            var pos = 1.5 - (i * offset);
            this.positionImage(n, pos);
        }    
    }
}

function positionImage (imageIndex, position) 
{
    position = Math.round(position * 1000) / 1000; // at most 3 digits after the decimal so WPF/E doesn't complain
    carousel.findName(generateName ("animateX", imageIndex)).BeginTime = "-0:0:" + position;
    carousel.findName(generateName ("animateY", imageIndex)).BeginTime = "-0:0:" + position;
    carousel.findName(generateName ("animateScaleX", imageIndex)).BeginTime = "-0:0:" + position;
    carousel.findName(generateName ("animateScaleY", imageIndex)).BeginTime = "-0:0:" + position;
}


function fire ()
{
    for (var i = 0; i < imagesLength; i++) {
	var sb = carousel.findName (generateName ("storyboard", i));
	sb.Begin ();
	}
}

function createCarousel ()
{    
	console.log ("Creating carousel...");
    var xaml = 
        '<Canvas \
            xmlns="http://schemas.microsoft.com/client/2007" \
            xmlns:x="http://schemas.microsoft.com/winfx/2006/xaml" \
            x:Name="carousel" Left="100" Top="100"> \
	    <Image Source="carousel.png" /> \
	    <Canvas.RenderTransform> \
                <TranslateTransform x:Name="%translate%" /> \
            </Canvas.RenderTransform> \
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
	carousel.Width = "272";
	carousel.Height = "92";
	master.Children.Add(carousel);
}

function resizeAnimation (width, height) 
{
    for (var i = 0; i < imagesLength; i++) {
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
                  Storyboard.TargetName="' + generateName ("scale", imageName) + '" \
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

    	carousel.findName(generateName ("scale", imageName)).CenterX = imageWidth / 2;
    	carousel.findName(generateName ("scale", imageName)).CenterY = imageHeight / 2;
}
