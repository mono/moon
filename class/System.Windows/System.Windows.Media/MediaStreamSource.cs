////
//// MediaStreamSource.cs
////
//// Authors:
////   Rolf Bjarne Kvinge (rkvinge@novell.com)
////
//// Copyright 2008 Novell, Inc.
////
//// Permission is hereby granted, free of charge, to any person obtaining
//// a copy of this software and associated documentation files (the
//// "Software"), to deal in the Software without restriction, including
//// without limitation the rights to use, copy, modify, merge, publish,
//// distribute, sublicense, and/or sell copies of the Software, and to
//// permit persons to whom the Software is furnished to do so, subject to
//// the following conditions:
//// 
//// The above copyright notice and this permission notice shall be
//// included in all copies or substantial portions of the Software.
//// 
//// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
//// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
//// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
//// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
//// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
////
//

using System;
using System.Collections.Generic;

namespace System.Windows.Media
{
	public abstract class MediaStreamSource
	{
		protected MediaStreamSource ()
		{
		}
		
		protected abstract void CloseMedia ();
		protected abstract void GetDiagnosticAsync (MediaStreamSourceDiagnosticKind diagnosticKind);
		protected abstract void GetSampleAsync (MediaStreamType mediaStreamType);
		protected abstract void OpenMediaAsync ();
		protected abstract void SeekAsync (long seekToTime);
		protected abstract void SwitchMediaStreamAsync (MediaStreamDescription mediaStreamDescription);

		protected void ErrorOccurred(string errorDescription)
		{
			throw new NotImplementedException ();
		}
		
		protected void ReportGetDiagnosticCompleted (MediaStreamSourceDiagnosticKind diagnosticKind, long diagnosticValue)
		{
			throw new NotImplementedException ();
		}
		
		protected void ReportGetSampleCompleted (MediaStreamSample mediaStreamSample)
		{
			throw new NotImplementedException ();
		}
		
		protected void ReportGetSampleProgress (double bufferingProgress)
		{
			throw new NotImplementedException ();
		}
		
		protected void ReportOpenMediaCompleted (IDictionary<MediaSourceAttributesKeys, string> mediaStreamAttributes, IEnumerable<MediaStreamDescription> availableMediaStreams)
		{
			throw new NotImplementedException ();
		}
		
		protected void ReportSeekCompleted (long timeSeekedTo)
		{
			throw new NotImplementedException ();
		}
		
		protected void ReportSwitchMediaStreamCompleted (MediaStreamDescription mediaStreamDescription)
		{
			throw new NotImplementedException ();
		}
		
	}
}
