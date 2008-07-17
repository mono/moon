# Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
#
# Contact:
#   Moonlight List (moonlight-list@lists.ximian.com)
# 
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation
# files (the "Software"), to deal in the Software without
# restriction, including without limitation the rights to use,
# copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following
# conditions:
# 
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

import System
import Gtk
import Gtk.Moonlight
import System.Windows
import System.Windows.Media
import System.Windows.Media.Animation
import System.Windows.Controls
import System.Windows.Shapes
import System.IO

class MyWindow(Window):

  _canvas as Canvas
  _currentId as int
  _silver as GtkSilver
  _setup as bool

  def constructor():
    _currentId = 1
    _setup = true

    super ("Photo browser")
    DeleteEvent += OnDeleteEvent
    ExposeEvent += OnExposeEvent

    vbox = VBox (false, 12)
    buttonBox = HButtonBox ()
    button1 = Button ("Previous")
    button2 = Button ("Next")
    buttonBox.Add (button1)
    buttonBox.Add (button2)

    button2.Clicked += OnNextClicked
    button1.Clicked += OnPrevClicked
   
    _silver = GtkSilver (320, 220)
    _canvas = Canvas ()
    _silver.Attach (_canvas)

    Add (vbox)
    vbox.PackStart (_silver)
    vbox.PackEnd (buttonBox)
    _silver.SetSizeRequest (320, 240)
    BorderWidth = 10

  private def CreateRectangleAtPosition (fileName as string, position as double, name as string):
    rectangle = Rectangle ()
    rectangle.Width = 300
    rectangle.Height = 200
    rectangle.RadiusX = 20
    rectangle.RadiusY = 20
    rectangle.SetValue (DependencyObject.NameProperty, name)

    solidBrush = SolidColorBrush ()
    solidBrush.Color = Color.FromArgb (255, 255, 255, 255)
    
    fillBrush = ImageBrush ()
  
    rectangle.Fill = fillBrush
    rectangle.Stroke = solidBrush
    rectangle.StrokeThickness = 2.0
    
    _canvas.Children.Add (rectangle)
    _canvas.SetLeft (rectangle, position)
    _canvas.SetTop (rectangle, 10.0)
    fillBrush.ImageSource = Uri(fileName, UriKind.Relative)

    rectangle.MouseEnter += OnHover
    rectangle.MouseLeave += OnUnHover

  private def CreateNewAnimationForName (name as string, pos as double):
    sb = Storyboard ()
      
    sb.SetValue (Storyboard.TargetNameProperty, name)
    sb.SetValue (Storyboard.TargetPropertyProperty, "(Canvas.Left)")
    animation = DoubleAnimationUsingKeyFrames ()
    xaml = String.Format ('<SplineDoubleKeyFrame KeyTime="0:0:1" Value="{0}" KeySpline="0.5,0.0 0.5, 1.0" />', pos)
    frame = _silver.CreateFromString (xaml, false)
    animation.KeyFrames.Add (frame)
    sb.Children.Add (animation)
    _canvas.Resources.Add (sb)
    sb.Begin ()

  private def OnExposeEvent ():
    if (_setup):
      pos = 10
      currentId = 1
      for file in Directory.GetFiles ("./", "*.jpg"):
        CreateRectangleAtPosition (file, pos, currentId.ToString ())
        currentId += 1
        pos = 400
      
    _setup = false

  private def OnDeleteEvent():
    Gtk.Application.Quit ()

  private def OnNextClicked ():
    if (_currentId == 5):
      return

    if (_currentId > 0):
      CreateNewAnimationForName (_currentId.ToString (), -400)

    _currentId += 1
    CreateNewAnimationForName (_currentId.ToString (), 10)

  private def OnPrevClicked ():
    if (_currentId == 1):
      return

    CreateNewAnimationForName (_currentId.ToString (), 400)

    _currentId -= 1
    CreateNewAnimationForName (_currentId.ToString (), 10)

  private def OnHover (sender as Rectangle):
    solid = sender.Stroke as SolidColorBrush
    solid.Color = Color.FromArgb (255, 255, 0, 0)

  private def OnUnHover (sender as Rectangle):
    solid = sender.Stroke as SolidColorBrush
    solid.Color = Color.FromArgb (255, 255, 255, 255)


Gtk.Application.Init ()
GtkSilver.Init ()
myWindow = MyWindow ()
myWindow.ShowAll ()
Gtk.Application.Run ()
