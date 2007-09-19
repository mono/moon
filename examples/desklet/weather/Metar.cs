/*
 * Metar.cs: Moonlight Desklets Weather Desklet METAR parser.
 *
 * Author:
 *   Marek Habersack (mhabersack@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 * METAR format description:
 *
 *
 */
using System;
using System.Collections.Generic;
using System.Text.RegularExpressions;

namespace Desklet.Weather
{
	class Regexps
	{
		public static Regex StationId = new Regex ("^[A-Z]{4}$", RegexOptions.Compiled);
		public static Regex ReportTime = new Regex ("([0-9]{2})([0-9]{2})([0-9]{2})Z", RegexOptions.Compiled);
		public static Regex Wind = new Regex ("([0-9]{3}|VRB)([0-9]{2,3})G?([0-9]{2,3})?(KT|MPS|KMH)", RegexOptions.Compiled);
		public static Regex Visibility = new Regex ("^([0-9]{4})([NS]?[EW]?)$", RegexOptions.Compiled);
		public static Regex Clouds = new Regex ("^(VV|FEW|SCT|SKC|CLR||BKN|OVC)([0-9]{3}|///)(CU|CB|TCU|CI)?$", RegexOptions.Compiled);
		public static Regex TempAndDew = new Regex ("^(M?[0-9]{2})/(M?[0-9]{2})?$", RegexOptions.Compiled);
		public static Regex PressureHg = new Regex ("A([0-9]{4})", RegexOptions.Compiled);
		public static Regex PressureMb = new Regex ("Q([0-9]{4})", RegexOptions.Compiled);
		public static Regex Weather = new Regex ("^(VC)?(-|\\+)?(MI|PR|BC|DR|BL|SH|TS|FZ)?((DZ|RA|SN|SG|IC|PL|GR|GS|UP)+)?(BR|FG|FU|VA|DU|SA|HZ|PY)?(PO|SQ|FC|SS)?$", RegexOptions.Compiled);
	}

	public enum WindSpeedUnits {
		Unknown,
		Knots,
		MetersPerSecond,
		KilometersPerHour
	};

	public enum VisibilityDirection {
		Invalid = 0x00,
		North = 0x01,
		South = 0x02,
		East  = 0x03,
		West  = 0x04
	};

	public enum VisibilityAccuracy {
		Invalid,
		LessThan,
		Exactly,
		MoreThan
	};

	public enum CloudsAccuracy {
		Invalid,
		LessThan,
		Exactly,
		Nil
	};

	public enum CloudsCoverage {
		Invalid,
		Clear,
		Few,
		Scatterred,
		Broken,
		Overcast,
		Vertical
	};

	public enum CloudsKind {
		Invalid,
		None,
		Cumulus,
		Cumulonimbus,
		ToweringCumulus,
		Cirrus
	};
	
	public class Clouds
	{
		CloudsAccuracy accuracy;
		CloudsCoverage coverage;
		CloudsKind kind;
		double height; // in feet

		public CloudsAccuracy Accuracy {
			get { return accuracy; }
		}

		public CloudsCoverage Coverage {
			get { return coverage; }
		}

		public CloudsKind Kind {
			get { return kind; }
		}
		
		public double Height {
			get { return height; }
			
		}

		public double Feet {
			get { return height; }
		}
		
		public double Meters {
			get { return height * 0.3048; }
		}

		public double Kilometers {
			get { return height * 0.0003048; }
		}
		
		public Clouds ()
		{
			this.coverage = CloudsCoverage.Clear;
			this.kind = CloudsKind.None;
			this.height = 0;
			this.accuracy = CloudsAccuracy.Exactly;
		}
		
