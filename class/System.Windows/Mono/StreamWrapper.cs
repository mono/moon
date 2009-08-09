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
		internal Stream stream;
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
				callbacks.handle = GCHandle.ToIntPtr (handle);
				callbacks.CanRead = CanRead;
				callbacks.CanSeek = CanSeek;
				callbacks.Length = Length;
				callbacks.Position = Position;
				callbacks.Read = Read;
				callbacks.Write = Write;
				callbacks.Seek = Seek;
				this.callbacks = callbacks;
			}
			return this.callbacks.Value;
		}
		
		public static bool CanSeek (IntPtr handle) {
			StreamWrapper wrapper = (StreamWrapper) GCHandle.FromIntPtr (handle).Target;
			bool result = wrapper.stream.CanSeek;
			return result;
		}
		
		public static bool CanRead (IntPtr handle) {
			StreamWrapper wrapper = (StreamWrapper)GCHandle.FromIntPtr (handle).Target;
			bool result = wrapper.stream.CanRead;
			return result;
		}

		public static long Length (IntPtr handle) {
			StreamWrapper wrapper = (StreamWrapper) GCHandle.FromIntPtr (handle).Target;
			long result = wrapper.stream.Length;
			return result;
		}
		
		public static long Position (IntPtr handle) {
			StreamWrapper wrapper = (StreamWrapper) GCHandle.FromIntPtr (handle).Target;
			long result = wrapper.stream.Position;
			return result;
		}
		
		public static int Read (IntPtr handle, [In (), Out (), MarshalAs (UnmanagedType.LPArray, SizeParamIndex=3)] byte [] buffer, int offset, int count)
		{
			try {
				StreamWrapper wrapper = (StreamWrapper) GCHandle.FromIntPtr (handle).Target;
				int result = wrapper.stream.Read (buffer, offset, count);
				return result;
			}
			catch (Exception e) {
				Console.WriteLine (e);
				return -1;
			}
		}
		
		public static void Write (IntPtr handle, [In (), Out (), MarshalAs (UnmanagedType.LPArray, SizeParamIndex=3)] byte [] buffer, int offset, int count)
		{
			StreamWrapper wrapper = (StreamWrapper) GCHandle.FromIntPtr (handle).Target;
			wrapper.stream.Write (buffer, offset, count);
		}
		
		public static void Seek (IntPtr handle, long offset, SeekOrigin origin)
		{
			StreamWrapper wrapper = (StreamWrapper) GCHandle.FromIntPtr (handle).Target;
			wrapper.stream.Seek (offset, origin);
		}
	}
}
