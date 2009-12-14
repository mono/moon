using System.Windows; 
using System;

namespace Controls.Extended 
{
    public partial class App : Application
    { 
 
        public App()
        { 
            this.Startup += this.OnStartup;
            this.Exit += this.OnExit;
 
            InitializeComponent();
        }
 
        private void OnStartup(object sender, EventArgs e) 
        {
            // Load the main control 
            this.RootVisual = new Page();
        }
 
        private void OnExit(object sender, EventArgs e)
        {
 
        } 
    }
} 