		public Clouds (string coverage, string height, string kind)
		{
			switch (coverage) {
				case "VV":
					this.coverage = CloudsCoverage.Vertical;
					break;

				case "FEW":
					this.coverage = CloudsCoverage.Few;
					break;

				case "SKC":
				case "CLR":
					this.coverage = CloudsCoverage.Clear;
					break;

				case "SCT":
					this.coverage = CloudsCoverage.Scatterred;
					break;

				case "BKN":
					this.coverage = CloudsCoverage.Broken;
					break;

				case "OVC":
					this.coverage = CloudsCoverage.Overcast;
					break;

				default:
					this.coverage = CloudsCoverage.Invalid;
					break;
			}

			if (height == "000") {
				this.height = 100.0;
				accuracy = CloudsAccuracy.LessThan;
			} else if (height == "///") {
				this.height = -1.0;
				accuracy = CloudsAccuracy.Nil;
			} else {
				try {
					this.height = Convert.ToDouble (height) * 100.0;
					accuracy = CloudsAccuracy.Exactly;
				} catch {
					this.height = -1;
					accuracy = CloudsAccuracy.Invalid;
				}
			}

			Console.WriteLine ("kind == {0}", kind);
			if (!String.IsNullOrEmpty (kind)) {
				switch (kind) {
					case "CB":
						this.kind = CloudsKind.Cumulonimbus;
						break;

					case "CU":
						this.kind = CloudsKind.Cumulus;
						break;

					case "TCU":
						this.kind = CloudsKind.Cumulus;
						break;

					case "CI":
						this.kind = CloudsKind.Cirrus;
						break;

					default:
						this.kind = CloudsKind.Invalid;
						break;
				}
			}
		}
	}
	
	public class Visibility
	{
		double range; // meters
		VisibilityDirection direction = VisibilityDirection.Invalid;
		VisibilityAccuracy accuracy = VisibilityAccuracy.Invalid;

		public double Range {
			get { return range; }
		}

		public double Meters {
			get { return range; }
		}

		public double Kilometers {
			get { return range * 0.001; }
		}

		public double Miles {
			get { return range * 0.00062137119; }
		}
		
		public VisibilityDirection Direction {
			get { return direction; }
		}

		public VisibilityAccuracy Accuracy {
			get { return accuracy; }
		}
	
		public Visibility (string range, string direction)
		{
			if (range == "0000") {
				this.range = 50;
				accuracy = VisibilityAccuracy.LessThan;
			} else if (range == "9999") {
				this.range = 10000;
				accuracy = VisibilityAccuracy.MoreThan;
			} else {
				try {
					this.range = Convert.ToInt32 (range);
					accuracy = VisibilityAccuracy.Exactly;
				} catch {
					this.range = -1;
					accuracy = VisibilityAccuracy.Invalid;
				}
			}

			if (!String.IsNullOrEmpty (direction)) {
				switch (direction [0]) {
					case 'N':
						this.direction |= VisibilityDirection.North;
						break;

					case 'S':
						this.direction |= VisibilityDirection.South;
						break;
				}

				switch (direction [1]) {
					case 'E':
						this.direction |= VisibilityDirection.East;
						break;

					case 'W':
						this.direction |= VisibilityDirection.West;
						break;
				}
			}
		}
	}

	public class Wind
	{
		WindSpeedUnits unit;
		int gusts = -1;
		double direction = -1;
		double speed = -1;

		public WindSpeedUnits Unit {
			get { return unit; }
		}

		public int Gusts {
			get { return gusts; }
		}

		public double Direction {
			get { return direction; }
		}

		public double Speed {
			get { return speed; }
		}

		public double Knots {
			get { return ToKnots (); }
		}

		public double MetersPerSecond {
			get { return ToMetersPerSecond (); }
		}

		public double KilometersPerHour {
			get { return ToKilometersPerHour (); }
		}

		public double MilesPerHour {
			get { return ToMilesPerHour (); }
		}
		
		public Wind (string direction, string speed, string gusts, string unit)
		{
			if (direction == "VRB")
				this.direction = Double.MaxValue;
			else {
				try {
					this.direction = Convert.ToDouble (direction);
				} catch {
					this.direction = -1;
				}
			}

			try {
				this.speed = Convert.ToDouble (speed);
			} catch {
				this.speed = -1;
			}

			if (gusts != String.Empty) {
				try {
					this.gusts = Convert.ToInt32 (gusts);
				} catch {
					this.gusts = -1;
				}
			}

			switch (unit) {
				case "KT":
					this.unit = WindSpeedUnits.Knots;
					break;

				case "MPS":
					this.unit = WindSpeedUnits.MetersPerSecond;
					break;

				case "KPH":
					this.unit = WindSpeedUnits.KilometersPerHour;
					break;

				default:
					this.unit = WindSpeedUnits.Unknown;
					break;
			}
		}

