clock1  - FillBehavior default (="HoldEnd").  no bells and whistles. 1 animation.
clock2  - same as clock1, but FillBehavior = "Stop" applied to storyboard
clock3  - same as clock1, but FillBehavior = "Stop" applied to animation.
clock4  - 2 animations, one moving right and the next (with a later BeginTime) moving down.
clock5  - same as clock1, but with default Duration (=Automatic)
clock6  - same as clock1, but with AutoReverse=true on the animation
clock7  - same as clock4, but with AutoReverse=true on the storyboard
clock8  - same as clock1, but with a speed ratio of 0.5 (so it take 20 seconds to run)
clock9  - same as clock8, but with a speed ratio of 2 (so it take 5 seconds to run)
clock10 - same as clock1, but the block should turn black at the beginning (immediately)
clock11 - same as clock10, but with the instantaneous animation having FillBehavior=Stop
clock12 - same as clock1, but it takes 2 seconds, and the block makes the journey over and over.
clock13 - same as clock12, but specifies the RepeatBehavior as a double.
clock14 - same as clock12, but specifies the RepeatBehavior as a timespan/duration.
clock15 - tests a non-zero BeginTime on a storyboard along with a 0 BeginTime.
clock16 - tests a storyboard with a longer duration than its consituent Animation, with a FillBehavior of Stop (animations have the default fill behavior)
clock17 - tests 2 storyboards with keyframe animations in it (one Double with linear keyframes, one Color with discrete keyframes).
clock18 - tests 2 storyboards, 1 with keyframes (DiscreteDouble).
clock19 - tests 2 storyboards, 1 with keyframes (SplineDouble)
clock20 - tests BeginTime set on the storyboard with a repeatforever animation inside it.
clock21 - tests BeginTime set on the storyboard, which also has repeatforever set on it.
clock22 - tests BeginTime set on the animation, with repeatforever on the parent storyboard.
clock23 - same as clock1, but FillBehavior = "Stop" applied to storyboard and animation
clock24 - same as clock1, but storyboard Duration < animation Duration
clock25 - same as clock2, but storyboard Duration < animation Duration
clock26 - same as clock3, but storyboard Duration < animation Duration
clock27 - same as clock23, but storyboard Duration < animation Duration
clock28 - same as clock6, but storyboard Duration < animation Duration
clock29 - same as clock28, but storyboard behavior is Stop
clock30 - same as clock6, but AutoReverse=true on storyboard
clock31 - same as clock14, but additionally the clock has AutoReverse="True"
clock32 - same as clock7, but very very fast. Showcases the jaggyness/align problem on end
clock33 - autoreversed storyboard with speed ratio > 1.0.
clock34 - storyboard containing keyframes + normal animation extending that. Take from Popfly.
clock35 - same as 33 but very fast (jaggy problems)
clock36 - scaling with FillBehavior = HoldEnd using SplineDoubleKeyFrame
clock37 - Animation starting at 00:00:01 and repeating forever 
clock38 - Same as 37 but with animation keyframes
clock40 - HoldEnd storyboard with Stop animation inside with SpeedBahavior (precision issues)
clock41 - An animation with Duration="00:00:00" and FillBehavior="Stop"

