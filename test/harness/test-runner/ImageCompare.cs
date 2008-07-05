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
// Copyright (c) 2007-2008 Novell, Inc.
//
// Authors:
//	Jackson Harper (jackson@ximian.com)
//


//
// FFT Image compare code is adapted from mcs/class/System.Drawing/Test/DrawingTest code
// written by Rafi Mizrahi and from Exocortex ImageFilter example code.
//
//

using System;
using System.IO;
using System.Drawing;
using System.Drawing.Imaging;

using Exocortex.DSP;

namespace MoonlightTests {

	public static class ImageCompare {

		// Default tolerance
		private static readonly double Tolerance = 2.0F;

		public static TestResult Compare (Test test, string result, string master)
		{
			string result_ext = Path.GetExtension (result).ToLower ();
			string master_ext = Path.GetExtension (master).ToLower ();

			if (result_ext != master_ext) {
				test.SetFailedReason (String.Format ("Files {0} and {1} do not have the same extension.", result, master));
				return TestResult.Ignore;
			}

			switch (result_ext) {
			case ".png":
				return ComparePngs (test, result, master);
			case ".tif":
			case ".tiff":
				return CompareTiffs (test, result, master);
			}

			test.SetToIgnore (String.Format ("Unknown file type: {0}", result_ext));
			return TestResult.Ignore;
		}

		private static TestResult ComparePngs (Test test, string result_path, string master_path)
		{
			TestResult res;
			
			if (!File.Exists (result_path)) {
				test.SetFailedReason (String.Format ("Can not find results file {0}", result_path));
				return TestResult.Fail;
			}

			if (!File.Exists (master_path)) {
				test.SetToIgnore (String.Format ("Can not find master file {0}", Path.GetFullPath (master_path)));
				return TestResult.Ignore;
			}

			using (Bitmap result = (Bitmap) Image.FromFile (result_path)) {
				using (Bitmap master = (Bitmap) Image.FromFile (master_path)) {
		
					res = CompareBitmaps (test, result, master);
		
					if (res == TestResult.Pass)
						res = EdgeCompare.CompareBitmaps (test, result, master);
		
				}
			}
			
			return res;
		}

		private static TestResult CompareTiffs (Test test, string result_path, string master_path)
		{
			if (!File.Exists (result_path)) {
				test.SetFailedReason (String.Format ("Can not find results file {0}", result_path));
				return TestResult.Fail;
			}

			if (!File.Exists (master_path)) {
				test.SetToIgnore (String.Format ("Can not find master file {0}", master_path));
				return TestResult.Ignore;
			}

			using (Bitmap result = (Bitmap) Image.FromFile (result_path)) {
				using (Bitmap master = (Bitmap) Image.FromFile (master_path)) {
					Guid [] result_frames = result.FrameDimensionsList;
					Guid [] master_frames = master.FrameDimensionsList;

					if (result_frames.Length != master_frames.Length) {
						test.SetFailedReason (String.Format ("Result and Master do not have the same number of layers: result: {0}, master: {1}",
									result_frames.Length, master_frames.Length));
						return TestResult.Fail;
					}

					for (int i = 0; i < result_frames.Length; i++) {
						FrameDimension result_dimension = new FrameDimension (result_frames [0]);
						FrameDimension master_dimension = new FrameDimension (master_frames [0]);
						int result_frames_count = result.GetFrameCount (result_dimension);
						int master_frames_count = master.GetFrameCount (master_dimension);
						
						if (result_frames_count != master_frames_count) {
							test.SetFailedReason (String.Format ("Result and Master do not have the same number of frames for frame dimension {0} ({1} vs {2})",
										i, result_frames_count, master_frames_count));
							return TestResult.Fail;
						}

						for (int f = 0; f < result_frames_count; f++) {
							result.SelectActiveFrame (result_dimension, f);
							master.SelectActiveFrame (master_dimension, f);
							
							TestResult res = CompareBitmaps (test, result, master);
							if (res != TestResult.Pass) {
								test.SetFailedReason (String.Format ("Layer {0} -- {1}", f, test.FailedReason));
								return res;
							}
							
							res = EdgeCompare.CompareBitmaps (test, result, master);
							
							if (res != TestResult.Pass) {
								test.SetFailedReason (String.Format ("Layer {0} -- {1}", f, test.FailedReason));
								return res;
							}
						}
					}
				}
			}
			
			return TestResult.Pass;
		}

		private static TestResult CompareBitmaps (Test test, Bitmap result, Bitmap master)
		{
			double result_norm = CalculateNorm (result);
			double master_norm = CalculateNorm (master);
			double difference = Math.Max (0, (Math.Abs (result_norm - master_norm) / (result_norm + master_norm + Double.Epsilon)));
			double tolerance = (test.ImageCompareTolerance != null ? (double) test.ImageCompareTolerance : Tolerance) / 100.0F;

			test.ImageDifference = difference;

			if (difference > tolerance) {
				test.SetFailedReason (String.Format ("Image difference was too great ({0:P})", difference));
				return TestResult.Fail;
			}
				
			return TestResult.Pass;
		}

