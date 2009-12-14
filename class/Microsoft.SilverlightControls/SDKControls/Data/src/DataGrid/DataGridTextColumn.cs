// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Public License (Ms-PL).
// Please see http://go.microsoft.com/fwlink/?LinkID=131993 for details.
// All other rights reserved.

using System.Windows.Input;
using System.Windows.Markup;
using System.Windows.Media;

namespace System.Windows.Controls
{
    public class DataGridTextColumn : DataGridBoundColumn
    {
        #region Constants

        private const string DATAGRIDTEXTCOLUMN_fontFamilyName = "FontFamily";
        private const string DATAGRIDTEXTCOLUMN_fontSizeName = "FontSize";
        private const string DATAGRIDTEXTCOLUMN_fontStyleName = "FontStyle";
        private const string DATAGRIDTEXTCOLUMN_fontWeightName = "FontWeight";
        private const string DATAGRIDTEXTCOLUMN_foregroundName = "Foreground";

        #endregion Constants

        #region Data

        private FontFamily _fontFamily;
        private double? _fontSize;
        private FontStyle? _fontStyle;
        private FontWeight? _fontWeight;
        private Brush _foreground;
        private static ControlTemplate _textBoxTemplate = InitializeTextBoxTemplate();

        #endregion Data

        public DataGridTextColumn()
        {
        }

        #region Public Properties

