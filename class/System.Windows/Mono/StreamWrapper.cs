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
				callbacks.Close = Close;
				this.callbacks = callbacks;
			}
			return this.callbacks.Value;
		}
		
		private static StreamWrapper GetWrapper (IntPtr handle)
		{
			return (StreamWrapper) GCHandle.FromIntPtr (handle).Target;
		}
		
		public static bool CanSeek (IntPtr handle)
		{
			try {
				return GetWrapper (handle).stream.CanSeek;
			} catch (Exception ex) {
				try {
					Console.WriteLine ("StreamWrapper.CanSeek: Unexpected exception: {0}", ex);
				} catch {
				}
				return false;
			}
		}
		
		public static bool CanRead (IntPtr handle)
		{
			try {
				return GetWrapper (handle).stream.CanRead;
			} catch (Exception ex) {
				try {
					Console.WriteLine ("StreamWrapper.CanRead: Unexpected exception: {0}", ex);
				} catch {
				}
				return false;
			}
		}

		public static long Length (IntPtr handle)
		{
			try {
				return GetWrapper (handle).stream.Length;
			} catch (Exception ex) {
				try {
					Console.WriteLine ("StreamWrapper.Length: Unexpected exception: {0}", ex);
				} catch {
				}
				return 0;
			}
		}
		
		public static long Position (IntPtr handle)
		{
			try {
				return GetWrapper (handle).stream.Position;
			} catch (Exception ex) {
				try {
					Console.WriteLine ("StreamWrapper.Position: Unexpected exception: {0}", ex);
				} catch {
				}
				return 0;
			}
		}
		
		public static int Read (IntPtr handle, [In (), Out (), MarshalAs (UnmanagedType.LPArray, SizeParamIndex=3)] byte [] buffer, int offset, int count)
		{
			try {
				StreamWrapper wrapper = GetWrapper (handle);
				int result = wrapper.stream.Read (buffer, offset, count);
				return result;
			}
			catch (Exception ex) {
				try {
					Console.WriteLine ("StreamWrapper.Read (): Unexpected exception: {0}", ex);
				} catch {
				}
				return -1;
			}
		}
		
		public static void Write (IntPtr handle, [In (), Out (), MarshalAs (UnmanagedType.LPArray, SizeParamIndex=3)] byte [] buffer, int offset, int count)
		{
			try {
				GetWrapper (handle).stream.Write (buffer, offset, count);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("StreamWrapper.Write (): Unexpected exception: {0}", ex);
				} catch {
				}
			}
		}
		
		public static void Seek (IntPtr handle, long offset, SeekOrigin origin)
		{
			try {
				GetWrapper (handle).stream.Seek (offset, origin);
			} catch (Exception ex) {
				try {
					Console.WriteLine ("StreamWrapper.Seek (): Unexpected exception: {0}", ex);
				} catch {
				}
			}
		}
		
		public static void Close (IntPtr handle)
		{
			try {
				StreamWrapper wrapper = GetWrapper (handle);
				
				wrapper.stream.Close ();
				wrapper.stream.Dispose ();
				wrapper.stream = null;
				
				wrapper.callbacks = null;
				wrapper.handle.Free ();
			} catch (Exception ex) {
				try {
					Console.WriteLine ("StreamWrapper.Close (): Unexpected exception: {0}", ex);
				} catch {
				}
			}
		}
	}
}
