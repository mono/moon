﻿//------------------------------------------------------------------------------
// <auto-generated>
//     This code was generated by a tool.
//     Runtime Version:2.0.50727.1434
//
//     Changes to this file may cause incorrect behavior and will be lost if
//     the code is regenerated.
// </auto-generated>
//------------------------------------------------------------------------------

namespace System.Windows.Controls.Extended.Test {
    using System;
    
    
    /// <summary>
    ///   A strongly-typed resource class, for looking up localized strings, etc.
    /// </summary>
    // This class was auto-generated by the StronglyTypedResourceBuilder
    // class via a tool like ResGen or Visual Studio.
    // To add or remove a member, edit your .ResX file then rerun ResGen
    // with the /str option, or rebuild your VS project.
    [global::System.CodeDom.Compiler.GeneratedCodeAttribute("System.Resources.Tools.StronglyTypedResourceBuilder", "2.0.0.0")]
    [global::System.Diagnostics.DebuggerNonUserCodeAttribute()]
    [global::System.Runtime.CompilerServices.CompilerGeneratedAttribute()]
    internal class Resource {
        
        private static global::System.Resources.ResourceManager resourceMan;
        
        private static global::System.Globalization.CultureInfo resourceCulture;
        
        [global::System.Diagnostics.CodeAnalysis.SuppressMessageAttribute("Microsoft.Performance", "CA1811:AvoidUncalledPrivateCode")]
        internal Resource() {
        }
        
        /// <summary>
        ///   Returns the cached ResourceManager instance used by this class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Resources.ResourceManager ResourceManager {
            get {
                if (object.ReferenceEquals(resourceMan, null)) {
                    global::System.Resources.ResourceManager temp = new global::System.Resources.ResourceManager("System.Windows.Controls.Extended.Test.Resource", typeof(Resource).Assembly);
                    resourceMan = temp;
                }
                return resourceMan;
            }
        }
        
        /// <summary>
        ///   Overrides the current thread's CurrentUICulture property for all
        ///   resource lookups using this strongly typed resource class.
        /// </summary>
        [global::System.ComponentModel.EditorBrowsableAttribute(global::System.ComponentModel.EditorBrowsableState.Advanced)]
        internal static global::System.Globalization.CultureInfo Culture {
            get {
                return resourceCulture;
            }
            set {
                resourceCulture = value;
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to &lt;{0}&gt;.
        /// </summary>
        internal static string DatePicker_WatermarkText {
            get {
                return ResourceManager.GetString("DatePicker_WatermarkText", resourceCulture);
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to &lt;toolkit:GridSplitter  xmlns=&apos;http://schemas.microsoft.com/client/2007&apos;
        ///  xmlns:toolkit=&quot;clr-namespace:System.Windows.Controls;assembly=System.Windows.Controls&quot; {0}&gt;{1}&lt;/toolkit:GridSplitter&gt;.
        /// </summary>
        internal static string GridSplitter_CustomXaml {
            get {
                return ResourceManager.GetString("GridSplitter_CustomXaml", resourceCulture);
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to &lt;toolkit:GridSplitter  xmlns=&apos;http://schemas.microsoft.com/client/2007&apos;
        ///  xmlns:toolkit=&quot;clr-namespace:System.Windows.Controls;assembly=System.Windows.Controls&quot; /&gt;.
        /// </summary>
        internal static string GridSplitter_DefaultXaml {
            get {
                return ResourceManager.GetString("GridSplitter_DefaultXaml", resourceCulture);
            }
        }
        
        /// <summary>
        ///   Looks up a localized string similar to &lt;Style xmlns=&apos;http://schemas.microsoft.com/client/2007&apos; TargetType=&quot;Control&quot;&gt;
        ///        &lt;Setter Property=&quot;Tag&quot; Value=&quot;TestStyle&quot;/&gt;
        ///        &lt;Setter Property=&quot;Template&quot;&gt;
        ///            &lt;Setter.Value&gt;
        ///                &lt;ControlTemplate TargetType=&quot;Control&quot;&gt;
        ///                    &lt;Rectangle Fill=&quot;Pink&quot;/&gt;
        ///                &lt;/ControlTemplate&gt;
        ///            &lt;/Setter.Value&gt;
        ///        &lt;/Setter&gt;
        ///    &lt;/Style&gt;.
        /// </summary>
        internal static string GridSplitter_PreviewStyle {
            get {
                return ResourceManager.GetString("GridSplitter_PreviewStyle", resourceCulture);
            }
        }
    }
}