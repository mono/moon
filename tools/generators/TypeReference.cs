/*
 * TypeReference.cs.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

using System;
using System.Collections.Generic;
using System.Text;

class TypeReference {
	public string Value;
	public bool IsConst;
	public bool IsRef;
	public bool IsOut;
	public bool IsReturnType;

	private string managed_type;
	private string np_type;

	private Nullable <bool> is_known;
	
	public TypeReference () {}
	public TypeReference (string value)
	{
		this.Value = value;
	}
	
	public void WriteFormatted (StringBuilder text)
	{
		if (IsConst)
			text.Append ("const ");
		text.Append (GetPrettyType ());
	}

	public string WriteFormatted ()
	{
		StringBuilder text = new StringBuilder ();
		WriteFormatted (text);
		return text.ToString ();
	}

	public void Write (StringBuilder text, SignatureType type, GlobalInfo info)
	{
		if (IsConst && (type == SignatureType.Native || type == SignatureType.NativeC))
			text.Append ("const ");
		
		if (type != SignatureType.Native && type != SignatureType.NativeC) {
			if (IsRef && !IsReturnType)
				text.Append ("ref ");
			if (IsOut && !IsReturnType)
				text.Append ("out ");
		}
		
		if (type == SignatureType.NativeC && info.IsEnum (Value)) {
			text.Append (GetPrettyType ().Replace (Value.Replace ("*", ""), "int"));
		} else if (type == SignatureType.Native || type == SignatureType.NativeC) {
			text.Append (GetPrettyType ());
		} else {
			text.Append (GetManagedType ());
		}
	}
	
	public bool IsKnown {
		get {
			if (!is_known.HasValue) {
				if (string.IsNullOrEmpty (GetManagedType ()))
					is_known = new Nullable<bool> (false);
				else if (GetManagedType ().Contains ("Unknown"))
					is_known = new Nullable<bool> (false);
				else
					is_known = new Nullable<bool> (true);
			}
			return is_known.Value;
		}
	}
	
	public bool IsPointer {
		get { return Value[Value.Length - 1] == '*'; }
	}
	
	public string GetPrettyType ()
	{
		if (IsPointer)
			return Value.Substring (0, Value.Length - 1) + " *";
		
		return Value;
	}

	public string GetNPType ()
	{
		if (np_type == null) {
			switch (Value) {
				case "int":
				case "guint32":
				case "uint32_t":
				case "gint32":
				case "int32_t":
					np_type = "i";
					break;
				case "bool":
				case "gboolean":
					np_type = "b";
					break;
				case "double":
				case "int64_t":
				case "gint64":
				case "uint64_t":
				case "guint64":
					np_type = "d";
					break;
				case "char*":
					np_type = "s";
					break;
				case "void":
					np_type = "v";
					break;
				default:
					np_type = "o";
					break;
			}
		}
		return np_type;
	}
	
	public string GetManagedType ()
	{
		if (managed_type == null) {
			switch (Value) {
			case "int":
			case "bool":
			case "void":
			case "long":
			case "double":
				managed_type = Value;
				break;
			case "guint32":
			case "uint32_t":
				managed_type = "uint";
				break;
			case "gint32":
			case "int32_t":
				managed_type = "int";
				break;
			case "int64_t":
			case "gint64":
				managed_type = "long";
				break;
			case "uint64_t":
			case "guint64":
				managed_type = "ulong";
				break;
			case "ManagedStreamCallbacks*":
				IsRef = true;
				managed_type = "ManagedStreamCallbacks";
				break;
			case "MoonError*":
				IsOut = true;
				managed_type = "MoonError";
				break;
			case "gboolean":
				managed_type = "bool";
				break;
			case "Value*":
				if (IsReturnType) {
					managed_type = "IntPtr";
				} else {
					IsRef = true;
					managed_type = "Value";
				}
				break;
			case "ApplyDefaultStyleCallback":
				managed_type = "Mono.ApplyDefaultStyleCallback";
				break;
			case "GetDefaultTemplateCallback":
				managed_type = "Mono.GetDefaultTemplateCallback";
				break;
			case "ApplyStyleCallback":
				managed_type = "Mono.ApplyStyleCallback";
				break;
			case "ConvertKeyframeValueCallback":
				managed_type = "Mono.ConvertKeyframeValueCallback";
				break;
			case "GetDefaultTemplateRootCallback":
				managed_type = "Mono.GetDefaultTemplateRootCallback";
				break;
			case "PropertyChangeHandler":
				managed_type = "Mono.UnmanagedPropertyChangeHandler";
				break;
			case "EasingFunction":
				managed_type = "Mono.EasingFunctionCallback";
				break;
			case "LoadedCallback":
				managed_type = "Mono.LoadedCallback";
				break;
			case "MeasureOverrideCallback":
				managed_type = "Mono.MeasureOverrideCallback";
				break;
			case "ArrangeOverrideCallback":
				managed_type = "Mono.ArrangeOverrideCallback";
				break;
			case "GetResourceCallback":
				managed_type = "Mono.GetResourceCallback";
				break;
			case "GetValueCallback":
				managed_type = "Mono.GetValueCallback";
				break;
			case "SetValueCallback":
				managed_type = "Mono.SetValueCallback";
				break;
			case "CloseDemuxerCallback":
			case "GetDiagnosticAsyncCallback":
			case "GetFrameAsyncCallback":
			case "OpenDemuxerAsyncCallback":
			case "SeekAsyncCallback":
			case "SwitchMediaStreamAsyncCallback":
				managed_type = "System.Windows.Media.MediaStreamSource." + Value.Replace ("Callback", "Delegate");
				break;
			case "GDestroyNotify":
				managed_type = "IntPtr"; // hack, because we never pass this from managed code
				                         // (it's used for EventObject::AddHandler).
				break;
			case "char*":
				managed_type = "string";
				break;
			case "double*":
				managed_type = "double";
				IsOut = true;
				break;
			case "EventHandler":
				managed_type = "UnmanagedEventHandler";
				break;
			case "Size":
				managed_type = "Size";
				break;
			case "Rect":
				managed_type = "Rect";
				break;
			case "Point":
				managed_type = "Point";
				break;
			case "Thickness":
				managed_type = "Thickness";
				break;
			case "CornerRadius":
				managed_type = "CornerRadius";
				break;
			case "TimeSpan":
				managed_type = "long";
				break;
			case "bool*":
				if (IsReturnType) {
					managed_type = "IntPtr";
				} else {
					IsOut = true;
					managed_type = "bool";
				}
				break;
			case "int*":
				if (IsReturnType) {
					managed_type = "IntPtr";
				} else {
					IsOut = true;
					managed_type = "int";
				}
				break;
			case "Type::Kind*":
				if (IsReturnType) {
					managed_type = "IntPtr";
				} else {
					IsOut = true;
					managed_type = "Kind";
				}
				break;
			case "Type::Kind":
				managed_type = "Kind";
				break;
			case "gpointer":
				managed_type = "IntPtr";
				break;
			case "ModifierKeys":
				managed_type = "int";
				break;
			case "BindingMode":
				managed_type = "int";
				break;
			case "ToggleNotifyHandler":
				managed_type = "Mono.ToggleRef.ToggleNotifyHandler";
				break;

			case "DownloaderCreateStateFunc":
				managed_type = "Mono.DownloaderCreateStateFunc";
				break;
			case "DownloaderDestroyStateFunc":
				managed_type = "Mono.DownloaderDestroyStateFunc";
				break;
			case "DownloaderOpenFunc":
				managed_type = "Mono.DownloaderOpenFunc";
				break;
			case "DownloaderSendFunc":
				managed_type = "Mono.DownloaderSendFunc";
				break;
			case "DownloaderAbortFunc":
				managed_type = "Mono.DownloaderAbortFunc";
				break;
			case "DownloaderHeaderFunc":
				managed_type = "Mono.DownloaderHeaderFunc";
				break;
			case "DownloaderBodyFunc":
				managed_type = "Mono.DownloaderBodyFunc";
				break;
			case "DownloaderCreateWebRequestFunc":
				managed_type = "Mono.DownloaderCreateWebRequestFunc";
				break;
			case "DownloaderSetResponseHeaderCallbackFunc":
				managed_type = "Mono.DownloaderSetResponseHeaderCallbackFunc";
				break;
			case "DownloaderGetResponseFunc":
				managed_type = "Mono.DownloaderGetResponseFunc";
				break;
			case "XamlLoaderCallbacks":
				managed_type = "Xaml.XamlLoaderCallbacks";
				break;
			case "get_image_uri_func":
				managed_type = "Mono.ImageUriFunc";
				break;
			case "TickCallHandler":
				managed_type = "TickCallHandler";
				break;
			case "DownloaderResponseStartedHandler":
				managed_type = "DownloaderResponseStartedDelegate";
				break;
			case "DownloaderResponseDataAvailableHandler":
				managed_type = "DownloaderResponseAvailableDelegate";
				break;
			case "DownloaderResponseFinishedHandler":
				managed_type = "DownloaderResponseFinishedDelegate";
				break;
			case "DownloaderResponseHeaderCallback":
				managed_type = "HeaderVisitor";
				break;
			case "callback_dom_event*":
				managed_type = "DomEventCallback";
				break;
			case "InvokeDelegate":
			case "SetPropertyDelegate":
			case "GetPropertyDelegate":
			case "EventHandlerDelegate":
				managed_type = Value;
				break;
			default:
				if (Value.Contains ("*"))
					managed_type = "IntPtr";
				else
					managed_type = "/* Unknown: '" + Value + "' */";
				break;
			}
		}
		
		return managed_type;
	}
}
