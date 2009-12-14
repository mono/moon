// Copyright © Microsoft Corporation. 
// This source is subject to the Microsoft Source License for Silverlight Controls (March 2008 Release).
// Please see http://go.microsoft.com/fwlink/?LinkID=111693 for details.
// All other rights reserved. 

using System.Collections.Generic;
using System.Diagnostics; 
using System.Windows.Input; 
using System.Windows.Threading;
 
namespace System.Windows.Controls
{
    /// <summary> 
    ///     Service class that provides the system implementation for displaying ToolTips.
    /// </summary>
    public static class ToolTipService 
    { 
        #region Constants
 
        private const int TOOLTIPSERVICE_betweenShowDelay = 100; // 100 milliseconds
        private const int TOOLTIPSERVICE_initialShowDelay = 400; // 400 milliseconds
        private const int TOOLTIPSERVICE_showDuration = 5000;    // 5000 milliseconds 

        #endregion Constants
 
        #region Data 

        private static ToolTip _currentToolTip; 
        private static DispatcherTimer _closeTimer;
        private static object _lastEnterSource;
        private static DateTime _lastToolTipOpenedTime = DateTime.MinValue; 
        private static DispatcherTimer _openTimer;
        private static UIElement _owner;
        private static FrameworkElement _rootVisual; 
        private static Dictionary<UIElement, ToolTip> _toolTipDictionary = new Dictionary<UIElement, ToolTip>(); 

        #endregion Data 

        #region ToolTip Property
        /// <summary>
        ///     The DependencyProperty for the ToolTip property. 
        /// </summary>
        public static readonly DependencyProperty ToolTipProperty =
                        DependencyProperty.RegisterAttachedCore( 
                        "ToolTip",              // Name 
                        typeof(object),         // Type
                        typeof(ToolTipService), // Owner 
                        new PropertyMetadata(OnToolTipPropertyChanged));

        /// <summary> 
        ///     Gets the value of the ToolTip property on the specified object.
        /// </summary>
        /// <param name="element">The object on which to query the ToolTip property.</param> 
        /// <returns>The value of the ToolTip property.</returns> 
        public static object GetToolTip(DependencyObject element)
        { 
            if (element == null)
            {
                throw new ArgumentNullException("element"); 
            }
            return element.GetValue(ToolTipService.ToolTipProperty);
        } 
 
        /// <summary>
        ///     Sets the ToolTip property on the specified object. 
        /// </summary>
        /// <param name="element">The object on which to set the ToolTip property.</param>
        /// <param name="value"> 
        ///     The value of the ToolTip property. If the value is of type ToolTip, then
        ///     that is the ToolTip that will be used (without any modification). If the value
        ///     is of any other type, then that value will be used as the content for a ToolTip 
        ///     provided by this service, and the other attached properties of this service 
        ///     will be used to configure the ToolTip.
        /// </param> 
        public static void SetToolTip(DependencyObject element, object value)
        {
            if (element == null) 
            {
                throw new ArgumentNullException("element");
            } 
 
            element.SetValue(ToolTipService.ToolTipProperty, value);
        } 

        private static void OnToolTipPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        { 
            UIElement owner = (UIElement)d;

            object toolTip = e.NewValue; 
            if (e.OldValue != null) 
            {
                UnregisterToolTip(owner); 
            }

            if (toolTip == null) 
            {
                return;
            } 
 
            RegisterToolTip(owner, toolTip);
 
            SetRootVisual();
        }
 
        #endregion ToolTip Property

        #region Internal Properties 
 
        /// <summary>
        /// Place the ToolTip relative to this point 
        /// </summary>
        internal static Point MousePosition { get; set; }
 
        /// <summary>
        /// VisualRoot - the main page
        /// </summary> 
        internal static FrameworkElement RootVisual 
        {
            get 
            {
                SetRootVisual();
                return ToolTipService._rootVisual; 
            }
        }
 
        /// <summary> 
        /// Access the toolTip which is currenly open by mouse movements
        /// </summary> 
        internal static ToolTip CurrentToolTip
        {
            get { return ToolTipService._currentToolTip; } 
        }

        #endregion Internal Properties 
 
        #region Internal Methods
 
