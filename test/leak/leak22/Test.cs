using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Media.Animation;
using System.Windows.Shapes;

namespace Leak
{
	public partial class Page
	{
		List<WeakReference> runs = new List<WeakReference> ();
		void RunTest ()
		{
			Queue (Create);
		}
		
		void Create ()
		{
			var run = new Run { Text = "Hello From The Run" };
            var paragraph = new Paragraph();
            paragraph.Inlines.Add(run);

            RichTextBox box = new RichTextBox { };
            box.Blocks.Add(paragraph);

            var insertion = box.ContentEnd.GetPositionAtOffset(-2, LogicalDirection.Backward).GetPositionAtOffset(-1, LogicalDirection.Backward);
            box.Selection.Select(insertion, insertion);
            box.Selection.Insert(new Run { Text = "ARGH" });

			if (((Paragraph)box.Blocks[0]).Inlines.Count != 3)
				Fail ("We should have three inlines");
				
            foreach (var v in ((Paragraph)box.Blocks[0]).Inlines) {
            	if (v == null)
            		Fail ("No inline should be null");
            	runs.Add (new WeakReference (v));
            }
            
            GCAndInvoke (() => {
            	foreach (var r in runs)
            		if (r.Target != null)
            			Fail ("The run should be GC'ed");
            	Succeed ();
            });
		}
	}
}
