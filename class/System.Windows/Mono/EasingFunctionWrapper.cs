
using System;
using System.Text;
using System.Windows;
using System.Windows.Media.Animation;

namespace Mono {

	internal delegate double EasingFunctionCallback (double normalizedTime);

	internal class EasingFunctionWrapper : EasingFunctionBase {

		IEasingFunction function;

		public EasingFunctionWrapper (IEasingFunction function)
		{
			this.function = function;
			this.EasingMode = EasingMode.EaseIn;
		}

		public override double EaseInCore (double normalizedTime)
		{
			return function.Ease (normalizedTime);
		}

		public static EasingFunctionCallback CreateSafeEasingFunction (EasingFunctionCallback function)
		{
			return delegate (double normalizedTime) {
				try {
					return function (normalizedTime);
				}
				catch (Exception ex) {
					try {
						if (IsPlugin ())
							ReportException (ex);
						else
							Console.WriteLine ("Moonlight: Unhandled exception in EasingHelper.CreateSafeEasingFunction: {0}",
									   ex);
					}
					catch {
						// Ignore
					}

					return 0.0;
				}
			};
		}


		internal static bool IsPlugin () {
			return System.Windows.Interop.PluginHost.Handle != IntPtr.Zero;
		}

		internal static void ReportException (Exception ex)
		{
			String msg = ex.Message;
			System.Text.StringBuilder sb = new StringBuilder (ex.GetType ().FullName);
			sb.Append (": ").Append (ex.Message);
			String details = sb.ToString ();
			String[] stack_trace = ex.StackTrace.Split (new [] { Environment.NewLine }, StringSplitOptions.None);

			NativeMethods.plugin_instance_report_exception (System.Windows.Interop.PluginHost.Handle, msg, details, stack_trace, stack_trace.Length);
		}
	}
}