		public static double CalculateNorm (Bitmap bitmap)
		{
			ComplexF [] matrix = GetImageFFTArray (bitmap);

			double norm = 0;
			int size_y = bitmap.Width;
			int size_z = bitmap.Height;
			for (int y = 1; y <= size_y; y++) {
				double norm_z = 0;
				for (int z=1; z<=size_z; z++) {
					ComplexF cur = matrix [(size_y - y) + size_y * (size_z - z)];
					norm_z += cur.GetModulusSquared ();
				}
				norm += norm_z;
			}

			return norm;
		}

		private static Bitmap MakeSquared (Bitmap bitmap)
		{
			Size correct_size = new Size ((int) Math.Pow (2, Math.Ceiling (Math.Log (bitmap.Width, 2))),
					(int) Math.Pow (2, Math.Ceiling (Math.Log (bitmap.Height, 2))));

			if (bitmap.Size != correct_size)
				bitmap = new Bitmap (bitmap, correct_size);

			return bitmap;
		}

		private static unsafe ComplexF[] GetImageFFTArray (Bitmap bitmap)
		{
			Bitmap squared = MakeSquared (bitmap);
			bool dispose = (object) squared != (object) bitmap;
			bitmap = squared;
			
			float scale = 1F / (float) Math.Sqrt (bitmap.Width * bitmap.Height);
			ComplexF [] data = new ComplexF [bitmap.Width * bitmap.Height];
			Rectangle rect = new Rectangle (0, 0, bitmap.Width, bitmap.Height);
			BitmapData bitmap_data = bitmap.LockBits (rect, ImageLockMode.ReadOnly, PixelFormat.Format32bppArgb);

			int* color_data = (int *) bitmap_data.Scan0.ToPointer ();
			for (int i = 0; i < bitmap.Width * bitmap.Height; i++)  {
				Color c = Color.FromArgb (color_data [i]);
				data [i].Re = ((float) c.R + (float) c.G + (float) c.B) / (3f * 256f);
			}
			bitmap.UnlockBits (bitmap_data);

			Fourier.FFT3 (data, bitmap.Width, bitmap.Height, 1, FourierDirection.Forward);
			
			for (int i = 0; i < data.Length; i++)
				data [i] *= scale;
			
			if (dispose)
				bitmap.Dispose ();
			
			return data;
		}
		
		public static string CreateMosaicFromTiff (string tiff_path)
		{
			int num_images = 0;
			string result_path = String.Concat (tiff_path, ".png");
			
			using (Bitmap tiff = (Bitmap) Image.FromFile (tiff_path)) {
				Guid [] tiff_frames = tiff.FrameDimensionsList;
				for (int i = 0; i < tiff_frames.Length; i++) {
					FrameDimension tiff_dimension = new FrameDimension (tiff_frames [0]);
					int frames_count = tiff.GetFrameCount (tiff_dimension);
	
					for (int f = 0; f < frames_count; f++)
						num_images++;
				}
	
				if (num_images == 1) {
					using (Bitmap result = new Bitmap (tiff.Width, tiff.Height)) {
						using (Graphics g = Graphics.FromImage (result)) {
							g.DrawImage (tiff, 0, 0);
							result.Save (result_path, ImageFormat.Png);
							return result_path;
						}
					}
				}
	
				
				int border_width = 2;
				int x = border_width;
				int y = border_width;
				int images_per_row = Math.Max (num_images / 4, 1);			
	
				using (Bitmap result = new Bitmap (tiff.Width + border_width * 5, (images_per_row * (tiff.Height / 4)) + (border_width * (images_per_row + 1)))) {
					using (Graphics g = Graphics.FromImage (result)) {
						g.Clear (Color.Black);
				
						for (int i = 0; i < tiff_frames.Length; i++) {
							FrameDimension tiff_dimension = new FrameDimension (tiff_frames [0]);
							int frames_count = tiff.GetFrameCount (tiff_dimension);
				
							for (int f = 0; f < frames_count; f++) {
								tiff.SelectActiveFrame (tiff_dimension, f);
				
								g.DrawImage (tiff, x, y, tiff.Width / 4, tiff.Height / 4);
				
								x += tiff.Width / 4 + border_width;
								if (x >= tiff.Width) {
									x = border_width;
									y += tiff.Height / 4 + border_width;
								}
									
							}
						}
									
						result.Save (result_path, ImageFormat.Png);
					}
				}
			}
			return result_path;
		}
	}
}



