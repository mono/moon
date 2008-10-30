/*
 * asf-testing.cpp: Includes methods used to test the demuxer. 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <glib.h>

#include "asf-test.h"
#include "asf.h"


static char*
create_new_file_name (const char* prefix, const char* number_format, const char* suffix)
{
	char* result = NULL;
	char* number = NULL;
	int counter = 0;
	FILE* fd = NULL;
	
	do { 
		counter++;
		if (fd != NULL)
			fclose (fd);
		g_free (result);
		
		number = g_strdup_printf (number_format, counter);
		result = g_strdup_printf ("%s%s%s", prefix, number, suffix);
		g_free (number);
	} while ((fd = fopen (result, "rb")) != NULL);
	
	return result;
}

static bool
corrupt_file (const char* input_filename, const char* output_filename, const char* log_filename, bool replay, double corruption)
{
	printf ("corrupt_file (%s, %s, %s, %s, %f)\n", input_filename, output_filename, log_filename, replay ? "true" : "false", corruption);
	
	FILE *in = NULL, *out = NULL, *log = NULL;
	char* out_fn = NULL;
	char* log_fn = NULL;
	char* msg = NULL;
	bool result = false;
	int counter = 0, added = 0, trashed = 0, removed = 0, total = 0;
	int corruption_range = (int) ((double) RAND_MAX * (1.0 / corruption));
	
	if (input_filename == NULL) {
		fprintf (stderr, "No input filename specified\n");
		goto cleanup;
	}
	
	if (input_filename != NULL && output_filename != NULL && strcmp (input_filename, output_filename) == 0) {
		fprintf (stderr, "Input file and output file are same: %s\n", input_filename);
		goto cleanup;
	}
	
	in = fopen (input_filename, "rb");
	if (in == NULL) {
		fprintf (stderr, "Could not open input file '%s': %s\n", input_filename, strerror (errno));
		goto cleanup;
	}
	
	out_fn = g_strdup (output_filename);
	if (out_fn == NULL) {
		int counter = 1;
		out_fn = g_strdup_printf ("%s.%.3i.wmv", input_filename, counter);
		while ((out = fopen (out_fn, "rb")) != NULL) {
			fclose (out);
			g_free (out_fn);
			out_fn = g_strdup_printf ("%s%.3i.wmv", input_filename, counter); 
			counter++;
		}
		printf ("Determined output filename: %s\n", out_fn);
	}
	out = fopen (out_fn, "wb");
	if (out == NULL) {
		fprintf (stderr, "Could not open output file '%s': %s\n", out_fn, strerror (errno));
		goto cleanup;
	}
	
	log_fn = g_strdup (log_filename);
	if (log_fn == NULL) {
		log_fn = g_strdup_printf ("%s.log", out_fn);
		printf ("Determined log file: %s\n", log_fn);
	}
	log = fopen (log_fn, "w+b");
	if (log == NULL) {
		fprintf (stderr, "Could not open log file '%s': %s\n", log_fn, strerror (errno));
		goto cleanup;
	}
	if (!replay) {
		msg = g_strdup ("#Where\tWhat\tCount\tByte1\tByte2\n");
		if ((fwrite (msg, 1, strlen (msg), log) != strlen (msg))) {
			fprintf (stderr, "Error while writing log: %s\n", strerror (ferror (log)));
			goto cleanup;
		}
		g_free (msg);
		msg = NULL;
	}
	srand (time (0));
	
	uint8_t byte;
	int random_value;
	
	while (fread (&byte, 1, 1, in) == 1) {
		int what = rand () % 3;
		random_value = rand ();
		total++;
		
		uint8_t c [2];
		unsigned int cc = 1;

		c [0] = byte;
		
		if (replay) {
			
		} else {
			if (random_value < corruption_range) {
				// Let's corrupt something.
				switch (what) {
				case 0: // write nothing.
					removed++;
					cc = 0;
					break;
				case 1: // write garbage
					trashed++;
					cc = 1;
					c [0] ^= (uint8_t) rand ();
					break;
				case 2: // write an extra byte
					added++;
					cc = 2;
					c [1] = (uint8_t) rand ();
					break;
				case 3: // write an extra byte (copying)
					added++;
					cc = 2;
					c [1] = c [0];
					break;
				}
				
				counter++;
				
				msg = g_strdup_printf ("%i\t%i\t%i\t%i\t%i\n", total, what, cc, (cc > 0) ? c [0] : 0, (cc > 1) ? c [1] : 0);
				if ((fwrite (msg, 1, strlen (msg), log) != strlen (msg))) {
					fprintf (stderr, "Error while writing log: %s\n", strerror (ferror (log)));
					goto cleanup;
				}
				g_free (msg); 
				msg = NULL;
			}
		}
		
		if (fwrite (c, 1, cc, out) != cc) {
			fprintf (stderr, "Error while writing output: %s\n", strerror (ferror (out)));
			goto cleanup;
		}
	}
	
	printf ("corrupt_file (%s, %s, %s, %s, %f): corrupted %i bytes (of %i bytes, %i added, %i removed, %i randomized).\n", input_filename, output_filename, log_filename, replay ? "true" : "false", corruption, counter, total, added, removed, trashed);
	
	result = true;

cleanup:
	g_free (out_fn);
	g_free (log_fn);
	fclose (in);
	fclose (out);
	fclose (log);
	g_free (msg);
	
	out_fn = NULL;
	log_fn = NULL;
	in = NULL;
	out = NULL;
	log = NULL;
	msg = NULL;
	
	return result;
}

static bool
test_file (const char* filename)
{	
	printf ("test_file (%s)\n", filename);
	
	int counter = 0;
	bool result = false;
	ASFParser* parser = NULL;
	ASFPacket* packet = NULL;
	ASFReader* reader = NULL;
	ASFFrameReader* frame_reader = NULL;
	FileSource *fs;
	MediaResult read_result;	

	fs = new FileSource (NULL, filename);
	if (!MEDIA_SUCCEEDED (read_result = fs->Initialize ())) {
		printf ("test_file (%s): could not open file (%i)\n", filename, read_result);
		return false;
	}

	parser = new ASFParser (fs, NULL);

	if (!MEDIA_SUCCEEDED (parser->ReadHeader ())) {
		printf ("test_file (%s): read header failed.\n", filename);
		goto end;
	}

#if 1
	packet = new ASFPacket ();
	printf ("Reading packet #0...\n");
	while (true) {
		read_result = parser->ReadPacket (packet);
		delete packet;
		printf ("Reading packet #%i, result: %i.\n", ++counter, read_result);
		if (!MEDIA_SUCCEEDED (read_result)) 
			break;
		packet = new ASFPacket ();
	}
#endif
#if 1
	printf ("Reading payloads...\n");
	reader = new ASFReader (parser, NULL);
	for (int i = 1; i < 128; i++) {
		if (!parser->IsValidStream (i))
			continue;
		printf ("Reading payloads in stream #%i...\n", i);
		reader->SelectStream (i, true);
		frame_reader = reader->GetFrameReader (i);
		while (MEDIA_SUCCEEDED (frame_reader->Advance ())) {
		}
	}
#endif
	
#if OBJECT_TRACKING
	ObjectTracker::PrintStatus ("ASFPacket");
	ObjectTracker::PrintStatus ("ASFFrameReader");
	ObjectTracker::PrintStatus ("ASFFrameReaderData");
	ObjectTracker::PrintStatus ("asf_single_payload");
#endif

	if (parser->GetLastError () != NULL) {
		printf ("Errors were reported, last error is: '%s'.\n", parser->GetLastErrorStr ());
		result = false;
	} else {
		result = true;
	}
		
end:
	printf ("test_file (%s): %s.\n", filename, result ? "OK" : "FAILED");
	delete parser;
	parser = NULL;
	fs->unref ();
	
	return result;
}


int
main (int argc, char *argv [])
{
	if (argc < 2) {
		printf ("ASF tester.\n");
		printf ("-test\n");
		printf ("  -test:<filename>\n");
		printf ("-corrupt\n");
		printf ("  -in:<filename>\n");
		printf ("  -out:<filename>\n");
		printf ("  -test:<filename>\n");
		printf ("  -corruption:<number>\n");
		printf ("  -replay\n");
		return 0;
	}

	char *in = NULL, *out = NULL, *log = NULL, *test = NULL;
	int corruption = 20000;
	bool replay = false;
	bool do_test = false, do_corrupt = false;
	int result = 1;
	
	for (int i = 1; i < argc; i++) {
		char* arg = argv [i];
		
		if (strcmp (arg, "-test") == 0) {
			do_test = true;
		} else if (strcmp (arg, "-corrupt") == 0) {
			do_corrupt = true;
		} else if (g_str_has_prefix (arg, "-in:")) { // required for corrupt
			in = arg + 4;
		} else if (g_str_has_prefix (arg, "-out:")) {  // for corrupt
			out = g_strdup (arg + 5);
		} else if (g_str_has_prefix (arg, "-log:")) { // for corrupt
			log = g_strdup (arg + 5);
		} else if (g_str_has_prefix (arg, "-test:")) { // required for test (automatically tests)
			test = arg + 6;
			do_test = true;
		} else if (g_str_has_prefix (arg, "-corruption:")) {  // for corrupt (automatically corrupts)
			corruption = atoi (arg + 12);
			do_corrupt = true;
		} else if (g_str_has_prefix (arg, "-replay")) { // for corrupt
			replay = true;
		} else {
			fprintf (stderr, "Unknown option: %s\n", arg);
			return 1;
		}
	}

	if (do_corrupt) {
		if (out == NULL) {
			out = create_new_file_name (in, "%.3i", ".wmv");
		}

		if (log == NULL) {
			log = g_strdup_printf ("%s.log", out);
		}

		if (!corrupt_file (in, out, log, replay, corruption)) {
			goto cleanup;
		}
		
		if (test == NULL) {
			test = out;
		}
	}
	
	if (do_test && !test_file (test)) {
		goto cleanup;
	}

	result = 0;
	
cleanup:
	g_free (out);
	g_free (log);
	
	out = NULL;
	log = NULL;
	
	return result;
}

