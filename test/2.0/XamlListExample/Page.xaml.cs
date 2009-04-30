using System;
using System.Collections.ObjectModel;
using System.Windows;
using System.Windows.Controls;

namespace XamlListExample
{
	public class ObjectCollection : Collection<object>
	{
		
	}

	public partial class Page : UserControl
	{
		public Page ()
		{
			InitializeComponent ();
			ObjectCollection collection = (ObjectCollection)Application.Current.Resources["Collection"];
			Console.WriteLine ("Collection has 3 elements? {0}", collection.Count == 3);
		}
	}
}

