using System;

namespace Desklet.Weather
{
	public class App
	{
		public static void Main (string[] args)
		{
			Metar metar = new Metar (String.Join (" ", args));

			Console.WriteLine ("Station code: {0}", metar.StationCode);
			Console.WriteLine ("Temperature: {0}C, {1}F, {2}K",
					   metar.Temperature.Celcius, metar.Temperature.Fahrenheit,
					   metar.Temperature.Kelvin);
			Console.WriteLine ("Dew point: {0}C, {1}F, {2}K",
					   metar.DewPoint.Celcius, metar.DewPoint.Fahrenheit,
					   metar.DewPoint.Kelvin);
			Console.WriteLine ("Pressure: {0}mm/Hg, {1}hPa",
					   metar.PressureHg, metar.PressureMb);
			Console.WriteLine ("Visibility: range == {0}m ({1} km, {2} miles), direction == {3}, accuracy == {4}",
					   metar.Visibility.Range, metar.Visibility.Kilometers, metar.Visibility.Miles,
					   metar.Visibility.Direction, metar.Visibility.Accuracy);
			
			if (metar.Weather != null)
				Console.WriteLine ("Weather: intensity == {0}, descriptor == {1}, precipitation == {2}, " +
						   "obscuration == {3}, misc == {4}",
						   metar.Weather.Intensity, metar.Weather.Descriptor, metar.Weather.Precipitation,
						   metar.Weather.Obscuration, metar.Weather.Misc);
			if (metar.Wind != null)
				Console.WriteLine ("Wind: unit == {0}, gusts == {1}, direction == {2}, " +
						   "speed == {3} (knots: {4}, m/s {5}, km/h {6})",
						   metar.Wind.Unit, metar.Wind.Gusts, metar.Wind.Direction,
						   metar.Wind.Speed, metar.Wind.Knots, metar.Wind.MetersPerSecond,
						   metar.Wind.KilometersPerHour);
		}
	}
}
