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
using System.Collections.Specialized;
using System.Windows;
using System.Windows.Automation;
using System.Windows.Controls;
using System.Windows.Markup;
using System.Windows.Media;
using System.Linq;

namespace System.Windows.Automation.Peers {

	public class FrameworkElementAutomationPeer : AutomationPeer {

		private FrameworkElement owner;
		private bool? isKeyboardFocusable;

		public FrameworkElementAutomationPeer (FrameworkElement owner)
		{
			if (owner == null)
				throw new NullReferenceException ("owner");
			this.owner = owner;
			isKeyboardFocusable = null;

			// Default Automation events
			owner.SizeChanged += (o, s) => {
				Point location = GetLocation (owner);
				RaisePropertyChangedEvent (AutomationElementIdentifiers.BoundingRectangleProperty, 
				                           new Rect (0, 0, s.PreviousSize.Width, s.PreviousSize.Height), 
							   new Rect (location.X, location.Y, s.NewSize.Width, s.NewSize.Height));
			};
			owner.UIAVisibilityChanged += (o, e) => {
				IAutomationCacheProperty cachedProperty
					= GetCachedProperty (AutomationElementIdentifiers.BoundingRectangleProperty);
				Rect newValue = GetBoundingRectangle ();
				RaisePropertyChangedEvent (AutomationElementIdentifiers.BoundingRectangleProperty,
				                           cachedProperty.OldValue,
							   newValue);

				RaiseIsKeyboardFocusableEvent ();

				bool isOffscreen = IsOffscreen ();
				RaisePropertyChangedEvent (AutomationElementIdentifiers.IsOffscreenProperty,
				                           !isOffscreen,
							   isOffscreen);
			};

			Control control = owner as Control;
			if (control != null) {
				control.IsEnabledChanged += (o, e) => {
					RaisePropertyChangedEvent (AutomationElementIdentifiers.IsEnabledProperty, 
								   e.OldValue,
					                           e.NewValue);

					RaiseIsKeyboardFocusableEvent ();
				};
				control.UIAIsTabStopChanged += (o, e) => {
					RaiseIsKeyboardFocusableEvent ();
				};

				// StructureChanged
				ContentControl contentControl = control as ContentControl;
				if (contentControl != null) {
					contentControl.UIAContentChanged += OnContentChanged;
					AddItemsChangedToPanel (contentControl.Content);
				}
			}

			// SWA.AutomationProperties events
			owner.AcceleratorKeyChanged += (o, e) => {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.AcceleratorKeyProperty,
							   e.OldValue,
				                           e.NewValue);
			};
			owner.AccessKeyChanged += (o, e) => {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.AccessKeyProperty,
							   e.OldValue,
				                           e.NewValue);
			};
			owner.AutomationIdChanged += (o, e) => {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.AutomationIdProperty,
							   e.OldValue,
				                           e.NewValue);
			};
			owner.HelpTextChanged += (o, e) => {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.HelpTextProperty,
							   e.OldValue,
				                           e.NewValue);
			};
			owner.IsRequiredForFormChanged += (o, e) => {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.IsRequiredForFormProperty,
							   e.OldValue,
				                           e.NewValue);
			};
			owner.ItemStatusChanged += (o, e) => {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.ItemStatusProperty,
							   e.OldValue,
				                           e.NewValue);
			};
			owner.ItemTypeChanged += (o, e) => {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.ItemTypeProperty,
							   e.OldValue,
				                           e.NewValue);
			};
			// LabeledBy and Name properties are "special" because they somehow depend on each other.
			owner.LabeledByChanged += (o, e) => {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.LabeledByProperty,
							   e.OldValue,
				                           e.NewValue);
				// Name property
				UIElement labeledByOld = e.OldValue as UIElement;
				if (labeledByOld != null) {
					FrameworkElementAutomationPeer peer
						= FrameworkElementAutomationPeer.CreatePeerForElement (labeledByOld) as FrameworkElementAutomationPeer;
					if (peer != null)
						peer.NameChanged -= LabeledBy_NameChanged;
				}
				UIElement labeledByNew = e.NewValue as UIElement;
				if (labeledByNew != null) {
					FrameworkElementAutomationPeer peer
						= FrameworkElementAutomationPeer.CreatePeerForElement (labeledByNew) as FrameworkElementAutomationPeer;
					if (peer != null)
						peer.NameChanged += LabeledBy_NameChanged;
				}
				RaiseNameChanged ();
			};
			owner.NameChanged += (o, e) => RaiseNameChanged ();
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
			isKeyboardFocusable = KeyboardFocusable;
			return isKeyboardFocusable.Value;
		}
		
		protected override bool IsOffscreenCore ()
		{
			// We need to keep track of parents to also raise IsOffscreenProperty
			// when their Visibility changes.
			CacheParents ();

			if (owner.Visibility == Visibility.Collapsed)
				return true;

			// Our parents must be Visible to be OnScreen
			if (parents != null && parents.Count > 0) {
				for (int index = parents.Count - 1; index >= 0; index--) {
					if (parents [index].Visibility == Visibility.Collapsed)
						return true;
				}
			}

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

		private void RaiseIsKeyboardFocusableEvent ()
		{
			if (!isKeyboardFocusable.HasValue || isKeyboardFocusable.Value != KeyboardFocusable) {
				bool focusable = isKeyboardFocusable.HasValue ? isKeyboardFocusable.Value : false;
				RaisePropertyChangedEvent (AutomationElementIdentifiers.IsKeyboardFocusableProperty,
				                           focusable,
				                           !focusable);
				isKeyboardFocusable = !focusable;
			}
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
			if (IsOffscreen ())
				return new Rect (0, 0, 0, 0);
			
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

		internal event EventHandler NameChanged;

		private void LabeledBy_NameChanged (object sender, EventArgs args)
		{
			RaiseNameChanged ();
		}

		// Raises UIA Event (NameProperty) and internal event (NameChanged)
		// NOTE: This method MUST BE called by AutomationPeers overriding GetNameCore().
		internal override void RaiseNameChanged ()
		{
			IAutomationCacheProperty cachedProperty
				= GetCachedProperty (AutomationElementIdentifiers.NameProperty);
			string newValue = GetName ();
			if (!object.Equals (newValue, cachedProperty.OldValue)) {
				RaisePropertyChangedEvent (AutomationElementIdentifiers.NameProperty,
							   cachedProperty.OldValue,
							   newValue);
				if (NameChanged != null)
					NameChanged (this, EventArgs.Empty);
			}
		}

		#endregion

		#region StructureChanged methods, classes and variables

		// NOTE: All this region handles children added/removed to raise StructureChanged
		//       when any child contains a Panel.

		internal virtual void OnContentChanged (object oldContent, object newContent)
		{
			bool raiseIfRemoved = RemoveItemsChangedFromPanel (oldContent);
			bool raiseIfAdded = AddItemsChangedToPanel (newContent);

			// Only if something really changed.
			if (raiseIfRemoved || raiseIfAdded) {
				RaiseAutomationEvent (AutomationEvents.StructureChanged);
				RaiseNameChanged ();
			}
		}

		private void Panel_ItemsChanged (object sender, NotifyCollectionChangedEventArgs args)
		{
			bool raiseStructueChanged = true;
			bool hadChildren = false;

			switch (args.Action) {
			case NotifyCollectionChangedAction.Add:
				raiseStructueChanged = AddItemsChangedToPanel (args.NewItems [0]);
				break;
			case NotifyCollectionChangedAction.Remove:
				raiseStructueChanged = RemoveItemsChangedFromPanel (args.OldItems [0]);
				break;
			case NotifyCollectionChangedAction.Replace:
				hadChildren = RemoveItemsChangedFromPanel (args.OldItems [0]);
				bool hasChildren = AddItemsChangedToPanel (args.NewItems [0]);
				raiseStructueChanged = hasChildren || hadChildren;
				break;
			case NotifyCollectionChangedAction.Reset:
				Panel panel = ((ContentControl) Owner).Content as Panel;
				// In case sender.Children is not the content.Children we look for it
				if (panel == null || sender == panel.Children)
					panel = (from p in PanelParents
				                 where p.Panel.Children == sender
					         select p.Panel).FirstOrDefault () as Panel;

				if (panel != null) {
					List<PanelParent> children
						= new List<PanelParent> (from p in PanelParents
					                                 where p.Parent == panel
									 select p);
					foreach (PanelParent panelParentInList in children) {
						hadChildren = RemoveItemsChangedFromPanel (panelParentInList.Panel);
						panelParents.Remove (panelParentInList);

						if (!raiseStructueChanged)
							raiseStructueChanged = hadChildren;
					}
					raiseStructueChanged = hadChildren;
				}
				break;
			}

			if (raiseStructueChanged)
				RaiseAutomationEvent (AutomationEvents.StructureChanged);
		}

		// Both 'AddItemsChangedToPanel' and 'RemoveItemsChangedFromPanel' return true when:
		// 1) panelObject is any control type but Panel or,
		// 2) Panel has children

		private bool AddItemsChangedToPanel (object panelObject)
		{
			if (panelObject == null)
				return false;

			Panel panel = panelObject as Panel;
			if (panel == null)
				return true;
			panel.Children.ItemsChanged += Panel_ItemsChanged;

			// When GetParent() returns null it means its parent is ContentControl.
			Panel panelParent = VisualTreeHelper.GetParent (panel) as Panel;
			if (panelParent != null)
				PanelParents.Add (new PanelParent () { Panel  = panel,
								       Parent = panelParent });

			bool raiseEvent = false;
			int childrenCount = VisualTreeHelper.GetChildrenCount (panel);
			for (int child = 0; child < childrenCount; child++) {
				if (AddItemsChangedToPanel (VisualTreeHelper.GetChild (panel, child)))
					raiseEvent = true;
			}

			return raiseEvent;
		}

		private bool RemoveItemsChangedFromPanel (object panelObject)
		{
			if (panelObject == null)
				return false;

			Panel panel = panelObject as Panel;
			if (panel == null)
				return true;
			panel.Children.ItemsChanged -= Panel_ItemsChanged;

			bool raiseEvent = false;
			// Removing all panel's children and the panel itself.
			List<PanelParent> children
				= new List<PanelParent> (from p in PanelParents
			                                 where p.Parent == panel || p.Panel == panel
							 select p);
			foreach (PanelParent panelParentInList in children) {
				// We are already removing panel's children, so
				// we *only* need to remove child's children.
				if (panelParentInList.Panel != panel) {
					if (RemoveItemsChangedFromPanel (panelParentInList.Panel))
						raiseEvent = true;
				} else {
					// When a new item is added to PanelParents it means is a Panel (p0),
					// so all p0's children are added *after* their parent.
					// In other words: in 'children' list, parent will be listed
					// before its children.
					if (panelParentInList.Panel.Children.Count >= children.Count)
						raiseEvent = true;
				}

				panelParents.Remove (panelParentInList);
			}

			return raiseEvent;
		}

		// Keeps a relation of Panel and its Parent
		// we need this, since Panel.Children.Clear() raises the event
		// but the argument doesn't include the removed items and we
		// need to remove all delegates from all children when those are Panel.
		internal class PanelParent {
			public Panel Panel;
			public Panel Parent;
		}

		private List<PanelParent> PanelParents {
			get {
				if (panelParents == null)
					panelParents = new List<PanelParent> ();
				return panelParents;
			}
		}
		private List<PanelParent> panelParents;

		#endregion

		#region IsOffscreen methods, class and variables

		// NOTE:
		// This region handles parent's Visibility to raise IsOffscreen UIA event.
		// The rule to do so is: IsOffscreen() depends on owner.Visibility and
		// "all its" parents.Visibility, for example, in:
		//
		// - C1
		// -- C2
		// --- C3 // owner
		//
		// If C1.Visibility is Collapsed then C3 IsOffscreen=true, even when
		// their C3.Visibility is Visible.

		private void CacheParents ()
		{
			UIElement parent = VisualTreeHelper.GetParent (owner) as UIElement;
			if (cachedParent != parent) {
				cachedParent = parent;

				// Removing delegates in older parents
				if (parents != null) {
					foreach (UIElement parentItem in parents)
						parentItem.UIAVisibilityChanged -= Parent_UIAVisibilityChanged;
					parents.Clear ();
				}

				// Caching new parents and listening for their UIAVisibilityChanged
				if (parent != null) {
					Parents.Add (parent);
					parent.UIAVisibilityChanged += Parent_UIAVisibilityChanged;
					do {
						parent = VisualTreeHelper.GetParent (parent) as UIElement;
						if (parent != null) {
							parents.Add (parent); // At this time parents is not null
							parent.UIAVisibilityChanged += Parent_UIAVisibilityChanged;
						}
					} while (parent != null);
				}
			}
		}

		private void Parent_UIAVisibilityChanged (object sender, DependencyPropertyChangedEventArgs args)
		{
			bool isOffscreen = IsOffscreen ();
			RaisePropertyChangedEvent (AutomationElementIdentifiers.IsOffscreenProperty,
			                           !isOffscreen,
			                           isOffscreen);
		}

		private List<UIElement> Parents {
			get {
				if (parents == null)
					parents = new List<UIElement> ();
				return parents;
			}
		}

		private bool KeyboardFocusable {
			get {
				Control control = Owner as Control;
				if (control == null || VisualTreeHelper.GetParent (control) == null)
					return false;
				else {
					// According to http://msdn.microsoft.com/en-us/library/cc903954%28VS.95%29.aspx
					// Notice that this method is similar to Control::Focus, the most important
					// difference is that Peer doesn't depend on its Parent Visibility.
					return control.IsEnabled
					       && control.IsTabStop
					       && control.Visibility == Visibility.Visible;
				}
			}
		}

		private UIElement cachedParent;
		private List<UIElement> parents;

		#endregion
	}

}
