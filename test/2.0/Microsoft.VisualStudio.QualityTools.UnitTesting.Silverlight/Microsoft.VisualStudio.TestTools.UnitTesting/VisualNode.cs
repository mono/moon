using System;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Documents;
using System.Windows.Ink;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Shapes;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace Microsoft.VisualStudio.TestTools.UnitTesting
{
	public abstract class VisualNode
	{
		public string Name {
			get; protected set;
		}

		public VisualNode[] Siblings {
			get; protected set;	
		}

		public Type Type {
			get; protected set;
		}

		protected VisualNode ()
		{

		}

		public abstract void DoCheck (object o);
	}

	public class VisualNode<T> : VisualNode
	{
		Action<T> Action {
			get; set;
		}

		public VisualNode (string name, params VisualNode [ ] siblings)
			: this (name, null, siblings)
		{
			
		}

		public VisualNode (string name, Action<T> action, params VisualNode [ ] siblings)
		{
			Action = action;
			Name = name;
			Siblings = siblings;
			Type = typeof (T);
		}

		public override void DoCheck (object o)
		{
			if (Action != null)
				Action ((T) o);
		}
	}
}
