using System;
using DebugLog;
using System.Windows;

namespace DebugLog.Extensions
{
	public static class UIElementExtension
	{
		public static void Log (this UIElement item, object message)
		{
			DebugLog.Log.WriteLine (item.ToString () + ":" + message.ToString ());
		}

		public static void Log (this UIElement item, string format, params object [] args)
		{
			string message = String.Format (format, args);
			DebugLog.Log.WriteLine (item.ToString () + ":" + message);
		}
	}
}
