//
// InputScopeNameValue.cs
//
// Contact:
//   Moonlight List (moonlight-list@lists.ximian.com)
//
// Copyright (C) 2010 Novell, Inc (http://www.novell.com)
//
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

using System;
using System.ComponentModel;

namespace System.Windows.Input {
	[EditorBrowsable (EditorBrowsableState.Never)]
	public enum InputScopeNameValue {
		EnumString = -5,
		Xml = -4,
		Srgs = -3,
		RegularExpression = -2,
		PhraseList = -1,
		Default = 0,
		Url = 1,
		FullFilePath = 2,
		FileName = 3,
		EmailUserName = 4,
		EmailSmtpAddress = 5,
		LogOnName = 6,
		PersonalFullName = 7,
		PersonalNamePrefix = 8,
		PersonalGivenName = 9,
		PersonalMiddleName = 10,
		PersonalSurname = 11,
		PersonalNameSuffix = 12,
		PostalAddress = 13,
		PostalCode = 14,
		AddressStreet = 15,
		AddressStateOrProvince = 16,
		AddressCity = 17,
		AddressCountryName = 18,
		AddressCountryShortName = 19,
		CurrencyAmountAndSymbol = 20,
		CurrencyAmount = 21,
		Date = 22,
		DateMonth = 23,
		DateDay = 24,
		DateYear = 25,
		DateMonthName = 26,
		DateDayName = 27,
		Digits = 28,
		Number = 29,
		OneChar = 30,
		Password = 31,
		TelephoneNumber = 32,
		TelephoneCountryCode = 33,
		TelephoneAreaCode = 34,
		TelephoneLocalNumber = 35,
		Time = 36,
		TimeHour = 37,
		TimeMinorSec = 38,
		NumberFullWidth = 39,
		AlphanumericHalfWidth = 40,
		AlphanumericFullWidth = 41,
		CurrencyChinese = 42,
		Bopomofo = 43,
		Hiragana = 44,
		KatakanaHalfWidth = 45,
		KatakanaFullWidth = 46,
		Hanja = 47,
		Yomi = 48,
		Text = 49,
		Chat = 50,
		Search = 51,
		NameOrPhoneNumber = 52,
		EmailNameOrAddress = 53,
		Private = 54,
		Maps = 55,
		ApplicationEnd = 55,
	}
}