		double ToMilesPerHour ()
		{
			switch (unit) {
				case WindSpeedUnits.Knots:
					return speed * 1.1507794;

				case WindSpeedUnits.MetersPerSecond:
					return speed * 2.2369363;

				case WindSpeedUnits.KilometersPerHour:
					return speed * 0.62137119;
			}

			return 0.0;
		}
		
		double ToKilometersPerHour ()
		{
			switch (unit) {
				case WindSpeedUnits.Knots:
					return speed * 1.852;

				case WindSpeedUnits.MetersPerSecond:
					return speed * 3.6;

				case WindSpeedUnits.KilometersPerHour:
					return speed;
			}

			return 0.0;
		}
		
		double ToMetersPerSecond ()
		{
			switch (unit) {
				case WindSpeedUnits.Knots:
					return speed * 0.51444444;

				case WindSpeedUnits.MetersPerSecond:
					return speed;

				case WindSpeedUnits.KilometersPerHour:
					return speed * 0.27777778;
			}

			return 0.0;
		}
		
		double ToKnots () 
		{
			switch (unit) {
				case WindSpeedUnits.Knots:
					return speed;

				case WindSpeedUnits.MetersPerSecond:
					return speed * 1.9438445;

				case WindSpeedUnits.KilometersPerHour:
					return speed * 0.5399568;
			}

			return 0.0;
		}
	}

	public class Temperature
	{
		double celcius = 0.0;
		double kelvin = 0.0;
		double fahrenheit = 0.0;

		public double Celcius {
			get { return celcius; }
		}

		public double Kelvin {
			get { return kelvin; }
		}

		public double Fahrenheit {
			get { return fahrenheit; }
		}
		
		public Temperature (string celcius)
		{
			try {
				this.celcius = Convert.ToDouble (celcius);
			} catch {
				this.celcius = 0.0;
			}

			this.fahrenheit = (this.celcius * 1.8) + 32.0;
			this.kelvin = this.celcius + 273.15;
		}
	}

	public enum WeatherIntensity {
		Invalid = 0x00,
		Light = 0x01,
		Moderate = 0x02,
		Heavy = 0x04,
		Vincinity = 0x08
	};

	public enum WeatherDescriptor {
		Invalid,
		Shallow,
		Partial,
		Patches,
		LowDrifting,
		Blowing,
		Shower,
		Thunderstorm,
		Freezing
	};

	public enum WeatherPrecipitation {
		Invalid,
		Drizzle,
		Rain,
		Snow,
		SnowGrains,
		IceCrystals,
		IcePellets,
		Hail,
		SmallHail,
		Unknown
	};

	public enum WeatherObscuration {
		Invalid,
		Mist,
		Fog,
		Smoke,
		VolcanicAsh,
		Dust,
		Sand,
		Haze,
		Spray
	};

	public enum WeatherMisc {
		Invalid,
		DustWhirls,
		Squalls,
		Tornado,
		DustStorm
	};
	
	public class Weather
	{
		WeatherIntensity intensity = WeatherIntensity.Invalid;
		WeatherDescriptor descriptor = WeatherDescriptor.Invalid;
		WeatherPrecipitation precipitation = WeatherPrecipitation.Invalid;
		WeatherObscuration obscuration = WeatherObscuration.Invalid;
		WeatherMisc misc = WeatherMisc.Invalid;

		public WeatherIntensity Intensity {
			get { return intensity; }
		}

		public WeatherDescriptor Descriptor {
			get { return descriptor; }
		}

		public WeatherPrecipitation Precipitation {
			get { return precipitation; }
		}

		public WeatherObscuration Obscuration {
			get { return obscuration; }
		}

