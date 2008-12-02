// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Windows.Input;
using System.Windows.Mediab1;
using System.Windows.Media;
using System.ComponentModel;
using System.Windows.Controls;
using System.Windowsb1;

namespace System.Windows.Controlsb1
{
    public class DataGridTextBoxColumn : DataGridBoundColumnBase
    { 
        #region Constants

        private const string DATAGRIDTEXTBOXCOLUMN_defaultFontFamily = "Trebuchet MS"; 
        private const string DATAGRIDTEXTBOXCOLUMN_fontFamilyName = "FontFamily"; 
        private const string DATAGRIDTEXTBOXCOLUMN_fontSizeName = "FontSize";
        private const string DATAGRIDTEXTBOXCOLUMN_fontStyleName = "FontStyle"; 
        private const string DATAGRIDTEXTBOXCOLUMN_fontWeightName = "FontWeight";
        private const string DATAGRIDTEXTBOXCOLUMN_foregroundName = "Foreground";
 
        #endregion Constants

        #region Data 
 
        private TextBox _editingTextBox;
        private FontFamily _fontFamily; 
        private double _fontSize;
        private FontStyle? _fontStyle;
        private FontWeight? _fontWeight; 
        private Brush _foreground;

        #endregion Data 
 
        public DataGridTextBoxColumn()
        { 
            this._fontFamily = new FontFamily(DATAGRIDTEXTBOXCOLUMN_defaultFontFamily);
            this._fontSize = 11;
        } 

        #region Public Properties
 
        /// <summary> 
        /// Gets or sets the font name.
        /// </summary> 
        [TypeConverter(typeof(System.Windows.Mediab1.FontFamilyConverter))]
        public FontFamily FontFamily
        { 
            get
            {
                return this._fontFamily; 
            } 
            set
            { 
                if (this._fontFamily != value)
                {
                    this._fontFamily = value; 
                    UpdateElements(DATAGRIDTEXTBOXCOLUMN_fontFamilyName);
                }
            } 
        } 

        /// <summary> 
        /// Gets or sets the font size.
        /// </summary>
        public double FontSize 
        {
            get
            { 
                return this._fontSize; 
            }
            set 
            {
                if (this._fontSize != value)
                { 
                    this._fontSize = value;
                    UpdateElements(DATAGRIDTEXTBOXCOLUMN_fontSizeName);
                } 
            } 
        }
 
        /// <summary>
        /// Gets or sets the font style.
        /// </summary> 
        [TypeConverter(typeof(System.Windowsb1.FontStyleConverter))]
        public FontStyle FontStyle
        { 
            get 
            {
                return this._fontStyle.HasValue ? this._fontStyle.Value : (new TextBlock()).FontStyle; 
            }
            set
            { 
                if (this._fontStyle != value)
                {
                    this._fontStyle = value; 
                    UpdateElements(DATAGRIDTEXTBOXCOLUMN_fontStyleName); 
                }
            } 
        }

        /// <summary> 
        /// Gets or sets the font weight or thickness.
        /// </summary>
        [TypeConverter(typeof(FontWeightConverter))] 
        public FontWeight FontWeight 
        {
            get 
            {
                return this._fontWeight.HasValue ? this._fontWeight.Value : (new TextBlock()).FontWeight;
            } 
            set
            {
                if (this._fontWeight != value) 
                { 
                    this._fontWeight = value;
                    UpdateElements(DATAGRIDTEXTBOXCOLUMN_fontWeightName); 
                }
            }
        } 

        /// <summary>
        /// Gets or sets a brush that describes the foreground of the column cells. 
        /// </summary> 
        public Brush Foreground
        { 
            get
            {
                return this._foreground; 
            }
            set
            { 
                if (this._foreground != value) 
                {
                    this._foreground = value; 
                    UpdateElements(DATAGRIDTEXTBOXCOLUMN_foregroundName);
                }
            } 
        }

        #endregion Public Properties 
 
        #region Internal Properties
 
        #endregion Internal Properties

        #region Public Methods 

        public override void CancelCellEdit(object uneditedValue)
        { 
            if (this._editingTextBox != null) 
            {
                this._editingTextBox.Text = uneditedValue as string; 
            }
        }
 
        public override object PrepareCellEdit(DataGridEditingTriggerInfo editingTriggerInfo)
        {
            if (this._editingTextBox != null) 
            { 
                string uneditedText = this._editingTextBox.Text;
                int len = uneditedText.Length; 
                if (editingTriggerInfo != null &&
                    editingTriggerInfo.KeyEventArgs != null &&
                    editingTriggerInfo.KeyEventArgs.Key == Key.F2) 
                {
                    // Put caret at the end of the text
                    this._editingTextBox.Select(len, len); 
                } 
                else
                { 
                    // Select all text
                    this._editingTextBox.Select(0, len);
                } 
                return uneditedText;
            }
            return string.Empty; 
        } 

