//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2008 Novell, Inc.
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
using System;

using System.Windows;
using System.Windows.Controls.Primitives;
using System.Windows.Automation.Peers;

namespace System.Windows.Controls{
	[TemplatePart (Name = ProgressBar.ProgressBarTrack, Type = typeof (FrameworkElement))]
	[TemplatePart (Name = ProgressBar.ProgressBarIndicator, Type = typeof (FrameworkElement))]
	[TemplateVisualState (Name = ProgressBar.DeterminateState, GroupName = ProgressBar.GroupName)]
	[TemplateVisualState (Name = ProgressBar.IndeterminateState, GroupName = ProgressBar.GroupName)]
	public partial class ProgressBar : RangeBase {

		const string ProgressBarTrack = "ProgressBarTrack";
		const string ProgressBarIndicator = "ProgressBarIndicator";
		const string DeterminateState = "Determinate";
		const string IndeterminateState = "Indeterminate";
		const string GroupName = "CommonStates";

		FrameworkElement progressbar_track;
		FrameworkElement progressbar_indicator;

		public static readonly DependencyProperty IsIndeterminateProperty =
			DependencyProperty.Register ("IsIndeterminate", typeof (bool), typeof (ProgressBar), null);

		public bool IsIndeterminate {
			get { return (bool) GetValue (IsIndeterminateProperty); }
			set { SetValue (IsIndeterminateProperty, value); }
		}

		public ProgressBar () : base ()
		{
			DefaultStyleKey = typeof (ProgressBar);
			SizeChanged += delegate { UpdateTrackLayout(); };
		}

		public override void OnApplyTemplate()
		{
			base.OnApplyTemplate();

			progressbar_track = GetTemplateChild (ProgressBarTrack) as FrameworkElement;
			progressbar_indicator = GetTemplateChild (ProgressBarIndicator) as FrameworkElement;

			ChangeVisualState(false);
		}

		protected override AutomationPeer OnCreateAutomationPeer ()
		{
			throw new NotImplementedException ();
		}

		protected override void OnMaximumChanged(double oldMaximum, double newMaximum)
		{
			base.OnMaximumChanged(oldMaximum, newMaximum);
			UpdateTrackLayout();
		}

		protected override void OnMinimumChanged(double oldMinimum, double newMinimum)
		{
			base.OnMinimumChanged(oldMinimum, newMinimum);
			UpdateTrackLayout();
		}

		protected override void OnValueChanged(double oldValue, double newValue)
		{
			base.OnValueChanged (oldValue, newValue);
			UpdateTrackLayout();
		}

		void ChangeVisualState(bool useTransitions)
		{
			VisualStateManager.GoToState(this, IsIndeterminate ? IndeterminateState : DeterminateState, useTransitions);
		}

		void UpdateTrackLayout()
		{
			if (IsIndeterminate)
				return;

			if (progressbar_indicator == null || progressbar_track == null)
				return;

			double maximum = Maximum;
			double minimum = Minimum;
			double value = Value;
			double multiplier = 1 - (maximum - value) / (maximum - minimum);

			progressbar_indicator.Width = multiplier * (ActualWidth - progressbar_track.Margin.Left - progressbar_track.Margin.Right);
		}
	}
}