        /// <summary>
        /// Gets or sets the font name.
        /// </summary>
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
                    NotifyPropertyChanged(DATAGRIDTEXTCOLUMN_fontFamilyName);
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
                return this._fontSize ?? Double.NaN;
            }
            set
            {
                if (this._fontSize != value)
                {
                    this._fontSize = value;
                    NotifyPropertyChanged(DATAGRIDTEXTCOLUMN_fontSizeName);
                }
            }
        }

        /// <summary>
        /// Gets or sets the font style.
        /// </summary>
        public FontStyle FontStyle
        {
            get
            {
                return this._fontStyle ?? FontStyles.Normal;
            }
            set
            {
                if (this._fontStyle != value)
                {
                    this._fontStyle = value;
                    NotifyPropertyChanged(DATAGRIDTEXTCOLUMN_fontStyleName);
                }
            }
        }

        /// <summary>
        /// Gets or sets the font weight or thickness.
        /// </summary>
        public FontWeight FontWeight
        {
            get
            {
                return this._fontWeight ?? FontWeights.Normal;
            }
            set
            {
                if (this._fontWeight != value)
                {
                    this._fontWeight = value;
                    NotifyPropertyChanged(DATAGRIDTEXTCOLUMN_fontWeightName);
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
                    NotifyPropertyChanged(DATAGRIDTEXTCOLUMN_foregroundName);
                }
            }
        }

        #endregion Public Properties

        #region Internal Properties

        #endregion Internal Properties

        #region Protected Methods

        protected override void CancelCellEdit(FrameworkElement editingElement, object uneditedValue)
        {
            TextBox textBox = editingElement as TextBox;
            if (textBox != null)
            {
                textBox.Text = uneditedValue as string;
            }
        }

        protected override FrameworkElement GenerateEditingElement(DataGridCell cell, object dataItem)
        {
            TextBox textBox = new TextBox();
            textBox.Template = _textBoxTemplate;
            textBox.VerticalAlignment = VerticalAlignment.Center;
            textBox.Background = new System.Windows.Media.SolidColorBrush(System.Windows.Media.Colors.Transparent);

            if (this._fontFamily != null)
            {
                textBox.FontFamily = this._fontFamily;
            }
            if (this._fontSize.HasValue)
            {
                textBox.FontSize = this._fontSize.Value;
            }
            if (this._fontStyle.HasValue)
            {
                textBox.FontStyle = this._fontStyle.Value;
            }
            if (this._fontWeight.HasValue)
            {
                textBox.FontWeight = this._fontWeight.Value;
            }
            if (this._foreground != null)
            {
                textBox.Foreground = this._foreground;
            }
            textBox.SetBinding(TextBox.TextProperty, this.Binding);
            return textBox;
        }

        protected override FrameworkElement GenerateElement(DataGridCell cell, object dataItem)
        {
            TextBlock textBlockElement = new TextBlock();
            textBlockElement.Margin = new Thickness(4);
            textBlockElement.VerticalAlignment = VerticalAlignment.Center;
            if (this._fontFamily != null)
            {
                textBlockElement.FontFamily = this._fontFamily;
            }
            if (this._fontSize.HasValue)
            {
                textBlockElement.FontSize = this._fontSize.Value;
            }
            if (this._fontStyle.HasValue)
            {
                textBlockElement.FontStyle = this._fontStyle.Value;
            }
            if (this._fontWeight.HasValue)
            {
                textBlockElement.FontWeight = this._fontWeight.Value;
            }
            if (this._foreground != null)
            {
                textBlockElement.Foreground = this._foreground;
            }
            textBlockElement.SetBinding(TextBlock.TextProperty, this.Binding);
            return textBlockElement;
        }

        protected override object PrepareCellForEdit(FrameworkElement editingElement, RoutedEventArgs editingEventArgs)
        {
            TextBox textBox = editingElement as TextBox;
            if (textBox != null)
            {
                string uneditedText = textBox.Text;
                int len = uneditedText.Length;
                KeyEventArgs keyEventArgs = editingEventArgs as KeyEventArgs;
                if (keyEventArgs != null && keyEventArgs.Key == Key.F2)
                {
                    // Put caret at the end of the text
                    textBox.Select(len, len);
                }
                else
                {
                    // Select all text
                    textBox.Select(0, len);
                }
                return uneditedText;
            }
            return string.Empty;
        }

        /// <summary>
        /// Called by the DataGrid control when this column asks for its elements to be
        /// updated, because a property changed.
        /// </summary>
        protected internal override void RefreshCellContent(FrameworkElement element, string propertyName)
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
                if (propertyName == DATAGRIDTEXTCOLUMN_fontFamilyName)
                {
                    textBlock.FontFamily = this.FontFamily;
                }
                else if (propertyName == DATAGRIDTEXTCOLUMN_fontSizeName)
                {
                    textBlock.FontSize = this.FontSize;
                }
                else if (propertyName == DATAGRIDTEXTCOLUMN_fontStyleName)
                {
                    textBlock.FontStyle = this.FontStyle;
                }
                else if (propertyName == DATAGRIDTEXTCOLUMN_fontWeightName)
                {
                    textBlock.FontWeight = this.FontWeight;
                }
                else if (propertyName == DATAGRIDTEXTCOLUMN_foregroundName)
                {
                    textBlock.Foreground = this.Foreground;
                }
                else
                {
                    if (this.FontFamily != null)
                    {
                        textBlock.FontFamily = this.FontFamily;
                    }
                    if (!Double.IsNaN(this.FontSize))
                    {
                        textBlock.FontSize = this.FontSize;
                    }
                    textBlock.FontStyle = this.FontStyle;
                    textBlock.FontWeight = this.FontWeight;
                    if (this.Foreground != null)
                    {
                        textBlock.Foreground = this.Foreground;
                    }
                }
                return;
            }
            if (propertyName == DATAGRIDTEXTCOLUMN_fontFamilyName)
            {
                textBox.FontFamily = this.FontFamily;
            }
            else if (propertyName == DATAGRIDTEXTCOLUMN_fontSizeName)
            {
                textBox.FontSize = this.FontSize;
            }
            else if (propertyName == DATAGRIDTEXTCOLUMN_fontStyleName)
            {
                textBox.FontStyle = this.FontStyle;
            }
            else if (propertyName == DATAGRIDTEXTCOLUMN_fontWeightName)
            {
                textBox.FontWeight = this.FontWeight;
            }
            else if (propertyName == DATAGRIDTEXTCOLUMN_foregroundName)
            {
                textBox.Foreground = this.Foreground;
            }
            else
            {
                if (this.FontFamily != null)
                {
                    textBox.FontFamily = this.FontFamily;
                }
                if (!Double.IsNaN(this.FontSize))
                {
                    textBox.FontSize = this.FontSize;
                }
                textBox.FontStyle = this.FontStyle;
                textBox.FontWeight = this.FontWeight;
                if (this.Foreground != null)
                {
                    textBox.Foreground = this.Foreground;
                }
            }
        }

        #endregion Protected Methods

        #region Internal Methods

        #endregion Internal Methods

        #region Private Methods

        private static ControlTemplate InitializeTextBoxTemplate()
        {
            // Loads our styles for the TextBox
            string styleXaml = null;
            System.IO.Stream stream = typeof(DataGridTextColumn).Assembly.GetManifestResourceStream("System.Windows.Controls.DataGrid.DataGridTextColumn.xaml");
            if (stream != null)
            {
                styleXaml = new System.IO.StreamReader(stream).ReadToEnd();
                stream.Close();
            }
            return XamlReader.Load(styleXaml) as ControlTemplate;
        }

        #endregion Private Methods
    }
}
