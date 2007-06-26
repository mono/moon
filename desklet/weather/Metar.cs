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
using System.Text.RegularExpressions;

namespace Desklets.Weather
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
	}

	public enum WindSpeedUnits {
		Unknown,
		Knots,
		MilesPerSecond,
		KilometersPerSecond
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
		Cumulus,
		Cumulonumbus,
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
		
		public double HeightFeet {
			get { return height; }
			
		}

		public double HeightMeters {
			get { return height * 0.3048; }
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
					this.height = Convert.ToDouble (height);
					accuracy = CloudsAccuracy.Exactly;
				} catch {
					this.height = -1;
					accuracy = CloudsAccuracy.Invalid;
				}
			}

			if (!String.IsNullOrEmpty (kind)) {
				switch (kind) {
					case "CB":
						this.kind = CloudsKind.Cumulonumbus;
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
		int range;
		VisibilityDirection direction = VisibilityDirection.Invalid;
		VisibilityAccuracy accuracy = VisibilityAccuracy.Invalid;

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
		int direction = -1;
		int speed = -1;

		public WindSpeedUnits Unit {
			get { return unit; }
		}

		public int Gusts {
			get { return gusts; }
		}

		public int Direction {
			get { return direction; }
		}

		public int Speed {
			get { return speed; }
		}
		
		public Wind (string direction, string speed, string gusts, string unit)
		{
			if (direction == "VRB")
				this.direction = Int32.MaxValue;
			else {
				try {
					this.direction = Convert.ToInt32 (direction);
				} catch {
					this.direction = -1;
				}
			}

			try {
				this.speed = Convert.ToInt32 (speed);
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
					this.unit = WindSpeedUnits.MilesPerSecond;
					break;

				case "KPH":
					this.unit = WindSpeedUnits.KilometersPerSecond;
					break;

				default:
					this.unit = WindSpeedUnits.Unknown;
					break;
			}
		}
		
	}
	
	public class Metar
	{
		string metar;
		string stationCode;
		DateTime time;
		Wind wind;
		Visibility visibility;
		Clouds clouds;
		double temperature; // Celcius
		double dewPoint; // Celcius
		double pressureHg;
		double pressureMb;

		public string StationCode {
			get { return stationCode; }
		}

		public DateTime Time {
			get { return time; }
		}

		public Wind Wind {
			get { return wind; }
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

		void DoPart (string part)
		{
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
					minute = Convert.ToInt32 (groups [1]);
				} catch {
					minute = now.Minute;
				}

				try {
					hour = Convert.ToInt32 (groups [2]);
				} catch {
					hour = now.Hour;
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

				clouds = new Clouds (groups [0], groups [1], groups [2]);
				return;
			}

			match = Regexps.TempAndDew.Match (part);
			if (match.Success) {
				groups = GetGroups (match);

				try {
					temperature = Convert.ToDouble (groups [0].Replace ("M", "-"));
				} catch {
					temperature = Double.MinValue;
				}

				try {
					dewPoint = Convert.ToDouble (groups [1].Replace ("M", "-"));
				} catch {
					dewPoint = Double.MinValue;
				}
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

			// We ignore all the other parts
		}
		
		string[] GetGroups (Match match)
		{
			string[] ret = new string [match.Groups.Count];

			GroupCollection groups = match.Groups;
			Group g;
			
			for (int i = 0; i < groups.Count; i++) {
				g = groups [i];
				if (g.Captures.Count <= 0) {
					ret [i] = String.Empty;
					continue;
				}
				ret [i] = g.Captures [0].Value;
			}

			return ret;
		}
	}
}