        internal static void OnOwnerMouseEnterInternal(object sender, object source)
        {
            if ((ToolTipService._lastEnterSource != null) && object.ReferenceEquals(ToolTipService._lastEnterSource, source)) 
            {
                // ToolTipService had processed this event once before, when it fired on the child
                // skip it now 
                return; 
            }
 
            UIElement senderElement = (UIElement)sender;
            if (ToolTipService._currentToolTip != null)
            { 
                if (ToolTipService._toolTipDictionary[senderElement] != ToolTipService._currentToolTip)
                {
                    // first close the previous ToolTip if entering nested elements with tooltips 
                    CloseAutomaticToolTip(null, EventArgs.Empty); 
                }
                else 
                {
                    // reentering the same element
                    return; 
                }
            }
 
            ToolTipService._owner = senderElement; 
            ToolTipService._lastEnterSource = source;
 
            Debug.Assert(ToolTipService._currentToolTip == null);

            SetRootVisual(); 

            TimeSpan sinceLastOpen = DateTime.Now - ToolTipService._lastToolTipOpenedTime; 
            if (TimeSpan.Compare(sinceLastOpen, new TimeSpan(0, 0, 0, 0, TOOLTIPSERVICE_betweenShowDelay)) <= 0) 
            {
                // open the ToolTip immediately 
                OpenAutomaticToolTip(null, EventArgs.Empty);
            }
            else 
            {
                // open the ToolTip after the InitialShowDelay interval expires
                if (ToolTipService._openTimer == null) 
                { 
                    ToolTipService._openTimer = new DispatcherTimer();
                    ToolTipService._openTimer.Tick += new EventHandler(OpenAutomaticToolTip); 
                }
                ToolTipService._openTimer.Interval = new TimeSpan(0, 0, 0, 0, TOOLTIPSERVICE_initialShowDelay); 
                ToolTipService._openTimer.Start();
            }
        } 
 
        internal static void OnOwnerMouseLeave(object sender, MouseEventArgs e)
        { 
            if (ToolTipService._currentToolTip == null)
            {
                // ToolTip had not been opened yet 
                ToolTipService._openTimer.Stop();
                ToolTipService._owner = null;
                ToolTipService._lastEnterSource = null; 
                return; 
            }
            CloseAutomaticToolTip(null, EventArgs.Empty); 
        }

        #endregion Internal Methods 

        #region Private Methods
 
        // This method should be executed on the UI thread 
        private static void CloseAutomaticToolTip(object sender, EventArgs e)
        { 
            ToolTipService._closeTimer.Stop();
            Debug.Assert(ToolTipService._currentToolTip != null, "no ToolTip to close");
 
            ToolTipService._currentToolTip.IsOpen = false;
            ToolTipService._currentToolTip = null;
            ToolTipService._owner = null; 
            ToolTipService._lastEnterSource = null; 

            // set last opened timestamp only if the ToolTip is opened by a mouse movement 
            ToolTipService._lastToolTipOpenedTime = DateTime.Now;

        } 

        private static ToolTip ConvertToToolTip(object o)
        { 
            ToolTip toolTip = o as ToolTip; 
            if (toolTip == null)
            { 
                toolTip = new ToolTip();
                toolTip.Content = o;
            } 
            return toolTip;
        }
 
        private static bool IsSpecialKey(Key key) 
        {
            Key[] specialKeys = 
            {
                Key.Alt,
                Key.Back, 
                Key.Delete,
                Key.Down,
                Key.End, 
                Key.Home, 
                Key.Insert,
                Key.Left, 
                Key.PageDown,
                Key.PageUp,
                Key.Right, 
                Key.Space,
                Key.Up
            }; 
 
            for (int i = 0; i < specialKeys.Length; i++)
            { 
                if (key == specialKeys[i])
                {
                    return true; 
                }
            }
            return false; 
        } 

        private static void OnOwnerKeyDown(object sender, KeyEventArgs e) 
        {
            if ((ToolTipService._lastEnterSource != null) && object.ReferenceEquals(ToolTipService._lastEnterSource, e.OriginalSource))
            { 
                return;
            }
 
            if (ToolTipService._owner != sender) 
            {
                return; 
            }

            // close the opened ToolTip or cancel mouse hover 
            if (ToolTipService._currentToolTip == null)
            {
                ToolTipService._openTimer.Stop(); 
                ToolTipService._owner = null; 
                ToolTipService._lastEnterSource = null;
                return; 
            }

            if (IsSpecialKey(e.Key)) 
            {
                return;
            } 
 
            CloseAutomaticToolTip(null, EventArgs.Empty);
        } 

        private static void OnOwnerMouseEnter(object sender, MouseEventArgs e)
        { 
            // cache mouse position relative to the plug-in
            ToolTipService.MousePosition = e.GetPosition(null);
 
            OnOwnerMouseEnterInternal(sender, e.OriginalSource); 
        }
 
        private static void OnOwnerMouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            if ((ToolTipService._lastEnterSource != null) && object.ReferenceEquals(ToolTipService._lastEnterSource, e.OriginalSource)) 
            {
                return;
            } 
 
            if (ToolTipService._owner != sender)
            { 
                return;
            }
 
