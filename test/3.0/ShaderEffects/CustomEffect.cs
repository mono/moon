using System;
using System.Windows.Media.Effects;

namespace ShaderEffects
{
	public class CustomEffect : ShaderEffect
	{
		public CustomEffect ()
		{
			PixelShader ps = new PixelShader ();
			ps.UriSource = new Uri ("CustomEffect.ps", UriKind.Relative);

			this.PixelShader = ps;
		}
	}
}
