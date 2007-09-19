/*
 * Locations.cs: Moonlight Desklets Weather Desklet locations dictionary.
 *
 * Author:
 *   Marek Habersack (mhabersack@novell.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 *
 */

using System;
using System.Collections.Generic;

namespace Desklet.Weather 
{
	public abstract class Element
	{
		string name;

		public string Name {
			get { return name; }
		}

		public Element (string name)
		{
			this.name = name;
		}

		public abstract void Add (Element element);
	}

	public class Location : Element
	{
		string code;
		string coordinates;

		public string Code {
			get { return code; }
		}

		public string Coordinates {
			get { return coordinates; }
		}
		
		public Location (string name, string code, string coordinates)
			: base (name)
		{
			this.code = code;
			this.coordinates = coordinates;
		}

		public override void Add (Element element)
		{}
	}

	public class City : Element
	{
		List <Location> locations;

		public List <Location> Locations {
			get { return locations; }
		}

		public City (string name, int nLocations) : base (name)
		{
			locations = new List <Location> (nLocations);
		}

		public override void Add (Element element)
		{
			locations.Add ((Location)element);
		}
	}

	public class State : Element
	{
		List <Element> locations;

		public List <Element> Locations {
			get { return locations; }
		}

		public State (string name, int nLocations) : base (name)
		{
			locations = new List <Element> (nLocations);
		}

		public override void Add (Element element)
		{
			locations.Add (element);
		}
	}
	
	public class Country : Element
	{
		List <Element> locations;

		public List <Element> Locations {
			get { return locations; }
		}

		public Country (string name, int nLocations) : base (name)
		{
			locations = new List <Element> (nLocations);
		}

		public override void Add (Element element)
		{
			locations.Add (element);
		}
	}

	public class Region : Element
	{
		List <Country> countries;

		public List <Country> Countries {
			get { return countries; }
		}

		public Region (string name, int nCountries) : base (name)
		{
			countries = new List <Country> (nCountries);
		}

		public override void Add (Element element)
		{
			countries.Add ((Country)element);
		}
	}

	public sealed partial class Locations
	{
		List <Region> regions;

		public List <Region> Regions {
			get { return regions; }
		}
		
		public Locations ()
		{
			BuildData ();
		}
	}
}