            // close the opened ToolTip or cancel mouse hover
            if (ToolTipService._currentToolTip == null)
            { 
                ToolTipService._openTimer.Stop(); 
                ToolTipService._owner = null;
                ToolTipService._lastEnterSource = null; 
                return;
            }
 
            CloseAutomaticToolTip(null, EventArgs.Empty);
        }
 
        private static void OnRootMouseMove(object sender, MouseEventArgs e) 
        {
            ToolTipService.MousePosition = e.GetPosition(null); 
        }

        private static void OpenAutomaticToolTip(object sender, EventArgs e) 
        {
            ToolTipService._openTimer.Stop();
 
            Debug.Assert(ToolTipService._owner != null, "ToolTip owner was not set prior to starting the open timer"); 
            Debug.Assert(ToolTipService._toolTipDictionary[ToolTipService._owner] != null, "ToolTip must have been registered");
 
            ToolTipService._currentToolTip = ToolTipService._toolTipDictionary[ToolTipService._owner];

            ToolTipService._currentToolTip.IsOpen = true; 

            // start the timer which closes the ToolTip
            if (ToolTipService._closeTimer == null) 
            { 
                ToolTipService._closeTimer = new DispatcherTimer();
                ToolTipService._closeTimer.Tick += new EventHandler(CloseAutomaticToolTip); 
            }
            ToolTipService._closeTimer.Interval = new TimeSpan(0, 0, 0, 0, TOOLTIPSERVICE_showDuration);
            ToolTipService._closeTimer.Start(); 
        }

        private static void PositiveValueValidation(DependencyObject d, DependencyPropertyChangedEventArgs e) 
        { 
            if ((int)e.NewValue <= 0)
            { 
                throw new ArgumentException(Resource.ToolTipService_SetTimeoutProperty_InvalidValue, "e");
            }
        } 

        private static void RegisterToolTip(UIElement owner, object toolTip)
        { 
            Debug.Assert(!ToolTipService._toolTipDictionary.ContainsKey(owner), "duplicate tooltip for the same owner element"); 
            Debug.Assert(owner != null, "ToolTip must have an owner");
            Debug.Assert(toolTip != null, "ToolTip can not be null"); 

            owner.MouseEnter += new MouseEventHandler(OnOwnerMouseEnter);
            owner.MouseLeave += new MouseEventHandler(OnOwnerMouseLeave); 
            owner.MouseLeftButtonDown += new MouseButtonEventHandler(OnOwnerMouseLeftButtonDown);
            owner.KeyDown += new KeyEventHandler(OnOwnerKeyDown);
            ToolTipService._toolTipDictionary[owner] = ConvertToToolTip(toolTip); 
        } 

        private static void SetRootVisual() 
        {
            if ((ToolTipService._rootVisual == null) && (Application.Current != null))
            { 
                ToolTipService._rootVisual = Application.Current.RootVisual as FrameworkElement;
                if (ToolTipService._rootVisual != null)
                { 
                    // keep caching mouse position because we can't query it from Silverlight 
                    ToolTipService._rootVisual.MouseMove += new MouseEventHandler(OnRootMouseMove);
                    ToolTipService._rootVisual.SizeChanged += new SizeChangedEventHandler(OnRootVisualSizeChanged); 
                }
            }
        } 

        private static void OnRootVisualSizeChanged(object sender, SizeChangedEventArgs e)
        { 
            if (ToolTipService._currentToolTip != null) 
            {
                ToolTipService._currentToolTip.OnRootVisualSizeChanged(); 
            }
        }
 
        private static void UnregisterToolTip(UIElement owner)
        {
            Debug.Assert(owner != null, "owner element is required"); 
 
            if (!ToolTipService._toolTipDictionary.ContainsKey(owner))
            { 
                return;
            }
 
            owner.MouseEnter -= new MouseEventHandler(OnOwnerMouseEnter);
            owner.MouseLeave -= new MouseEventHandler(OnOwnerMouseLeave);
            owner.MouseLeftButtonDown -= new MouseButtonEventHandler(OnOwnerMouseLeftButtonDown); 
            owner.KeyDown -= new KeyEventHandler(OnOwnerKeyDown); 

            ToolTip toolTip = ToolTipService._toolTipDictionary[owner]; 
            if (toolTip.IsOpen)
            {
                if (toolTip == ToolTipService._currentToolTip) 
                {
                    // unregistering a currently open automatic toltip
                    // thus need to stop the timer 
                    ToolTipService._closeTimer.Stop(); 
                    ToolTipService._currentToolTip = null;
                    ToolTipService._owner = null; 
                    ToolTipService._lastEnterSource = null;
                }
 
                toolTip.IsOpen = false;
            }
 
            ToolTipService._toolTipDictionary[owner] = null; 
            ToolTipService._toolTipDictionary.Remove(owner);
        } 

        #endregion  Private Methods
    } 
}
