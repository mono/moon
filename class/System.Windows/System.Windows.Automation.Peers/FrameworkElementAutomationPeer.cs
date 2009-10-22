// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//
// Copyright (c) 2008 Novell, Inc. (http://www.novell.com)
//
// Contact:
//   Moonlight Team (moonlight-list@lists.ximian.com)
//

using System;
using System.Collections.Generic;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Media;
using System.Linq;

namespace System.Windows.Automation.Peers {

	public class FrameworkElementAutomationPeer : AutomationPeer {

		private FrameworkElement owner;

		public FrameworkElementAutomationPeer (FrameworkElement owner)
		{
			if (owner == null)
				throw new NullReferenceException ("owner");
			this.owner = owner;
			
			// Default Automation events
			owner.SizeChanged += (o, s) => {
				Point location = GetLocation (owner);
				RaisePropertyChangedEvent (AutomationElementIdentifiers.BoundingRectangleProperty, 
				                           new Rect (0, 0, s.PreviousSize.Width, s.PreviousSize.Height), 
							   new Rect (location.X, location.Y, s.NewSize.Width, s.NewSize.Height));
			};

			Control control = owner as Control;
			if (control != null)
				control.IsEnabledChanged += (o, e) => {
					RaisePropertyChangedEvent (AutomationElementIdentifiers.IsEnabledProperty, 
								   e.OldValue,
					                           e.NewValue); 
				};
		}

		public UIElement Owner {
			get { return owner; }
		}

		protected override string GetNameCore ()
		{
			return owner.GetValue (AutomationProperties.NameProperty) as string ?? string.Empty;
		}

		protected override string GetItemTypeCore ()
		{
			return owner.GetValue (AutomationProperties.ItemTypeProperty) as string ?? string.Empty;
		}

		protected override AutomationPeer GetLabeledByCore ()
		{
			UIElement labeledBy = owner.GetValue (AutomationProperties.LabeledByProperty) as UIElement;
			if (labeledBy != null)
				return FrameworkElementAutomationPeer.CreatePeerForElement (labeledBy);
			else
				return null;
		}

		protected override List<AutomationPeer> GetChildrenCore ()
		{
			return ChildrenCore;
		}

		public override object GetPattern (PatternInterface pattern)
		{
			return null;
		}
		
		public static AutomationPeer FromElement (UIElement element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			return element.AutomationPeer;
		}
		
		protected override string GetAcceleratorKeyCore ()
		{
			return owner.GetValue (AutomationProperties.AcceleratorKeyProperty) as string ?? string.Empty;
		}
		
		protected override string GetAccessKeyCore ()
		{
			return owner.GetValue (AutomationProperties.AccessKeyProperty) as string ?? string.Empty;
		}
		
		protected override AutomationControlType GetAutomationControlTypeCore ()
		{
			// Some Peers don't override GetAutomationControlTypeCore and return something different to Custom
			// for example: TextBoxAutomationPeer
			return AutomationControlTypeCore.HasValue ? AutomationControlTypeCore.Value : AutomationControlType.Custom;
		}
		
		protected override string GetAutomationIdCore ()
		{
			return owner.GetValue (AutomationProperties.AutomationIdProperty) as string ?? string.Empty;
		}
		
		protected override Rect GetBoundingRectangleCore ()
		{
			if (VisualTreeHelper.GetParent (owner) == null)
				return new Rect (0, 0, 0, 0);

			return GetBoundingRectangleFrom (owner);
		}
		
		protected override string GetClassNameCore ()
		{
			return ClassNameCore ?? string.Empty;
		}
		
		protected override Point GetClickablePointCore ()
		{
			return new Point (0, 0);
		}
		
		protected override string GetHelpTextCore ()
		{
			return owner.GetValue (AutomationProperties.HelpTextProperty) as string ?? string.Empty;
		}
		
		protected override string GetItemStatusCore ()
		{
			return owner.GetValue (AutomationProperties.ItemStatusProperty) as string ?? string.Empty;
		}
		
		protected override string GetLocalizedControlTypeCore ()
		{
			// LAMESPEC: http://msdn.microsoft.com/en-us/library/ms743581.aspx
			// "CamelCase" literal values should be "camel case", not "camelcase"
			return GetAutomationControlType ().ToString ().ToLower ();
		}
		
		protected override AutomationOrientation GetOrientationCore ()
		{
			return AutomationOrientation.None;
		}
		
		protected override bool HasKeyboardFocusCore ()
		{
			return owner == System.Windows.Input.FocusManager.GetFocusedElement ();
		}
		
		protected override bool IsContentElementCore ()
		{
			return true;
		}
		
		protected override bool IsControlElementCore ()
		{
			return true;
		}
		