		public WeatherMisc Misc {
			get { return misc; }
		}
		
		public Weather (string proximity, string intensity, string descriptor,
				string precipitation, string obscuration, string misc)
		{
			if (proximity == "VC")
				this.intensity = WeatherIntensity.Vincinity;
			
			switch (intensity) {
				case "-":
					this.intensity |= WeatherIntensity.Light;
					break;
					
				case "":
					this.intensity |= WeatherIntensity.Moderate;
					break;
					
				case "+":
					this.intensity |= WeatherIntensity.Heavy;
					break;
			}
			
			switch (descriptor) {
				case "MI":
					this.descriptor = WeatherDescriptor.Shallow;
					break;

				case "PR":
					this.descriptor = WeatherDescriptor.Partial;
					break;

				case "BC":
					this.descriptor = WeatherDescriptor.Patches;
					break;

				case "DR":
					this.descriptor = WeatherDescriptor.LowDrifting;
					break;

				case "BL":
					this.descriptor = WeatherDescriptor.Blowing;
					break;

				case "SH":
					this.descriptor = WeatherDescriptor.Shower;
					break;

				case "TH":
					this.descriptor = WeatherDescriptor.Thunderstorm;
					break;

				case "FZ":
					this.descriptor = WeatherDescriptor.Freezing;
					break;

				default:
					this.descriptor = WeatherDescriptor.Invalid;
					break;
			};

			switch (precipitation) {
				case "DZ":
					this.precipitation = WeatherPrecipitation.Drizzle;
					break;

				case "RA":
					this.precipitation = WeatherPrecipitation.Rain;
					break;

				case "SN":
					this.precipitation = WeatherPrecipitation.Snow;
					break;

				case "SG":
					this.precipitation = WeatherPrecipitation.SnowGrains;
					break;
					
				case "IC":
					this.precipitation = WeatherPrecipitation.IceCrystals;
					break;

				case "PE":
					this.precipitation = WeatherPrecipitation.IcePellets;
					break;

				case "GR":
					this.precipitation = WeatherPrecipitation.Hail;
					break;

				case "GS":
					this.precipitation = WeatherPrecipitation.SmallHail;
					break;

				case "UP":
					this.precipitation = WeatherPrecipitation.Unknown;
					break;

				default:
					this.precipitation = WeatherPrecipitation.Invalid;
					break;
			};

			switch (obscuration) {
				case "BR":
					this.obscuration = WeatherObscuration.Mist;
					break;

				case "FG":
					this.obscuration = WeatherObscuration.Fog;
					break;

				case "FU":
					this.obscuration = WeatherObscuration.Smoke;
					break;

				case "VA":
					this.obscuration = WeatherObscuration.VolcanicAsh;
					break;

				case "DU":
					this.obscuration = WeatherObscuration.Dust;
					break;

				case "SA":
					this.obscuration = WeatherObscuration.Sand;
					break;

				case "HZ":
					this.obscuration = WeatherObscuration.Haze;
					break;

				case "PY":
					this.obscuration = WeatherObscuration.Spray;
					break;

				default:
					this.obscuration = WeatherObscuration.Invalid;
					break;
			};

			switch (misc) {
				case "PO":
					this.misc = WeatherMisc.DustWhirls;
					break;

				case "SQ":
					this.misc = WeatherMisc.Squalls;
					break;

				case "FC":
					this.misc = WeatherMisc.Tornado;
					break;

				case "SS":
					this.misc = WeatherMisc.DustStorm;
					break;

				default:
					this.misc = WeatherMisc.Invalid;
					break;
			};
		}
	}
	
	public class Metar
	{
		string metar;
		string stationCode;
		DateTime time;
		Wind wind;
		Visibility visibility;
		List<Clouds> clouds;
		Temperature temperature;
		Temperature dewPoint;
		double pressureHg;
		double pressureMb;
		Weather weather;
		
		public string StationCode {
			get { return stationCode; }
		}

		public DateTime Time {
			get { return time; }
		}

		public Wind Wind {
			get { return wind; }
		}

		public Visibility Visibility {
			get { return visibility; }
		}

