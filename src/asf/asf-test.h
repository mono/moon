/*
 * asf-testing.h: Includes methods used to test the demuxer. 
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 */


/*
	This function reads all the bytes in the input, 
	corrupts them in various random ways, 
	writes them to the output, 
	and at the same time writes a log of what it does 
	in order to replay the corruption later on.
	
	output_filename:	can be NULL, the method computes an output filename based on the input filename.
	log_filename:		can be NULL, the method computes an output filename based on the input filename.
	replay:				if true, replays the log (in which case the log filename can't be NULL):
	corruption:			introduce corruption approximately once every X bytes.
*/

//bool
//corrupt_file (const char* input_filename, const char* output_filename, const char* log_filename, bool replay, double corruption);

/*
	Tries to do everything possible with the file (parses everything, reads all packets, frames, etc).
*/

//bool
//test_file (const char* file);



