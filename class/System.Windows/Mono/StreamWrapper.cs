/*
 * StreamWrapper.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.IO;
using System.Runtime.InteropServices;

namespace Mono
{	
	internal class StreamWrapper
	{
		private Stream stream;
		private ManagedStreamCallbacks? callbacks;
		private GCHandle handle;
		
		public StreamWrapper (Stream stream)
		{
			this.stream = stream;
		}
		
		~StreamWrapper ()
		{
			// TODO:
			// Add a native call to clear out the handle
			// native code has.
			handle.Free ();
		}
		
		public ManagedStreamCallbacks GetCallbacks ()
		{
			ManagedStreamCallbacks callbacks;
			if (!this.callbacks.HasValue) {
				handle = GCHandle.Alloc (this);
				callbacks.handle = Helper.GCHandleToIntPtr (handle);
				callbacks.CanRead = CanRead;
				callbacks.CanSeek = CanSeek;
				callbacks.Length = Length;
				callbacks.Position = Position;
				callbacks.Read = Read;
				callbacks.Seek = Seek;
				this.callbacks = callbacks;
			}
			return this.callbacks.Value;
		}
		
		public static bool CanSeek (IntPtr handle) {
			StreamWrapper wrapper = (StreamWrapper) Helper.GCHandleFromIntPtr (handle).Target;
			Console.WriteLine ("StreamWrapper.CanSeek");
			bool result = wrapper.stream.CanSeek;
			Console.WriteLine ("StreamWrapper.CanSeek: {0}", result);
			return result;
		}
		
		public static bool CanRead (IntPtr handle) {
			StreamWrapper wrapper = (StreamWrapper) Helper.GCHandleFromIntPtr (handle).Target;
			Console.WriteLine ("StreamWrapper.CanRead");
			bool result = wrapper.stream.CanRead;
			Console.WriteLine ("StreamWrapper.CanRead: {0}", result);
			return result;
		}

		public static long Length (IntPtr handle) {
			StreamWrapper wrapper = (StreamWrapper) Helper.GCHandleFromIntPtr (handle).Target;
			Console.WriteLine ("StreamWrapper.Length");
			long result = wrapper.stream.Length;
			Console.WriteLine ("StreamWrapper.Length: {0}", result);
			return result;
		}
		
		public static long Position (IntPtr handle) {
			StreamWrapper wrapper = (StreamWrapper) Helper.GCHandleFromIntPtr (handle).Target;
			Console.WriteLine ("StreamWrapper.Position");
			long result = wrapper.stream.Position;
			Console.WriteLine ("StreamWrapper.Position: {0}", result);
			return result;
		}
		
		public static int Read (IntPtr handle, [In (), Out (), MarshalAs (UnmanagedType.LPArray, SizeParamIndex=3)] byte [] buffer, int offset, int count)
		{
			StreamWrapper wrapper = (StreamWrapper) Helper.GCHandleFromIntPtr (handle).Target;
			Console.WriteLine ("StreamWrapper.Read ({0}, {1}, {2})", buffer, offset, count);
			int result = wrapper.stream.Read (buffer, offset, count);
			Console.WriteLine ("StreamWrapper.Read ({0}, {1}, {2}): {3}", buffer, offset, count, result);
			for (int i = 0; i < global::System.Math.Min (count / 4, 4); i++) {
				Console.WriteLine (" #{0} {1:X} {2:X} {3:X} {4:X}", i, buffer [i * 4], buffer [i * 4 + 1], buffer [i * 4 + 2], buffer [i * 4 + 3]);
			}
			return result;
		}
		
		public static void Seek (IntPtr handle, long offset, SeekOrigin origin)
		{
			StreamWrapper wrapper = (StreamWrapper) Helper.GCHandleFromIntPtr (handle).Target;
			Console.WriteLine ("StreamWrapper.Seek ({0}, {1})", offset, origin);
			wrapper.stream.Seek (offset, origin);
			Console.WriteLine ("StreamWrapper.Seek ({0}, {1}) [Done]", offset, origin);
		}
	}
}