		public List<Clouds> Clouds {
			get { return clouds; }
		}

		public Temperature Temperature {
			get { return temperature; }
		}

		public Temperature DewPoint {
			get { return dewPoint; }
		}

		public double PressureHg {
			get { return pressureHg; }
		}

		public double PressureMb {
			get { return pressureMb; }
		}

		public Weather Weather {
			get { return weather; }
		}
		
		public Metar (string metar)
		{
			this.metar = metar;
			Parse ();
		}
		
		void Parse (string metar)
		{
			this.metar = metar;
			Parse ();
		}
		
		void Parse ()
		{
			if (String.IsNullOrEmpty (metar))
				throw new ApplicationException ("METAR string is empty");

			string[] parts = metar.Split (new char[] {' ', '\t'});

			foreach (string p in parts)
				DoPart (p);
		}

		void AddClouds (Clouds c)
		{
			if (clouds == null)
				clouds = new List <Clouds> (1);
			clouds.Add (c);
		}
		
		void DoPart (string part)
		{
			Console.WriteLine ("part {0}", part);
			if (part == "CAVOK") {
				AddClouds (new Clouds ());
				return;
			}
			
			Match match;
			match = Regexps.StationId.Match (part);
			if (match.Success) {
				stationCode = part;
				return;
			}

			string[] groups;
			match = Regexps.ReportTime.Match (part);
			if (match.Success) {
				groups = GetGroups (match);

				// format: ddmmhhZ
				DateTime now = DateTime.Now;
				int day, minute, hour;

				try {
					day = Convert.ToInt32 (groups [0]);
				} catch {
					day = now.Day;
				}

				try {
					hour = Convert.ToInt32 (groups [1]);
				} catch {
					hour = now.Hour;
				}

				try {
					minute = Convert.ToInt32 (groups [2]);
				} catch {
					minute = now.Minute;
				}
				
				time = new DateTime (now.Year, now.Month, day, hour, minute, 0);
				return;				
			}

			match = Regexps.Wind.Match (part);
			if (match.Success) {
				groups = GetGroups (match);

				wind = new Wind (groups [0], groups [1], groups [2], groups [3]);
				return;
			}

			match = Regexps.Visibility.Match (part);
			if (match.Success) {
				groups = GetGroups (match);

				visibility = new Visibility (groups [0], groups [1]);
				return;
			}
			
			match = Regexps.Clouds.Match (part);
			if (match.Success) {
				groups = GetGroups (match);

				AddClouds (new Clouds (groups [0], groups [1], groups [2]));
				return;
			}

			match = Regexps.TempAndDew.Match (part);
			if (match.Success) {
				groups = GetGroups (match);
				
				temperature = new Temperature (groups [0].Replace ("M", "-"));
				dewPoint = new Temperature (groups [1].Replace ("M", "-"));
				return;
			}

			match = Regexps.PressureHg.Match (part);
			if (match.Success) {
				groups = GetGroups (match);
				
				try {
					pressureHg = Convert.ToDouble (groups [0]) / 100;
				} catch {
					pressureHg = -1;
				}
				return;
			}

			match = Regexps.PressureMb.Match (part);
			if (match.Success) {
				groups = GetGroups (match);

				try {
					pressureMb = Convert.ToDouble (groups [0]);
				} catch {
					pressureMb = -1;
				}
				return;
			}

			match = Regexps.Weather.Match (part);
			if (match.Success) {
				groups = GetGroups (match);

				weather = new Weather (groups [0], groups [1], groups [2],
						       groups [3], groups [4], groups [5]);
				return;
			}
			
			// We ignore all the other parts
		}
		
		string[] GetGroups (Match match)
		{
			GroupCollection groups = match.Groups;
			string[] ret = new string [groups.Count];
			Group g;
			
			for (int i = 1; i < groups.Count; i++) {
				g = groups [i];
				if (g.Captures.Count <= 0) {
					ret [i - 1] = String.Empty;
					continue;
				}
				ret [i - 1] = g.Captures [0].Value;
			}

			return ret;
		}
	}
}
