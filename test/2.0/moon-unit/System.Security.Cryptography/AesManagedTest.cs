//
// AesManagedTest.cs - NUnit Test Cases for AES
//
// Author:
//	Sebastien Pouliot (sebastien@ximian.com)
//
// (C) 2002 Motus Technologies Inc. (http://www.motus.com)
// Copyright (C) 2004,2009 Novell, Inc (http://www.novell.com)
//

using System;
using System.IO;
using System.Security.Cryptography;
using System.Text;

using Mono.Moonlight.UnitTesting;
using Microsoft.VisualStudio.TestTools.UnitTesting;

namespace MoonTest.System.Security.Cryptography {

	[TestClass]
	public class AesTest {

		public void AreEqual (byte [] expected, byte [] actual, string message)
		{
			Assert.AreEqual (BitConverter.ToString (expected), BitConverter.ToString (actual), message);
		}

		[TestMethod]
		public void DefaultProperties ()
		{
			Aes algo = new AesManaged ();
			// AES only support block size == 128 
			Assert.AreEqual (128, algo.BlockSize, "BlockSize");
			Assert.Throws<CryptographicException> (delegate {
				algo.BlockSize = 256;
			}, "256");
			Assert.AreEqual (1, algo.LegalBlockSizes.Length, "LegalBlockSizes");
			Assert.AreEqual (128, algo.LegalBlockSizes [0].MaxSize, "LegalBlockSizes.MaxSize");
			Assert.AreEqual (128, algo.LegalBlockSizes [0].MinSize, "LegalBlockSizes.MinSize");
			Assert.AreEqual (0, algo.LegalBlockSizes [0].SkipSize, "LegalBlockSizes.SkipSize");

			Assert.AreEqual (256, algo.KeySize, "KeySize");
			Assert.AreEqual (1, algo.LegalKeySizes.Length, "LegalKeySizes");
			Assert.AreEqual (256, algo.LegalKeySizes [0].MaxSize, "LegalKeySizes.MaxSize");
			Assert.AreEqual (128, algo.LegalKeySizes [0].MinSize, "LegalKeySizes.MinSize");
			Assert.AreEqual (64, algo.LegalKeySizes [0].SkipSize, "LegalKeySizes.SkipSize");
		}

		[TestMethod]
		public void ChangingKeySize ()
		{
			Aes aes = new AesManaged ();
			byte [] original_key = aes.Key;
			byte [] original_iv = aes.IV;
			foreach (KeySizes ks in aes.LegalKeySizes) {
				for (int key_size = ks.MinSize; key_size <= ks.MaxSize; key_size += ks.SkipSize) {
					aes.KeySize = key_size;
					string s = key_size.ToString ();
					// key is updated
					Assert.AreEqual ((key_size >> 3), aes.Key.Length, s + ".Key.Length");
					// iv isn't
					AreEqual (original_iv, aes.IV, s + ".IV");
				}
			}
		}

		public void CheckCBC (ICryptoTransform encryptor, ICryptoTransform decryptor,
					   byte [] plaintext, byte [] expected)
		{

			if ((plaintext.Length % encryptor.InputBlockSize) != 0) {
				throw new ArgumentException ("Must have complete blocks");
			}

			byte [] ciphertext = new byte [plaintext.Length];
			for (int i = 0; i < plaintext.Length; i += encryptor.InputBlockSize) {
				encryptor.TransformBlock (plaintext, i, encryptor.InputBlockSize, ciphertext, i);
			}
			AreEqual (expected, ciphertext, "CBC");

			byte [] roundtrip = new byte [plaintext.Length];
			for (int i = 0; i < ciphertext.Length; i += decryptor.InputBlockSize) {
				decryptor.TransformBlock (ciphertext, i, decryptor.InputBlockSize, roundtrip, i);
			}
			AreEqual (plaintext, roundtrip, "CBC-rt");
		}

		[TestMethod]
		public void CBC_0 ()
		{
			byte [] plaintext = new byte [32];
			byte [] iv = new byte [16];

			Aes r = new AesManaged ();
			byte [] key = new byte [16];

			for (int i = 0; i < 16; i++)
				r.Key [i] = 0;
			r.Key = key;

			byte [] expected = { 
				0x66, 0xe9, 0x4b, 0xd4, 0xef, 0x8a, 0x2c, 0x3b, 
				0x88, 0x4c, 0xfa, 0x59, 0xca, 0x34, 0x2b, 0x2e, 
				0xf7, 0x95, 0xbd, 0x4a, 0x52, 0xe2, 0x9e, 0xd7, 
				0x13, 0xd3, 0x13, 0xfa, 0x20, 0xe9, 0x8d, 0xbc };

			CheckCBC (r.CreateEncryptor (key, iv), r.CreateDecryptor (key, iv), plaintext, expected);
		}

		[TestMethod]
		public void CBC_1 ()
		{
			byte [] plaintext = new byte [32];
			byte [] iv = new byte [16];
			for (byte i = 0; i < iv.Length; i++) {
				iv [i] = i;
			}

			Aes r = new AesManaged ();
			byte [] key = new byte [16];
			for (byte i = 0; i < 16; i++)
				key [i] = 0;

			r.Key = key;

			byte [] expected = { 
				0x7a, 0xca, 0x0f, 0xd9, 0xbc, 0xd6, 0xec, 0x7c, 
				0x9f, 0x97, 0x46, 0x66, 0x16, 0xe6, 0xa2, 0x82, 
				0x66, 0xc5, 0x84, 0x17, 0x1d, 0x3c, 0x20, 0x53, 
				0x6f, 0x0a, 0x09, 0xdc, 0x4d, 0x1e, 0x45, 0x3b };

			CheckCBC (r.CreateEncryptor (key, iv), r.CreateDecryptor (key, iv), plaintext, expected);
		}
	}
}

