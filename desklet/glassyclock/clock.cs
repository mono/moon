/*
 * clock.cs: Glassy clock desklet.
 *
 * Author:
 *   Everaldo Canuto (ecanuto@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Desklets
{
	public class Clock : Canvas 
	{
		RotateTransform secondsHand;
		RotateTransform minuteHand;
		RotateTransform hourHand;

		public void PageLoaded (object o, EventArgs e)
		{
			secondsHand = FindName ("secondsHand") as RotateTransform;
			minuteHand  = FindName ("minuteHand")  as RotateTransform;
			hourHand    = FindName ("hourHand")    as RotateTransform;

			if (secondsHand == null || minuteHand == null || hourHand == null)
				return;

			DateTime now = DateTime.Now;

			secondsHand.Angle = now.Second * 6;
			minuteHand.Angle  = now.Minute * 6;
			hourHand.Angle    = now.Hour * 30;
		}
	}
}
