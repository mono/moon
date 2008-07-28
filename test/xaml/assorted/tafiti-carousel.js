var SJ = null;
var master = null;
var carousel = null;
var imagesLength = 0;
var selection = 0;
var unveiling;
var target = 1;
var direction = 0;

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

	//fire ();
	//animateTurn (1.5 / 4);
	unveil ();
}

function animate (animation, handlerName)
{
	var sb = carousel.findName (animation);
	if (handlerName)
		sb.AddEventListener("Completed", handlerName);
	sb.Begin ();
}

function unveil () 
{

    // hide all images
    //for (var i = 0; i < imagesLength; i++) {
    //   var image = carousel.findName (generateName ("image", i));
    //    image.Opacity = 0;
    //ss}

    // setup our multi-part animation
    unveiling = {};

    unveiling.next = 0;
    unveiling.callback = unveilNextSpinStep;
    unveiling.storyboard = carousel.findName (generateName ("image", "0")).resources.GetItem(0);
    //unveiling.eventToken = unveiling.storyboard.addEventListener("Completed", this.unveiling.callback);
    //unveiling.onCompleted = onCompleted;
    
    animate ("fadeIn", unveilNextSpinStep);
}



function animateClockwiseCore() {

    if (direction == 1) {
        flipDirection();
    }

    // update position based on current selection
    positionImages();

    // It takes 1.5 seconds to animate the carrousel 360 degrees; thus to turn 1 image, run the animation 1.5 / (# of images) seconds
    var duration = 1.5 / imagesLength;
    animateTurn(duration);

    selection++;
    selection %= imagesLength;
}

function animateCounterclockwiseCore () 
{
    if (direction == 0) {
        flipDirection();
    }
    
    // update position based on current selection
    positionImages();

    // It takes 1.5 seconds to animate the carrousel 360 degrees; thus to turn 1 image, run the animation 1.5 / (# of images) seconds
    var duration = 1.5 / imagesLength;
    animateTurn(duration);

    selection += imagesLength - 1;
    selection %= imagesLength;
}

function flipDirection () 
{
    // WPF/E doesn't support running an animation in reverse (AutoReverse doesn't cut it).
    // As a workground, we set up the X coords of the animation based on the current direction.
    
    direction = direction ? 0 : 1;
    resizeAnimation(272, 92);
}


function animateClockwise () 
{
    target++;
    target %= imagesLength;
    TurnTowardTarget();
}

function animateCounterclockwise () 
{
    target += imagesLength - 1;
    target %= imagesLength;
    TurnTowardTarget();
}

function unveilNextSpinStep ()
{
	console.log ("Unveiling next spin step...");

    if (unveiling.next == imagesLength) {
        // cleanup
        //var onCompleted = this.unveiling.onCompleted;
        //this.unveiling.storyboard.removeEventListener("Completed", this.unveiling.eventToken);
        //this.unveiling = null;
        //delete this.unveiling;
        //if (onCompleted)
        //    onCompleted(this, null);
    }
    else {
        // unhide the next image
	var image = carousel.findName (generateName ("image", unveiling.next))
        image.Opacity = 1;
        
        unveiling.next++;

        // animate one turn
        animateClockwise();
    }
}

function animateTurn (duration) 
{
    duration = Math.round(duration * 1000) / 1000; // only 3 decimal places to keep WPF/E happy
    for (var i = 0; i < imagesLength; i++) {
        //var image = carousel.findName (generateName ("image", i));
        var storyBoard = carousel.findName(generateName ("storyboard", i));
        storyBoard["Duration"] = "0:0:" + duration; // assumes duration <= 60 seconds
        storyBoard.Begin();
    }
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
            x:Name="carousel" Canvas.Left="100" Canvas.Top="100"> \
	    <Image Source="carousel.png" /> \
	    <Canvas.RenderTransform> \
                <TranslateTransform x:Name="%translate%" /> \
            </Canvas.RenderTransform> \
            <Canvas.Resources> \
                <Storyboard x:Name="fadeIn"> \
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
        if (direction == 0) {
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

function onStoryboardCompleted ()
{
	console.log ("Completed, will turn toward target");
	TurnTowardTarget();
}

function generateName (keyword, number)
{
	return "image_" + keyword + "_" + number;
}

function TurnTowardTarget ()
{
    if (target != selection) {
        // Determine the best direction to turn the carousel
        var clockwiseOffset = (target + imagesLength - selection) % imagesLength;
        var turnClockwise = clockwiseOffset < imagesLength / 2;
        if (turnClockwise)
            animateClockwiseCore();
        else
            animateCounterclockwiseCore();
    } else {
	console.log ("Target matches selection, not turning!");
    }
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

	imagesLength += 1;


    	if (imagesLength == 1) {
        	// Hook the storyboard's 'Completed' event for just the first image.
        	// We use this event to turn the carousel multiple times.
        	var storyboard = element.resources.GetItem(0);
		storyboard.AddEventListener ("Completed", onStoryboardCompleted);
    	}
}
