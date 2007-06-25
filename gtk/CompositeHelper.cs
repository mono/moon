//
// CompositeHelper.cs
//
// Simple helper methods for doing rgba drawing
//
// Author: Larry Ewing <lewing@novell.com>
//
//
using System;
using System.Runtime.InteropServices;
using Cairo;
using Gtk;
using Gdk;

static class CompositeHelper {
		[DllImport("libgdk-win32-2.0-0.dll")]
		static extern IntPtr gdk_cairo_create (IntPtr raw);

		[DllImport("libgdk-win32-2.0-0.dll")]
		static extern IntPtr gdk_screen_get_rgba_colormap (IntPtr screen);

		[DllImport("libgdk-win32-2.0-0.dll")]
		static extern void gdk_cairo_region (IntPtr ctx, IntPtr region);

		public static Cairo.Context Create (Gdk.Drawable drawable)
		{
			Cairo.Context ctx = new Cairo.Context (gdk_cairo_create (drawable.Handle));
			if (ctx == null) 
				throw new Exception ("Couldn't create Cairo Graphics!");
			
			return ctx;
		}
		
		public static void Region (Cairo.Context ctx, Gdk.Region region)
		{
			gdk_cairo_region (ctx.Handle, region.Handle);
		}		

		public static Colormap GetRgbaColormap (Screen screen)
		{
			try {
				IntPtr raw_ret = gdk_screen_get_rgba_colormap (screen.Handle);
				Gdk.Colormap ret = GLib.Object.GetObject(raw_ret) as Gdk.Colormap;
				return ret;
			} catch {
				Gdk.Visual visual = Gdk.Visual.GetBestWithDepth (32);
				if (visual != null) {
					Gdk.Colormap cmap = new Gdk.Colormap (visual, false);
					System.Console.WriteLine ("fallback");
					return cmap;
				}
			}
			return null;
		}

		public static bool SetRgbaColormap (Widget w)
		{
			Gdk.Colormap cmap = GetRgbaColormap (w.Screen);

			if (cmap != null) {
				w.Colormap = cmap;
				return true;
			}

			return false;
		}
}
