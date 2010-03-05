//
// PixelShader.cs
//
// Copyright 2009 Novell, Inc.
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
using System.Windows;
using System.Windows.Media;

namespace System.Windows.Media.Effects
{
	public abstract partial class ShaderEffect : Effect
	{
		static void UpdateShaderConstant (IntPtr native, int register, double x, double y, double z, double w)
		{
			NativeMethods.shader_effect_update_shader_constant (native, register, x, y, z, w);
		}

		protected static PropertyChangedCallback PixelShaderConstantCallback (int register)
		{
			return delegate (DependencyObject sender, DependencyPropertyChangedEventArgs e)
			       {
					double x = 1.0, y = 1.0, z = 1.0, w = 1.0;

					if (e.NewValue != null) {
						if (e.NewValue is double) {
							double v = (double) e.NewValue;
							x = v;
						} else if (e.NewValue is float) {
							float v = (float) e.NewValue;
							x = v;
						} else if (e.NewValue is Size) {
							Size v = (Size) e.NewValue;
							x = v.Width;
							y = v.Height;
						} else if (e.NewValue is Point) {
							Point v = (Point) e.NewValue;
							x = v.X;
							y = v.Y;
						} else if (e.NewValue is Color) {
							Color v = (Color) e.NewValue;
							x = v.R / 255.0;
							y = v.G / 255.0;
							z = v.B / 255.0;
							w = v.A / 255.0;
						}
					}

					UpdateShaderConstant (sender.native, register, x, y, z, w);
				};
		}

		static void UpdateShaderSampler (IntPtr native, int register, int samplingMode, IntPtr brush)
		{
			NativeMethods.shader_effect_update_shader_sampler (native, register, samplingMode, brush);
		}

		protected static PropertyChangedCallback PixelShaderSamplerCallback (int register)
		{
			return PixelShaderSamplerCallback (register, SamplingMode.Auto);
		}

		protected static PropertyChangedCallback PixelShaderSamplerCallback (int register,
										     SamplingMode samplingMode)
		{
			return delegate (DependencyObject sender, DependencyPropertyChangedEventArgs e)
			       {
					Brush brush = (Brush) e.NewValue;
					IntPtr input = brush == null ? IntPtr.Zero : brush.native;

					UpdateShaderSampler (sender.native, register, (int) samplingMode, input);
				};
		}

		protected static DependencyProperty RegisterPixelShaderSamplerProperty (string dpName,
											Type ownerType,
											int samplerRegisterIndex)
		{
			return DependencyProperty.Register (dpName, typeof(Brush), ownerType, new PropertyMetadata (null, PixelShaderSamplerCallback (samplerRegisterIndex, SamplingMode.Auto)));
		}

		protected static DependencyProperty RegisterPixelShaderSamplerProperty (string dpName,
											Type ownerType,
											int samplerRegisterIndex,
											SamplingMode samplingMode)
		{
			return DependencyProperty.Register (dpName, typeof(Brush), ownerType, new PropertyMetadata (null, PixelShaderSamplerCallback (samplerRegisterIndex, samplingMode)));
		}

		protected void UpdateShaderValue (DependencyProperty dp)
		{
			// Apparently this method just raises a PropertyChanged event without
			// the value actually changing.
			CustomDependencyProperty cdp;
			object obj = GetValue (dp);

			cdp = dp as CustomDependencyProperty;
			if (cdp != null && cdp.GetMetadata (null).property_changed_callback != null) {
				var args = new DependencyPropertyChangedEventArgs (obj, obj, dp);
				cdp.GetMetadata (null).property_changed_callback (this, args);
			}
		}
	}
}
