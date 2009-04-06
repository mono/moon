//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright 2007 Novell, Inc.
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

using Mono;

namespace System.Windows.Media.Animation 
{
	public abstract partial class EasingFunctionBase : DependencyObject, IEasingFunction
	{
		void Initialize ()
		{
			callback = EasingFunctionWrapper.CreateSafeEasingFunction (new EasingFunctionCallback (Ease));

			NativeMethods.easing_function_base_set_easing_function (native, callback);
		}

		public double Ease (double normalizedTime)
		{
			switch (EasingMode) {
			case EasingMode.EaseIn:
				return EaseInCore (normalizedTime);
			case EasingMode.EaseOut:
				return 1.0 - EaseInCore (1 - normalizedTime);
			case EasingMode.EaseInOut:
				return (normalizedTime <= 0.5

					? EaseInCore (normalizedTime * 2) * 0.5

					: 1.0 - EaseInCore ((1 - normalizedTime) * 2) * 0.5);
			}

			// XXX
			throw new Exception();
		}

		public abstract double EaseInCore (double normalizedTime);

		EasingFunctionCallback callback;
	}
}
