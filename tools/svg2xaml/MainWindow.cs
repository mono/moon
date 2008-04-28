using System;
using Gtk;
using System.IO;
using System.Text;
using System.Xml;
using System.Xml.Xsl;
using Gtk.Moonlight;
using System.Windows;
using System.Windows.Media;
using System.Windows.Controls;
using System.Reflection;
using Application = Gtk.Application;


public partial class MainWindow: Gtk.Window
{	
	StringBuilder svg;
	StringBuilder xaml;
	string svgFilename;
	static GtkSilver silver;
	
	public MainWindow (): base (Gtk.WindowType.Toplevel)
	{
		Build ();
		svg = new StringBuilder();
		xaml = new StringBuilder();
		silver = new GtkSilver (400, 400);
		hbox1.PackStart (silver, true, true, 0);

	}
	
	protected void OnDeleteEvent (object sender, DeleteEventArgs a)
	{
		Application.Quit ();
		a.RetVal = true;
	}

	protected virtual void OnChooseFile (object sender, System.EventArgs e)
	{
		Gtk.FileChooserDialog fc = new FileChooserDialog (
		"Choose svg file", this, FileChooserAction.Open,
		"Cancel", ResponseType.Cancel, 
		"Open", ResponseType.Accept);
		
		if (fc.Run() == (int)ResponseType.Accept)
		{
			svg.Remove (0, svg.Length);
			svgFilename = fc.Filename;
			FileStream file = File.OpenRead(svgFilename);
			using (StreamReader sr = new StreamReader (file))
				svg.Append(sr.ReadToEnd ());
			file.Close();
			RefreshSvg();
		}
		fc.Destroy();
	}

	protected virtual void OnConvert (object sender, System.EventArgs e)
	{
		XmlDocument xsltdoc = new XmlDocument();
		Stream s = Assembly.GetExecutingAssembly().GetManifestResourceStream("svg2xaml.xslt");
		xsltdoc.Load (s);
		XslTransform t = new XslTransform();
		t.Load (xsltdoc);
		t.Transform (svgFilename, svgFilename + ".xaml");
		FileStream file = File.OpenRead(svgFilename + ".xaml");
		using (StreamReader sr = new StreamReader (file))
			xaml.Append(sr.ReadToEnd ());
		file.Close();
		RefreshXaml();
	}
	
	void RefreshSvg ()
	{
		Gdk.Pixbuf pixbuf = Rsvg.Pixbuf.FromFile (svgFilename);
		image1.Pixbuf = pixbuf;
	}
	
	void RefreshXaml ()
	{
		DependencyObject d = XamlReader.Load (xaml.ToString());
		if (d == null){
			Console.Error.WriteLine ("No dependency object returned from XamlReader");
			return;
		}
		
		if (!(d is Canvas)){
			Console.Error.WriteLine ("No Canvas as root");
			return;
		}
		Canvas canvas = (Canvas) d;
		silver.Attach (canvas);
		
		
	}
}