		protected override bool IsEnabledCore ()
		{
			Control ownerAsControl = owner as Control;
			if (ownerAsControl != null)
				return ownerAsControl.IsEnabled;

			// Fall back to default value
			return true;
		}
		
		protected override bool IsKeyboardFocusableCore ()
		{
			return false;
		}
		
		protected override bool IsOffscreenCore ()
		{
			return false;
		}
		
		protected override bool IsPasswordCore ()
		{
			return PasswordCore;
		}
		
		protected override bool IsRequiredForFormCore ()
		{
			bool? isRequired = (bool?) owner.GetValue (AutomationProperties.IsRequiredForFormProperty);
			return isRequired.HasValue ? isRequired.Value : false;
		}
		
		protected override void SetFocusCore ()
		{
			Control ownerAsControl = owner as Control;
			if (ownerAsControl != null)
				ownerAsControl.Focus ();
		}
		
		public static AutomationPeer CreatePeerForElement (UIElement element)
		{
			if (element == null)
				throw new ArgumentNullException ("element");
			if (element.AutomationPeer == null) {
				element.AutomationPeer = element.CreateAutomationPeer ();
				// We need to cache old values to raise PropertyChanged events
				// when calling AutomationPeer.InvalidatePeer()
				if (element.AutomationPeer != null)
					element.AutomationPeer.CacheMainProperties ();
			}
			return element.AutomationPeer;
		}

		internal override AutomationPeer GetParentCore ()
		{
			return GetParentPeer (owner);
		}

		#region Private Methods

		private AutomationPeer GetParentPeer (FrameworkElement element)
		{
			// We are returning parents of children already instantiated.
			if (element == null)
				return null;

			FrameworkElement parent = VisualTreeHelper.GetParent (element) as FrameworkElement;
			if (parent == null)
				return null;

			// Some parents don't return an Automation Peer (for example: Border or Panel subclasses)
			// We need to create the Peer because some Peers return children that don't
			// necesarily return this peer when calling GetParent 
			// (for example: ListBox when Template is not null)
			AutomationPeer peer = FrameworkElementAutomationPeer.CreatePeerForElement (parent);
			if (peer == null)
				return GetParentPeer (parent);
			else
				return peer;
		}

		#endregion

		#region Internal properties

		// Internal properties used by Peers that don't override XXXXCore and return something else 
		// than FrameworkElementAutomationPeer's default value.

		internal virtual AutomationControlType? AutomationControlTypeCore {
			get { return null; }
		}

		internal virtual string ClassNameCore {
			get { return null; }
		}

		internal virtual List<AutomationPeer> ChildrenCore {
			get { return GetChildrenRecursively (owner); }
		}

		internal virtual bool PasswordCore {
			get { return false; }
		}

		#endregion

		#region Internal methods 

		internal static List<AutomationPeer> GetChildrenRecursively (UIElement uielement)
		{
			List<AutomationPeer> children = new List<AutomationPeer> ();
			int childrenCount = VisualTreeHelper.GetChildrenCount (uielement);

			for (int child = 0; child < childrenCount; child++) {
				UIElement element = VisualTreeHelper.GetChild (uielement, child) as UIElement;
				if (element == null)
					continue;

				AutomationPeer peer 
					= FrameworkElementAutomationPeer.CreatePeerForElement (element);
				if (peer != null)
					children.Add (peer);
				else {
					List<AutomationPeer> returnedChildren 
						= GetChildrenRecursively (element);
					if (returnedChildren != null)
						children.AddRange (returnedChildren);
				}
			}

			if (children.Count == 0)
				return null;

			return children;
		}

		internal Point GetLocation (FrameworkElement owner)
		{
			if (VisualTreeHelper.GetParent (owner) == null)
				return new Point (0, 0);

			Point point = new Point (0, 0);
			try { 
				// This happens when an item is not visible yet but exists, for 
				// example ListBoxItems in ComboBox when Template is not null
				point = owner.TransformToVisual (Application.Current.RootVisual).Transform (new Point ());
			} catch (ArgumentException) { }

			return point;
		}

		internal Rect GetBoundingRectangleFrom (FrameworkElement owner)
		{
			Point location = GetLocation (owner);
			
			double width = (double) owner.GetValue (FrameworkElement.WidthProperty);
			double height = (double) owner.GetValue (FrameworkElement.HeightProperty);
			
			if (double.IsNaN (width))
				width = 0;
			if (double.IsNaN (height))
				height = 0;

			// Some Controls may not be honoring the specified Height or Width and would
			// use a different value, that's why we need to test these properties too, 
			// Examples of those Controls are TextBlock and Image.
			if (height == 0)
				height = (double) owner.GetValue (FrameworkElement.ActualHeightProperty);
			if (width == 0)
				width = (double) owner.GetValue (FrameworkElement.ActualWidthProperty);

			return new Rect (location.X, location.Y, width, height);
		}

		#endregion
	}

}
