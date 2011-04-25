#if ANDROID_HACK

using System;
using System.IO;
using System.Text;
using System.Runtime.InteropServices;

namespace Mono {
    internal static class AndroidLogger {
	[DllImport ("/system/lib/liblog.so")]
        static extern void __android_log_print (AndroidLogLevel level, string appname, string format, string args, IntPtr zero);

	public static void Log (AndroidLogLevel level, string appname, string log) {
	    __android_log_print (level, appname, "%s", log, IntPtr.Zero);
	}
    }

    internal enum AndroidLogLevel {
	Unknown,
	Default,
	Verbose,
	Debug,
	Info,
	Warn,
	Error,
	Fatal,
	Silent
    }

    internal class AndroidLogTextWriter : TextWriter {
	StringBuilder bufferedLine;

	public AndroidLogTextWriter () {
	    bufferedLine = new StringBuilder();
	}

	void FlushLineToLog () {
	    AndroidLogger.Log (AndroidLogLevel.Debug, "moonlight", bufferedLine.ToString());
	    bufferedLine = new StringBuilder();
	}
	
	public override void Write (bool value) {
	    bufferedLine.Append (value);
	}

	public override void Write (char value) {
	    bufferedLine.Append (value);
	}

	public override void Write (char [] buffer) {
	    bufferedLine.Append (buffer);
	}

	public override void Write (char [] buffer, int index, int count) {
	    bufferedLine.Append (buffer, index, count);
	}

	public override void Write (decimal value) {
	    bufferedLine.Append (value);
	}

	public override void Write (double value) {
	    bufferedLine.Append (value);
	}

	public override void Write (int value) {
	    bufferedLine.Append (value);
	}

	public override void Write (long value) {
	    bufferedLine.Append (value);
	}

	public override void Write (object value) {
	    bufferedLine.Append (value);
	}

	public override void Write (float value) {
	    bufferedLine.Append (value);
	}

	public override void Write (string value) {
	    bufferedLine.Append (value);
	}

	public override void Write (string format, object arg0) {
	    bufferedLine.AppendFormat (format, arg0);
	}

	public override void Write (string format, object arg0, object arg1) {
	    bufferedLine.AppendFormat (format, arg0, arg1);
	}

#if !NET_2_1
	public override void Write (string format, object arg0, object arg1, object arg2) {
	    bufferedLine.AppendFormat (format, arg0, arg1, arg2);
	}
#endif

	public override void Write (string format, params object [] arg) {
	    bufferedLine.AppendFormat (format, arg);
	}

	public override void Write (uint value) {
	    bufferedLine.Append (value);
	}

	public override void Write (ulong value) {
	    bufferedLine.Append (value);
	}

	public override void WriteLine () {
	    FlushLineToLog ();
	}

	public override void WriteLine (bool value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (char value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (char [] buffer) {
	    Write (buffer);
	    FlushLineToLog();
	}

	public override void WriteLine (char [] buffer, int index, int count) {
	    Write (buffer, index, count);
	    FlushLineToLog();
	}

	public override void WriteLine (decimal value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (double value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (int value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (long value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (object value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (float value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (string value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (string format, object arg0) {
	    Write(format, arg0);
	    FlushLineToLog();
	}

	public override void WriteLine (string format, object arg0, object arg1) {
	    Write(format, arg0, arg1);
	    FlushLineToLog();
	}

#if !NET_2_1
	public override void WriteLine (string format, object arg0, object arg1, object arg2) {
	    Write(format, arg0, arg1, arg2);
	    FlushLineToLog();
	}
#endif

	public override void WriteLine (string format, params object [] arg) {
	    Write(format, arg);
	    FlushLineToLog();
	}

	public override void WriteLine (uint value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override void WriteLine (ulong value) {
	    Write(value);
	    FlushLineToLog();
	}

	public override Encoding Encoding {
	    get { return Encoding.UTF8; }
	}

    }
}
#endif