        /// <summary> 
        /// Called by the DataGrid control when this column asks for its elements to be
        /// updated, because a property changed.
        /// </summary> 
        public override void UpdateElement(FrameworkElement element, string propertyName)
        {
            if (element == null) 
            { 
                throw new ArgumentNullException("element");
            } 
            TextBox textBox = element as TextBox;
            if (textBox == null)
            { 
                TextBlock textBlock = element as TextBlock;
                if (textBlock == null)
                { 
                    throw DataGridError.DataGrid.ValueIsNotAnInstanceOfEitherOr("element", typeof(TextBox), typeof(TextBlock)); 
                }
                if (propertyName == DATAGRIDTEXTBOXCOLUMN_fontFamilyName) 
                {
                    textBlock.FontFamily = this.FontFamily;
                } 
                else if (propertyName == DATAGRIDTEXTBOXCOLUMN_fontSizeName)
                {
                    textBlock.FontSize = this.FontSize; 
                } 
                else if (propertyName == DATAGRIDTEXTBOXCOLUMN_fontStyleName)
                { 
                    textBlock.FontStyle = this.FontStyle;
                }
                else if (propertyName == DATAGRIDTEXTBOXCOLUMN_fontWeightName) 
                {
                    textBlock.FontWeight = this.FontWeight;
                } 
                else if (propertyName == DATAGRIDTEXTBOXCOLUMN_foregroundName) 
                {
                    textBlock.Foreground = this.Foreground; 
                }
                else
                { 
                    textBlock.FontFamily = this.FontFamily;
                    textBlock.FontSize = this.FontSize;
                    textBlock.FontStyle = this.FontStyle; 
                    textBlock.FontWeight = this.FontWeight; 
                    if (this.Foreground != null)
                    { 
                        textBlock.Foreground = this.Foreground;
                    }
                } 
                return;
            }
            if (propertyName == DATAGRIDTEXTBOXCOLUMN_fontFamilyName) 
            { 
                textBox.FontFamily = this.FontFamily;
            } 
            else if (propertyName == DATAGRIDTEXTBOXCOLUMN_fontSizeName)
            {
                textBox.FontSize = this.FontSize; 
            }
            else if (propertyName == DATAGRIDTEXTBOXCOLUMN_fontStyleName)
            { 
                textBox.FontStyle = this.FontStyle; 
            }
            else if (propertyName == DATAGRIDTEXTBOXCOLUMN_fontWeightName) 
            {
                textBox.FontWeight = this.FontWeight;
            } 
            else if (propertyName == DATAGRIDTEXTBOXCOLUMN_foregroundName)
            {
                textBox.Foreground = this.Foreground; 
            } 
            else
            { 
                textBox.FontFamily = this.FontFamily;
                textBox.FontSize = this.FontSize;
                textBox.FontStyle = this.FontStyle; 
                textBox.FontWeight = this.FontWeight;
                if (this.Foreground != null)
                { 
                    textBox.Foreground = this.Foreground; 
                }
            } 
        }

        #endregion Public Methods 

        #region Protected Methods
 
        protected override FrameworkElement GenerateEditingElement() 
        {
            this._editingTextBox = new TextBox(); 
            this._editingTextBox.Margin = new Thickness(3, 0, 3, 0);
            this._editingTextBox.BorderThickness = new Thickness(0);
            this._editingTextBox.VerticalAlignment = VerticalAlignment.Center; 
            this._editingTextBox.Background = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent);
            this._editingTextBox.FontFamily = this.FontFamily;
            this._editingTextBox.FontSize = this.FontSize; 
            if (this._fontStyle.HasValue) 
            {
                this._editingTextBox.FontStyle = this._fontStyle.Value; 
            }
            if (this._fontWeight.HasValue)
            { 
                this._editingTextBox.FontWeight = this._fontWeight.Value;
            }
            if (this.Foreground != null) 
            { 
                this._editingTextBox.Foreground = this.Foreground;
            } 
            this._editingTextBox.SetBinding(TextBox.TextProperty, this.DisplayMemberBinding);
            return this._editingTextBox;
        } 

        protected override FrameworkElement GenerateElement()
        { 
            TextBlock textBlockElement = new TextBlock(); 
            textBlockElement.Margin = new Thickness(4, 0, 4, 0);
            textBlockElement.VerticalAlignment = VerticalAlignment.Center; 
            textBlockElement.FontFamily = this.FontFamily;
            textBlockElement.FontSize = this.FontSize;
            if (this._fontStyle.HasValue) 
            {
                textBlockElement.FontStyle = this._fontStyle.Value;
            } 
            if (this._fontWeight.HasValue) 
            {
                textBlockElement.FontWeight = this._fontWeight.Value; 
            }
            if (this.Foreground != null)
            { 
                textBlockElement.Foreground = this.Foreground;
            }
            textBlockElement.SetBinding(TextBlock.TextProperty, this.DisplayMemberBinding); 
            return textBlockElement; 
        }
 
        #endregion Protected Methods

        #region Internal Methods 

        #endregion Internal Methods
 
        #region Private Methods 

        #endregion Private Methods 
    }
}
