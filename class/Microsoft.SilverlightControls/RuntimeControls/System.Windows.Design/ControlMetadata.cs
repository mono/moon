// (c) Copyright Microsoft Corporation. 
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved. 

using System;
using System.Collections.Generic; 
using System.Linq; 
using System.Text;
using Microsoft.Windows.Design.Metadata; 
using Microsoft.Windows.Design;
using System.Windows.Controls;
 
namespace System.Windows.Controls.Toolbox.Design {
    public class MetadataRegistration : IRegisterMetadata {
        private static AttributeTable _customAttributes; 
        static bool _initialized; 

        #region IRegisterMetadata Members 
        public void Register() {
            if (_initialized)
                return; 

            // Wire up the metadata store with
            // our attributes 
            MetadataStore.AddAttributeTable(CustomAttributes); 
            _initialized = true;
        } 

        /// <summary>
        /// Returns the default attribute table for a developer-focused designer. 
        /// </summary>
        public static AttributeTable CustomAttributes {
            get { 
                if (_customAttributes == null) { 
                    DeveloperMetadataBuilder builder = new DeveloperMetadataBuilder();
 
                    _customAttributes = builder.CreateTable();
                }
                return _customAttributes; 
            }
        }
        #endregion 
 
        private class DeveloperMetadataBuilder : AttributeTableBuilder {
            internal DeveloperMetadataBuilder() { 

                AddTypeAttributes(typeof(ComboBoxItem), new ToolboxBrowsableAttribute(false));
                AddTypeAttributes(typeof(ContentControl), new ToolboxBrowsableAttribute(false)); 
                AddTypeAttributes(typeof(ContentPresenter), new ToolboxBrowsableAttribute(false));
                AddTypeAttributes(typeof(InkPresenter), new ToolboxBrowsableAttribute(false));
                AddTypeAttributes(typeof(ItemsControl), new ToolboxBrowsableAttribute(false)); 
                AddTypeAttributes(typeof(ItemsPresenter), new ToolboxBrowsableAttribute(false)); 
                AddTypeAttributes(typeof(ListBoxItem), new ToolboxBrowsableAttribute(false));
                AddTypeAttributes(typeof(Primitives.RepeatButton), new ToolboxBrowsableAttribute(false)); 
                AddTypeAttributes(typeof(Primitives.Thumb), new ToolboxBrowsableAttribute(false));
                AddTypeAttributes(typeof(Primitives.ScrollBar), new ToolboxBrowsableAttribute(false));
                AddTypeAttributes(typeof(ScrollContentPresenter), new ToolboxBrowsableAttribute(false)); 

                AddTypeAttributes(typeof(Button), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(Canvas), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(CheckBox), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(ComboBox), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(Grid), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(HyperlinkButton), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(Image), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(ListBox), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(MediaElement), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(MultiScaleImage), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(OpenFileDialog), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(PasswordBox), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(Primitives.ButtonBase), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(Primitives.RangeBase), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(Primitives.ToggleButton), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(ProgressBar), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(RadioButton), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(ScrollViewer), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(Slider), new ToolboxBrowsableAttribute(true));
                AddTypeAttributes(typeof(StackPanel), new ToolboxBrowsableAttribute(true)); 
                AddTypeAttributes(typeof(TextBox), new ToolboxBrowsableAttribute(true)); 
            }
 
            private void AddTypeAttributes(Type type, params Attribute[] attribs) {
                AddCallback(type,
                    delegate(AttributeCallbackBuilder builder) { 
                        builder.AddCustomAttributes(attribs);
                    });
            } 
        } 
    }
} 
